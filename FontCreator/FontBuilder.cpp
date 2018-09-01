#include "./FontBuilder.h"


#include "./TextureAtlasPack.h"


#include "./Externalncludes.h"

#ifdef USE_VFS
#include "../../Utils/VFS/VFS.h"
#endif

//http://www.freetype.org/freetype2/documentation.html
//http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

FontBuilder::FontBuilder(const std::vector<Font> & fonts, RenderSettings r)
{
	
	this->texPacker = new TextureAtlasPack(r.textureW, r.textureH, LETTER_BORDER_SIZE);
	this->texPacker->SetTightPacking();
	//this->texPacker->SetGridPacking(fontSize, fontSize);

	
	if (FT_Init_FreeType(&this->library))
	{
		MY_LOG_ERROR("Failed to initialize FreeType library.");
		//return;
	}

	
	for (auto & f : fonts)
	{
		int index = this->InitializeFont(f.name);

		if (index == -1)
		{
			continue;
		}

				
		if (f.size.sizeType == FontSize::SizeType::px)
		{
			this->SetFontSizePixels(this->fis[index], static_cast<int>(f.size));
		}
		else if (f.size.sizeType == FontSize::SizeType::em)
		{
			int size = static_cast<int>(f.defaultFontSizeInPx * f.size.size * r.screenScale);

			this->SetFontSizePixels(this->fis[index], size);
		}
		else
		{
			this->SetFontSizePts(this->fis[index], static_cast<int>(f.size), r.screenDpi);
		}
	}

	
	int maxEmSize = this->GetMaxEmSize();
	for (auto & f : this->fis)
	{		
		if (f.onlyBitmapGlyphs)
		{
			f.scaleFactor = static_cast<double>(maxEmSize) / f.maxPixelsHeight;

			f.maxPixelsHeight *= f.scaleFactor;
			f.maxPixelsWidth *= f.scaleFactor;
			f.newLineOffset *= f.scaleFactor;			
		}
	}

}


FontBuilder::~FontBuilder()
{
	SAFE_DELETE(this->texPacker);

	this->Release();
}

/// <summary>
/// Release allocated data
/// </summary>
void FontBuilder::Release()
{	
	for (FontInfo & f : this->fis)
	{
		FT_Done_Face(f.fontFace);
		f.fontFace = nullptr;
	}


	FT_Done_FreeType(this->library);
	this->library = nullptr;
	
	for (FontInfo & f : this->fis)
	{
		for (GlyphInfo & c : f.glyphs)
		{
			SAFE_DELETE_ARRAY(c.rawData);
		}
	}

	for (auto & f : this->memoryFonts)
	{
		SAFE_DELETE_ARRAY(f);
	}
}

//================================================================
// Initialization
//================================================================

/// <summary>
/// Test if FreeType library is inited
/// </summary>
/// <returns></returns>
bool FontBuilder::IsInited() const
{
	return this->library != nullptr;
}

/// <summary>
/// Load font from file fontFacePath to a buffer
/// </summary>
/// <param name="fontFacePath"></param>
/// <param name="bufSize"></param>
/// <returns></returns>
uint8_t * FontBuilder::LoadFontFromFile(const std::string & fontFacePath, size_t * bufSize)
{
#ifdef USE_VFS
	return (uint8_t *)VFS::GetInstance()->GetFileContent(fontFacePath.c_str(), bufSize);
#else

	FILE * f = nullptr;
	my_fopen(&f, fontFacePath.c_str(), "rb");

	if (f == nullptr)
	{
		*bufSize = 0;
		return nullptr;
	}

	fseek(f, 0, SEEK_END);
	*bufSize = static_cast<size_t>(ftell(f));
	fseek(f, 0, SEEK_SET);
		
	uint8_t * buf = new uint8_t[*bufSize];
	fread(buf, sizeof(uint8_t), *bufSize, f);

	fclose(f);
	return buf;
#endif
}

