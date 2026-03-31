#include "./CustomImagesFontBuilder.h"

#include "../Externalncludes.h"

#include "./TextureAtlasPack.h"

#include "./lodepng.h"

CustomImageFontBuilder::CustomImageFontBuilder(const std::vector<CustomGlyph>& glyphsData)
{
	for (const auto& g : glyphsData)
	{
		this->glyphsData.try_emplace(g.c, g);
	}

	this->InitializeFont();
}

CustomImageFontBuilder::~CustomImageFontBuilder()
{
	this->Release();
}

void CustomImageFontBuilder::Release()
{
	for (auto& [c, gi] : this->customFi[0].glyphs)
	{
		SAFE_DELETE_ARRAY(gi.rawData);
	}
}

void CustomImageFontBuilder::InitializeFont()
{
	FontInfo fi;
	fi.faceName = "";
	fi.maxPixelsWidth = 0;
	fi.maxPixelsHeight = 0;
	fi.newLineOffset = 0;
	fi.onlyBitmapGlyphs = true;
	fi.fontFace = nullptr;
	fi.index = 0;
	fi.scaleFactor = 1.0f;

	this->customFi.push_back(fi);

	//todo
	this->texPacker = new TextureAtlasPack(256, 256, LETTER_BORDER_SIZE);


	//after each change, reset font infos in texture packer	
	this->texPacker->SetAllFontInfos(&this->customFi);
}

void CustomImageFontBuilder::SetAllFontSize(const FontSize& fs, uint16_t defaultFontSizeInPx)
{
}

bool CustomImageFontBuilder::AddString(const StringUtf8& str)
{
	bool res = false;
	auto it = CustomIteratorCreator::Create(str);
	uint32_t c;
	while ((c = it.GetCurrentAndAdvance()) != it.DONE)
	{
		res |= this->AddCharacter(c);
	}

	return res;
}

bool CustomImageFontBuilder::AddCharacter(CHAR_CODE c)
{
	if (c == '\n')
	{
		return false;
	}
	
	if (this->customFi[0].glyphs.find(c) != this->customFi[0].glyphs.end())
	{
		//character already exist
		this->reused.insert(c);
		return false;	
	}

	//new character
	//can insert the same character again, set does not contain duplicities
	this->newCodes.insert(c);

	return true;
}

uint16_t CustomImageFontBuilder::GetMaxFontPixelHeight() const
{
	return this->customFi[0].maxPixelsHeight;
}

int16_t CustomImageFontBuilder::GetMaxNewLineOffset() const
{
	return this->customFi[0].newLineOffset;
}

/// <summary>
/// Get single Glyph for char with given unicode.
/// Output true/false if exist
/// If not exist, return iterator is end() from first font
/// </summary>
/// <param name="c"></param>
/// <param name="exist"></param>
/// <returns></returns>
FontInfo::GlyphIterator CustomImageFontBuilder::GetGlyph(CHAR_CODE c, bool& exist)
{
	FontInfo* f = nullptr;
	return this->GetGlyph(c, exist, &f);
}

FontInfo::GlyphIterator CustomImageFontBuilder::GetGlyph(CHAR_CODE c, bool& exist, FontInfo** usedFi)
{
	exist = false;

	auto it = this->customFi[0].glyphs.find(c);
	if (it != this->customFi[0].glyphs.end())
	{
		*usedFi = &this->customFi[0];
		exist = true;
		return it;
	}

	*usedFi = &this->customFi[0];
	return this->customFi[0].glyphs.end();
}

/// <summary>
/// Get font texture width
/// </summary>
/// <returns></returns>
uint16_t CustomImageFontBuilder::GetTextureWidth() const
{
	return this->texPacker->GetTextureWidth();
}

/// <summary>
/// Get font texture height
/// </summary>
/// <returns></returns>
uint16_t CustomImageFontBuilder::GetTextureHeight() const
{
	return this->texPacker->GetTextureHeight();
}