/// <summary>
/// Initialize font from font file with file name fontFacePath
/// From font, selected char map is Unicode
/// If units_per_EM are 0 => font contains only bitmap glyphs
/// </summary>
/// <param name="fontFacePath"></param>
/// <returns></returns>
int FontBuilder::InitializeFont(const std::string & fontFacePath)
{
	FontInfo fi;
	FT_Face ff;
	FT_Error error;

	fi.faceName = fontFacePath.substr(fontFacePath.find_last_of("/\\") + 1);
	std::string::size_type const p(fi.faceName.find_last_of('.'));
	fi.faceName = fi.faceName.substr(0, p);

	size_t bufSize = 0;
	auto data = this->LoadFontFromFile(fontFacePath, &bufSize);
	
	if (data != nullptr)
	{
		memoryFonts.push_back(data);
		error = FT_New_Memory_Face(this->library, data, bufSize, 0, &ff);
	}
	else
	{
		error = FT_New_Face(this->library, fontFacePath.c_str(), 0, &ff);
	}

	if (error == FT_Err_Unknown_File_Format)
	{
		MY_LOG_ERROR("Failed to initialize Font Face %s. File not supported", fi.faceName.c_str());
		return -1;
	}
	else if (error)
	{
		MY_LOG_ERROR("Failed to initialize Font Face %s.", fi.faceName.c_str());
		return -1;
	}

	FT_Select_Charmap(ff, FT_ENCODING_UNICODE);

	fi.fontFace = ff;
	fi.index = static_cast<int>(this->fis.size());
	fi.onlyBitmapGlyphs = false;
	fi.scaleFactor = 1.0f;
	
	if (ff->num_fixed_sizes != 0)
	{
		if (ff->units_per_EM == 0)
		{
			//only bitmap font
			fi.onlyBitmapGlyphs = true;
			//fi.scaleFactor = 0.5f;
		}
	}
	
	this->fis.push_back(fi);

	return fi.index;

}


/// <summary>
/// Set font size in "pt". This value is used for height, width is
/// auto calculated
/// For this, display DPI is needed
/// </summary>
/// <param name="f"></param>
/// <param name="size"></param>
/// <param name="dpi"></param>
/// <returns></returns>
bool FontBuilder::SetFontSizePts(FontInfo & f, int size, int dpi)
{
	//https://www.freetype.org/freetype2/docs/glyphs/glyphs-2.html
		
	
	if (f.onlyBitmapGlyphs)
	{
		//only bitmap font
		return this->SetClosestFontSizeForBitmaps(f, size);
	}

	if (FT_Error err = FT_Set_Char_Size(f.fontFace, 0, size * 64, dpi, dpi))
	{
		MY_LOG_ERROR("Failed to set font size in points: %i", err);
		return false;
	}

	
	double pixel_size = size * dpi / 72;

	
	f.maxPixelsHeight = round((f.fontFace->bbox.yMax - f.fontFace->bbox.yMin) * pixel_size / f.fontFace->units_per_EM);
	f.maxPixelsWidth = round((f.fontFace->bbox.xMax - f.fontFace->bbox.xMin) * pixel_size / f.fontFace->units_per_EM);

	//f.fontSizePixels = (size * (dpi / 64)); // this->fontFace->size->metrics.y_ppem;	
	f.newLineOffset = static_cast<int>(f.fontFace->size->metrics.height / 64);

	return true;
}

/// <summary>
/// Set font size directly in pixel size
/// </summary>
/// <param name="f"></param>
/// <param name="size"></param>
/// <returns></returns>
bool FontBuilder::SetFontSizePixels(FontInfo & f, int size)
{	
	//https://www.freetype.org/freetype2/docs/tutorial/step1.html
	
	if (f.onlyBitmapGlyphs)
	{
		//only bitmap font
		return this->SetClosestFontSizeForBitmaps(f, size);				
	}

	if (FT_Error err = FT_Set_Pixel_Sizes(f.fontFace, 0, size))
	{
		MY_LOG_ERROR("Failed to set font size in pixels: %i", err);
		return false;
	}

	//http://stackoverflow.com/questions/28009564/new-line-pixel-distance-in-freetype

	
	f.maxPixelsHeight = round((f.fontFace->bbox.yMax - f.fontFace->bbox.yMin) * size / f.fontFace->units_per_EM);
	f.maxPixelsWidth = round((f.fontFace->bbox.xMax - f.fontFace->bbox.xMin) * size / f.fontFace->units_per_EM);


	//f.fontSizePixels = size;

	// get the scaled line spacing (for 48pt), also measured in 64ths of a pixel
	f.newLineOffset = static_cast<int>(f.fontFace->size->metrics.height / 64);

	return true;
}

/// <summary>
/// For bitmap based only fonts, set the active size to be
/// the closest one to sedired pixel size
/// </summary>
/// <param name="f"></param>
/// <param name="size"></param>
/// <returns></returns>
bool FontBuilder::SetClosestFontSizeForBitmaps(FontInfo & f, int size)
{
	FT_Pos minDif = std::numeric_limits<FT_Pos>::max();
	int minDifIndex = 0;

	for (int i = 0; i < f.fontFace->num_fixed_sizes; i++)
	{
		auto s = f.fontFace->available_sizes[i];
		auto dif = std::abs(s.width - size);
		if (dif < minDif)
		{
			minDif = dif;
			minDifIndex = i;
		}
	}
	
	if (FT_Error err = FT_Select_Size(f.fontFace, minDifIndex))
	{
		MY_LOG_ERROR("Failed to set closest font size: %i", err);
		return false;
	}

	auto tmp = f.fontFace->available_sizes[minDifIndex];

	f.maxPixelsWidth = tmp.width;
	f.maxPixelsHeight = tmp.height;
	//f.fontSizePixels = std::max(tmp.width, tmp.height);

	// get the scaled line spacing (for 48pt), also measured in 64ths of a pixel
	f.newLineOffset = static_cast<int>(f.fontFace->size->metrics.height / 64);

	return true;
}

//================================================================


const std::vector<FontInfo> & FontBuilder::GetFontInfos() const
{
	return this->fis;
}

int FontBuilder::GetMaxFontPixelHeight() const
{
	int maxHeight = std::numeric_limits<int>::min();
	for (auto & fi : this->fis)
	{			
		maxHeight = std::max(maxHeight, fi.maxPixelsHeight);		
	}	
	return maxHeight;
}

int FontBuilder::GetMaxFontPixelWidth() const
{
	int maxWidth = std::numeric_limits<int>::min();
	for (auto & fi : this->fis)
	{				
		maxWidth = std::max(maxWidth, fi.maxPixelsWidth);
	}	
	return maxWidth;
}

/// <summary>
/// Get maximal pixel size of M glyph from all fonts
/// that are not obly bitmap
/// </summary>
/// <returns></returns>
int FontBuilder::GetMaxEmSize() const
{
	int maxSize = std::numeric_limits<int>::min();
	for (auto & fi : this->fis)
	{		
		if (fi.onlyBitmapGlyphs)
		{
			continue;
		}
		maxSize = std::max(maxSize, (int)fi.fontFace->size->metrics.y_ppem);
		maxSize = std::max(maxSize, (int)fi.fontFace->size->metrics.x_ppem);
	}
	return maxSize;
}

int FontBuilder::GetMaxNewLineOffset() const
{
	int lineOffset = std::numeric_limits<int>::min();
	for (auto & fi : this->fis)
	{
		if (fi.onlyBitmapGlyphs)
		{
			continue;
		}

		lineOffset = std::max(lineOffset, fi.newLineOffset);
	}

	return lineOffset;
}

int FontBuilder::GetNewLineOffsetBasedOnGlyph(CHAR_CODE c)
{
	this->LoadGlyphInfo(c);

	for (auto & fi : this->fis)
	{		
		auto it = fi.usedGlyphs.find(c);
		if (it != fi.usedGlyphs.end())
		{
			return fi.newLineOffset;
		}		
	}
	
	return this->GetMaxNewLineOffset();
}

/// <summary>
/// Get single Glyph for char with given unicode.
/// Output true/false if exist
/// If not exist, return iterator is end() from first font
/// </summary>
/// <param name="c"></param>
/// <param name="exist"></param>
/// <returns></returns>
FontInfo::UsedGlyphIterator FontBuilder::GetGlyph(CHAR_CODE c, bool & exist)
{
	FontInfo * f = nullptr;
	return this->GetGlyph(c, exist, &f);
}

FontInfo::UsedGlyphIterator FontBuilder::GetGlyph(CHAR_CODE c, bool & exist, FontInfo ** usedFi)
{
	exist = false;

	for (auto & fi : this->fis)
	{

		auto it = fi.usedGlyphs.find(c);
		if (it != fi.usedGlyphs.end())
		{	
			*usedFi = &fi;			
			exist = true;
			return it;
		}		
	}

	*usedFi = &this->fis[0];	
	return this->fis[0].usedGlyphs.end();
}