/// <summary>
/// Get font texture raw data
/// </summary>
/// <returns></returns>
const uint8_t* CustomImageFontBuilder::GetTextureData() const
{
	return this->texPacker->GetTextureData();
}

bool CustomImageFontBuilder::CreateFontAtlas()
{
	if (this->newCodes.empty())
	{
		//all is reused
		//no need to generate new texture, all characters all already in it
		//clear reused
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
	std::list<FontInfo::GlyphIterator> unused;
	

	for (FontInfo::GlyphIterator it = this->customFi[0].glyphs.begin();
		it != this->customFi[0].glyphs.end();
		it++)
	{
		if (this->reused.find(it->first) == this->reused.end())
		{
			//loaded character from font is not found in reused fonts - 
			//we can remove it from active texture

			//UnusedGlyphInfo ugi;
			//ugi.gi = it;
			//ugi.fontIndex = fi.index;

			unused.push_back(it);
		}
	}


	//try to add new codes to texture	

	//add unused space to texture - this can be reused
	this->texPacker->SetUnusedGlyphs(&unused);


	if (this->texPacker->Pack() == false)
	{

#if defined(_WIN32) && defined(_DEBUG)
		this->texPacker->SaveToFile("D://outofspace.png");
#endif
		MY_LOG_ERROR("Problem - no space, but we need all characters");

		//here do something if packing failed

		//BUT we still need to continue and erase "unused" glyphs from this->fi
		//because before packing run out of space, it could remove glyphs from
		//texture
	}

#if defined(_WIN32) && defined(_DEBUG)
	this->texPacker->SaveToFile("D://packed_tex_img.png");
#endif

	this->texPacker->RemoveErasedGlyphsFromFontInfo();


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
GlyphInfo* CustomImageFontBuilder::LoadGlyphInfo(CHAR_CODE c)
{

	auto it = glyphsData.find(c);
	if (it == glyphsData.end())
	{
		MY_LOG_ERROR("Character %lu not found", c);
		return nullptr;
	}

	auto tmp = this->FillGlyphInfo(it->first, it->second);
	if (tmp != nullptr)
	{
		return tmp;
	}
	
	MY_LOG_ERROR("Character %lu not found", c);
	return nullptr;
}

/// <summary>
/// Load single glyph info
/// and fill local fi
/// </summary>
/// <param name="c">glyph code</param>
/// <param name="fi">structure to be filled</param>
/// <returns></returns>
GlyphInfo* CustomImageFontBuilder::FillGlyphInfo(CHAR_CODE c, CustomGlyph& g)
{
	auto it = this->customFi[0].glyphs.find(c);
	if (it != this->customFi[0].glyphs.end())
	{
		//glyph already exist
		return &it->second;
	}


	std::vector<uint8_t> png;
	std::vector<uint8_t> buffer; //the raw pixels
	unsigned width, height;
	lodepng::State state; //optionally customize this one
	state.decoder.color_convert = 0; //keep input data channels count

	lodepng::load_file(png, g.fileName); //load the image file with given filename
	auto error = lodepng::decode(buffer, width, height, state, png);

	//if there's an error, display it
	if (error)
	{
		MY_LOG_ERROR("decoder error %d: %s", error,  lodepng_error_text(error));
		return nullptr;
	}

	this->customFi[0].maxPixelsWidth = std::max<uint16_t>(this->customFi[0].maxPixelsWidth, width);
	this->customFi[0].maxPixelsHeight = std::max<uint16_t>(this->customFi[0].maxPixelsHeight, height);
	
	//glyphLeft is the horizontal distance from the current pen position 
	//to the left - most border of the glyph bitmap, 
	//glyphTop is the vertical distance from the pen position (on the baseline) 
	//to the top - most border of the glyph bitmap.
	//It is positive to indicate an upwards distance.

	GlyphInfo gInfo;
	gInfo.code = c;
	gInfo.fontIndex = 0;
	gInfo.bmpX = 0;
	gInfo.bmpY = 0;
	gInfo.bmpW = width;
	gInfo.bmpH = height;
	gInfo.adv = 0;
	gInfo.rawData = nullptr;

	if (c > 32)
	{
		//add texture for only "non whitespace chars"		
		if (this->customFi[0].scaleFactor != 1.0)
		{
			gInfo.rawData = this->ResizeBitmapHermite(buffer, width, height);
		}
		else		
		{
			int bitmapSize = width * height;
			uint8_t* textureData = new uint8_t[bitmapSize];

			std::copy(buffer.data(),
				buffer.data() + bitmapSize,
				textureData);
			
			gInfo.rawData = textureData;
		}
	}

	auto tmp = this->customFi[0].glyphs.try_emplace(c, std::move(gInfo));

	return &tmp.first->second;

}

/// <summary>
/// Nearest neighbor resize
/// Fast, but "ugly"
/// </summary>
/// <param name="glyph"></param>
/// <param name="fi"></param>
/// <returns></returns>
uint8_t* CustomImageFontBuilder::ResizeBitmap(const std::vector<uint8_t>& buffer, uint16_t w, uint16_t h) const
{
	size_t width = static_cast<size_t>(w * this->customFi[0].scaleFactor);
	size_t height = static_cast<size_t>(h * this->customFi[0].scaleFactor);

	uint8_t* textureData = new uint8_t[width * height];

	double x_ratio = w / (double)width;
	double y_ratio = h / (double)height;
	double px, py;
	for (size_t i = 0; i < height; i++)
	{
		py = floor(i * y_ratio);
		double pyW = py * width;
		size_t iw = i * width;

		for (size_t j = 0; j < width; j++)
		{
			px = floor(j * x_ratio);
			textureData[j + iw] = buffer[static_cast<int>(px + pyW)];
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
uint8_t* CustomImageFontBuilder::ResizeBitmapHermite(const std::vector<uint8_t>& buffer, uint16_t w, uint16_t h) const
{

	size_t width = static_cast<size_t>(w * this->customFi[0].scaleFactor);
	size_t height = static_cast<size_t>(h * this->customFi[0].scaleFactor);

	uint8_t* textureData = new uint8_t[width * height];

	double ratio_w = static_cast<double>(w) / width;
	double ratio_h = static_cast<double>(h) / height;
	double ratio_w_half = std::ceil(ratio_w / 2.0);
	double ratio_h_half = std::ceil(ratio_h / 2.0);


	for (size_t j = 0; j < height; j++)
	{
		double center_y = (j + 0.5) * ratio_h;

		size_t yy_start = static_cast<size_t>(std::floor(j * ratio_h));
		size_t yy_stop = static_cast<size_t>(std::ceil((j + 1) * ratio_h));
		yy_stop = std::min(yy_stop, static_cast<size_t>(h));

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
			double center_x = (i + 0.5) * ratio_w;

			size_t xx_start = static_cast<size_t>(std::floor(i * ratio_w));
			size_t xx_stop = static_cast<size_t>(std::ceil((i + 1) * ratio_w));
			xx_stop = std::min(xx_stop, static_cast<size_t>(w));

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
					double w2 = w * w;

					//hermite filter
					double weight = 2 * w2 * w - 3 * w2 + 1;
					size_t pos_x = (xx + yy * w); //* 4;
					//alpha
					//gx_a += weight * glyphBmp.buffer[pos_x + 3];
					//weights_alpha += weight;
					//colors
					//if (glyphBmp.buffer[pos_x + 3] < 255)
					//{
					//	weight = weight * glyphBmp.buffer[pos_x + 3] / 250;
					//}
					gx_r += weight * buffer[pos_x];
					//gx_g += weight * glyphBmp.buffer[pos_x + 1];
					//gx_b += weight * glyphBmp.buffer[pos_x + 2];
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