/// <summary>
/// Get font texture width
/// </summary>
/// <returns></returns>
int FontBuilder::GetTextureWidth() const
{
	return this->texPacker->GetTextureWidth();
}

/// <summary>
/// Get font texture height
/// </summary>
/// <returns></returns>
int FontBuilder::GetTextureHeight() const
{
	return this->texPacker->GetTextureHeight();
}

/// <summary>
/// Get font texture raw data
/// </summary>
/// <returns></returns>
const uint8_t * FontBuilder::GetTexture() const
{
	return this->texPacker->GetTexture();
}

/// <summary>
/// Save font texture to file
/// </summary>
/// <param name="fileName"></param>
void FontBuilder::Save(const std::string & fileName)
{
	this->texPacker->SaveToFile(fileName);
}

/// <summary>
/// Set tight packing. Each glyph will be fully stored
/// </summary>
void FontBuilder::SetTightPacking()
{
	this->texPacker->SetTightPacking();
}

/// <summary>
/// Glyhps will be packed to equal-sized grid
/// The default bin size is set as Em size
/// If some glyphs are bigger than specified bin size (binW x binH),
/// glyphs are truncated to bin size
/// </summary>
/// <param name="binW"></param>
/// <param name="binH"></param>
void FontBuilder::SetGridPacking(int binW, int binH)
{
	this->texPacker->SetGridPacking(binW, binH);
}

/// <summary>
/// Add new string
/// String is iterated character by character and they are added
/// to set
/// </summary>
void FontBuilder::AddString(const UnicodeString & str)
{
	FOREACH_32_CHAR_ITERATION(c, str)
	{
		this->AddCharacter(c);
	}
}

/// <summary>
/// Add all ASCII letters (a-z, A-Z)
/// </summary>
void FontBuilder::AddAllAsciiLetters()
{
	for (char c = 'a'; c <= 'z'; c++)
	{
		this->AddCharacter(c);
	}

	for (char c = 'A'; c <= 'Z'; c++)
	{
		this->AddCharacter(c);
	}
}

/// <summary>
/// Add all ASCII numbers (0-9)
/// </summary>
void FontBuilder::AddAllAsciiNumbers()
{
	for (char c = '0'; c <= '9'; c++)
	{
		this->AddCharacter(c);
	}
}

/// <summary>
/// Add single character in unicode
/// </summary>
/// <param name="c"></param>
void FontBuilder::AddCharacter(CHAR_CODE c)
{
	if (c == '\n')
	{
		return;
	}

	for (const FontInfo & fi : this->fis)
	{
		if (fi.usedGlyphs.find(c) != fi.usedGlyphs.end())
		{
			//character already exist
			this->reused.insert(c);
		}
		else
		{
			//new character
			//can insert the same character again, set does not contain duplicities
			this->newCodes.insert(c);

			//if (this->newCodes.find(c) == this->newCodes.end())
			//{
			//	this->newCodes.insert(c);
			//}
		}
	}

}



bool FontBuilder::CreateFontAtlas()
{		
	if (this->newCodes.size() == 0)
	{
		//all is reused - clear reused
		this->reused.clear();

		return false;
	}


	//Load new glyph infos
	for (CHAR_CODE c : this->newCodes)
	{
		this->LoadGlyphInfo(c);
		this->reused.insert(c);
	}

	
	//find unused glyphs, that can possibly be deleted
	std::list<UnusedGlyphInfo> unused;
	


	int index = 0;
	for (FontInfo & fi : this->fis)
	{
		for (FontInfo::UsedGlyphIterator it = fi.usedGlyphs.begin();
			it != fi.usedGlyphs.end();
			it++)
		{
			if (this->reused.find(it->first) == this->reused.end())
			{
				UnusedGlyphInfo ugi;
				ugi.gi = it;
				ugi.fontIndex = index;

				unused.push_back(ugi);				
			}
		}
		
		index++;
	}


	

	//try to add new codes to texture
	this->texPacker->SetAllGlyphs(&this->fis);
	this->texPacker->SetUnusedGlyphs(&unused);


	if (this->texPacker->Pack() == false)
	{

#if defined(_WIN32) && defined(_DEBUG)
		this->texPacker->SaveToFile("D://outofspace.png");
#endif
		MY_LOG_ERROR("Problem - no space, but we need all characters\n");
		
		//here do something if packing failed

		//BUT we still need to continue and erase "unused" glyphs from this->fi
		//because before packing run out of space, it could remove glyphs from
		//texture
	}

#if defined(_WIN32) && defined(_DEBUG)
	//this->texPacker->SaveToFile("D://packed_tex.png");
#endif
	
	this->texPacker->RemoveUnusedGlyphsFromFontInfo();


	//packing successfully finished
	//there was a space in texture and new glyphs can be added
	this->newCodes.clear();
	this->reused.clear();

	this->texPacker->SetUnusedGlyphs(nullptr);

	return true;
}


/// <summary>
/// Load single glyph info
/// and fill local structure
/// </summary>
/// <param name="c"></param>
void FontBuilder::LoadGlyphInfo(CHAR_CODE c)
{	
	for (auto & fi : this->fis)
	{
		if (this->LoadGlyphInfo(c, fi))
		{
			return;
		}
	}

	MY_LOG_ERROR("Character %lu not found", c);
}

bool FontBuilder::LoadGlyphInfo(CHAR_CODE c, FontInfo & fi)
{
	
	if (fi.usedGlyphs.find(c) != fi.usedGlyphs.end())
	{
		//glyph already exist
		return true;
	}

	FT_UInt ci = FT_Get_Char_Index(fi.fontFace, c);

	if (ci == 0)
	{		
		return false;
	}
	
	
	//FT_LOAD_RENDER
	if (FT_Error err = FT_Load_Glyph(fi.fontFace, ci, FT_LOAD_RENDER))
	{		
		return false;
	}
	
	FT_GlyphSlot glyph = fi.fontFace->glyph;

	/*
	if (FT_Error err = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL))
	{		
		return false;
	}
	*/
			
	if (glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
	{
		MY_LOG_ERROR("Only gray-scale glyphs are supported");
		return false;		
	}


	//bitmap_left is the horizontal distance from the current pen position 
	//to the left - most border of the glyph bitmap, 
	//bitmap_top is the vertical distance from the pen position (on the baseline) 
	//to the top - most border of the glyph bitmap.
	//It is positive to indicate an upwards distance.
	
	GlyphInfo gInfo;
	gInfo.code = c;
	gInfo.bmpX = static_cast<int>(glyph->bitmap_left * fi.scaleFactor);
	gInfo.bmpY = static_cast<int>(glyph->bitmap_top * fi.scaleFactor);
	gInfo.bmpW = static_cast<int>(glyph->bitmap.width * fi.scaleFactor);
	gInfo.bmpH = static_cast<int>(glyph->bitmap.rows * fi.scaleFactor);
	gInfo.adv = static_cast<long>(glyph->advance.x * fi.scaleFactor);
	gInfo.rawData = nullptr;

	if (c > 32)
	{
		//add texture for only "non whitespace chars"

		if (fi.scaleFactor != 1.0)
		{
			gInfo.rawData = this->ResizeBitmapHermite(glyph, fi);
		}
		else
		{
			int bitmapSize = glyph->bitmap.width * glyph->bitmap.rows;
			uint8_t * textureData = new uint8_t[bitmapSize];

			size_t j = 0;
			for (unsigned int y = 0; y < glyph->bitmap.rows; y++)
			{
				for (unsigned int x = 0; x < glyph->bitmap.width; x++)
				{
					textureData[j] = glyph->bitmap.buffer[x + y * glyph->bitmap.pitch];
					j++;
				}
			}

			//for (int j = 0; j < bitmapSize; j++)
			//{
				//textureData[j + 0] = glyph->bitmap.buffer[j];
			//}
			//---------------------


			gInfo.rawData = textureData;
		}
	}


	fi.glyphs.push_back(gInfo);

	FontInfo::GlyphIterator lastIter = std::prev(fi.glyphs.end());

	fi.usedGlyphs[gInfo.code] = lastIter;

	return true;

}


/// <summary>
/// Nearest neighbor resize
/// Fast, but "ugly"
/// </summary>
/// <param name="glyph"></param>
/// <param name="fi"></param>
/// <returns></returns>
uint8_t * FontBuilder::ResizeBitmap(FT_GlyphSlot glyph, FontInfo & fi)
{
	size_t w = static_cast<size_t>(glyph->bitmap.width * fi.scaleFactor);
	size_t h = static_cast<size_t>(glyph->bitmap.rows * fi.scaleFactor);

	uint8_t * textureData = new uint8_t[w * h];
	
	double x_ratio = glyph->bitmap.width / (double)w;
	double y_ratio = glyph->bitmap.rows / (double)h;
	double px, py;
	for (size_t i = 0; i < h; i++) 
	{
		for (size_t j = 0; j < w; j++)
		{
			px = floor(j * x_ratio);
			py = floor(i * y_ratio);
			textureData[(i*w) + j] = glyph->bitmap.buffer[static_cast<int>(px + py * glyph->bitmap.pitch)];			
		}
	}
	return textureData;
}

/// <summary>
/// Hermite resize (slower, but better quality than ResizeBitmap)
/// Taken from:
/// https://github.com/viliusle/Hermite-resize/blob/master/src/hermite.js
/// </summary>
/// <param name="glyph"></param>
/// <param name="fi"></param>
/// <returns></returns>
uint8_t * FontBuilder::ResizeBitmapHermite(FT_GlyphSlot glyph, FontInfo & fi)
{

	size_t width = static_cast<size_t>(glyph->bitmap.width * fi.scaleFactor);
	size_t height = static_cast<size_t>(glyph->bitmap.rows * fi.scaleFactor);
	
	uint8_t * textureData = new uint8_t[width * height];

	double ratio_w = static_cast<double>(glyph->bitmap.width) / width;
	double ratio_h = static_cast<double>(glyph->bitmap.width) / height;
	double ratio_w_half = std::ceil(ratio_w / 2.0);
	double ratio_h_half = std::ceil(ratio_h / 2.0);


	for (size_t j = 0; j < height; j++) 
	{		
		for (size_t i = 0; i < width; i++) 
		{
			size_t x2 = (i + j * width); // *4;

			//double weight = 0;
			double weights = 0;
			//double weights_alpha = 0;
			double gx_r = 0;
			//double gx_g = 0;
			//double gx_b = 0;
			//double gx_a = 0;
			double center_y = (j + 0.5) * ratio_h;
			double center_x = (i + 0.5) * ratio_w;
			
			size_t yy_start = std::floor(j * ratio_h);
			size_t yy_stop = std::ceil((j + 1) * ratio_h);

			size_t xx_start = std::floor(i * ratio_w);
			size_t xx_stop = std::ceil((i + 1) * ratio_w);

			xx_stop = std::min(xx_stop, glyph->bitmap.width);
			yy_stop = std::min(yy_stop, glyph->bitmap.rows);

			for (size_t yy = yy_start; yy < yy_stop; yy++) 
			{
				double dy = std::abs(center_y - (yy + 0.5)) / ratio_h_half;				
				double w0 = dy * dy; //pre-calc part of w
								
				for (size_t xx = xx_start; xx < xx_stop; xx++) 
				{
					double dx = std::abs(center_x - (xx + 0.5)) / ratio_w_half;
					double w = std::sqrt(w0 + dx * dx);
					if (w >= 1) 
					{
						//pixel too far
						continue;
					}
					//hermite filter
					double weight = 2 * w * w * w - 3 * w * w + 1;
					size_t pos_x = (xx + yy * glyph->bitmap.width); //* 4;
					//alpha
					//gx_a += weight * glyph->bitmap.buffer[pos_x + 3];
					//weights_alpha += weight;
					//colors
					//if (glyph->bitmap.buffer[pos_x + 3] < 255)
					//{
					//	weight = weight * glyph->bitmap.buffer[pos_x + 3] / 250;
					//}
					gx_r += weight * glyph->bitmap.buffer[pos_x];
					//gx_g += weight * glyph->bitmap.buffer[pos_x + 1];
					//gx_b += weight * glyph->bitmap.buffer[pos_x + 2];
					weights += weight;
				}
			}
			textureData[x2] = static_cast<uint8_t>(gx_r / weights);
			//textureData[x2 + 1] = gx_g / weights;
			//textureData[x2 + 2] = gx_b / weights;
			//textureData[x2 + 3] = gx_a / weights_alpha;
		}
	}

	return textureData;
}