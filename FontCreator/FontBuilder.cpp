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


}


FontBuilder::~FontBuilder()
{
	SAFE_DELETE(this->texPacker);

	this->Release();
}

void FontBuilder::Release()
{	
	for (auto & f : this->fis)
	{
		FT_Done_Face(f.fontFace);
		f.fontFace = nullptr;
	}


	FT_Done_FreeType(this->library);
	this->library = nullptr;
	
	for (auto & f : this->fis)
	{
		for (auto & c : f.glyphs)
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

bool FontBuilder::IsInited() const
{
	return this->library != nullptr;
}

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

int FontBuilder::InitializeFont(const std::string & fontFacePath)
{
	FontInfo fi;
	FT_Face ff;
	FT_Error error;

	fi.fontFaceName = fontFacePath.substr(fontFacePath.find_last_of("/\\") + 1);
	std::string::size_type const p(fi.fontFaceName.find_last_of('.'));
	fi.fontFaceName = fi.fontFaceName.substr(0, p);

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
		MY_LOG_ERROR("Failed to initialize Font Face %s. File not supported", fi.fontFaceName.c_str());
		return -1;
	}
	else if (error)
	{
		MY_LOG_ERROR("Failed to initialize Font Face %s.", fi.fontFaceName.c_str());
		return -1;
	}

	FT_Select_Charmap(ff, FT_ENCODING_UNICODE);

	fi.fontFace = ff;
	fi.index = static_cast<int>(this->fis.size());


	this->fis.push_back(fi);

	return fi.index;

}


void FontBuilder::SetFontSizePts(FontInfo & f, int size, int dpi)
{
	//https://www.freetype.org/freetype2/docs/glyphs/glyphs-2.html
	
	if (FT_Set_Char_Size(f.fontFace, 0, size * 64, dpi, dpi))
	{
		MY_LOG_ERROR("Failed to set font size in points");
		return;
	}

	f.fontSizePixels = (size * dpi / 72); // this->fontFace->size->metrics.y_ppem;	
	f.newLineOffset = static_cast<int>(f.fontFace->size->metrics.height / 64);
}

void FontBuilder::SetFontSizePixels(FontInfo & f, int size)
{	
	//https://www.freetype.org/freetype2/docs/tutorial/step1.html

	if (FT_Set_Pixel_Sizes(f.fontFace, 0, size))
	{
		MY_LOG_ERROR("Failed to set font size in pixels\n");
		return;
	}

	//http://stackoverflow.com/questions/28009564/new-line-pixel-distance-in-freetype

	f.fontSizePixels = size;

	// get the scaled line spacing (for 48pt), also measured in 64ths of a pixel
	f.newLineOffset = static_cast<int>(f.fontFace->size->metrics.height / 64);

}


//================================================================



const std::vector<FontInfo> & FontBuilder::GetFontInfos() const
{
	return this->fis;
}

int FontBuilder::GetMaxFontPixelSize() const
{
	int fontSizePixels = std::numeric_limits<int>::min();
	for (auto & fi : this->fis)
	{
		fontSizePixels = std::max(fontSizePixels, fi.fontSizePixels);
	}

	return fontSizePixels;
}

int FontBuilder::GetMaxNewLineOffset() const
{
	int lineOffset = std::numeric_limits<int>::min();
	for (auto & fi : this->fis)
	{
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

int FontBuilder::GetTextureWidth() const
{
	return this->texPacker->GetTextureWidth();
}

int FontBuilder::GetTextureHeight() const
{
	return this->texPacker->GetTextureHeight();
}

const uint8_t * FontBuilder::GetTexture() const
{
	return this->texPacker->GetTexture();
}

void FontBuilder::Save(const std::string & fileName)
{
	this->texPacker->SaveToFile(fileName);
}

void FontBuilder::SetTightPacking()
{
	this->texPacker->SetTightPacking();
}

void FontBuilder::SetGridPacking(int binW, int binH)
{
	this->texPacker->SetGridPacking(binW, binH);
}


void FontBuilder::AddString(const UnicodeString & str)
{
	FOREACH_32_CHAR_ITERATION(c, str)
	{
		this->AddCharacter(c);
	}
}

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

void FontBuilder::AddAllAsciiNumbers()
{
	for (char c = '0'; c <= '9'; c++)
	{
		this->AddCharacter(c);
	}
}

void FontBuilder::AddCharacter(CHAR_CODE c)
{
	if (c == '\n')
	{
		return;
	}

	for (auto & fi : this->fis)
	{
		if (fi.usedGlyphs.find(c) != fi.usedGlyphs.end())
		{
			//character already exist
			this->reused.insert(c);
		}
		else
		{
			//new character
			if (this->newCodes.find(c) == this->newCodes.end())
			{
				this->newCodes.insert(c);
			}
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
	for (auto & fi : this->fis)
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
	this->texPacker->SetUnusedGlyphs(unused);


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
	
	
	auto erased = this->texPacker->GetErasedGlyphs();

	//remove unused, that were removed from texture	
	for (auto r : erased)
	{
		SAFE_DELETE_ARRAY(r.second.gi->second->rawData);

		auto & fi = this->fis[r.second.fontIndex];

		fi.glyphs.erase(r.second.gi->second);
		fi.usedGlyphs.erase(r.second.gi);		
	}



	//packing successfully finished
	//there was a space in texture and new glyphs can be added
	this->newCodes.clear();
	this->reused.clear();

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

	if (FT_Load_Glyph(fi.fontFace, ci, FT_LOAD_RENDER))
	{		
		return false;
	}

	FT_GlyphSlot glyph = fi.fontFace->glyph;


	//bitmap_left is the horizontal distance from the current pen position 
	//to the left - most border of the glyph bitmap, 
	//bitmap_top is the vertical distance from the pen position (on the baseline) 
	//to the top - most border of the glyph bitmap.
	//It is positive to indicate an upwards distance.

	GlyphInfo gInfo;
	gInfo.code = c;
	gInfo.bmpX = glyph->bitmap_left;
	gInfo.bmpY = glyph->bitmap_top;
	gInfo.bmpW = glyph->bitmap.width;
	gInfo.bmpH = glyph->bitmap.rows;
	gInfo.adv = glyph->advance.x;
	gInfo.rawData = nullptr;

	if (c > 32)
	{
		//add texture for only "non whitespace chars"

		//je to nutny kopirovat ?
		int bitmapSize = glyph->bitmap.width * glyph->bitmap.rows;
		uint8_t * textureData = new uint8_t[bitmapSize];

		for (int j = 0; j < bitmapSize; j++)
		{
			textureData[j + 0] = glyph->bitmap.buffer[j];
		}
		//---------------------

		gInfo.rawData = textureData;
	}


	fi.glyphs.push_back(gInfo);

	FontInfo::GlyphIterator lastIter = std::prev(fi.glyphs.end());

	fi.usedGlyphs[gInfo.code] = lastIter;

	return true;

}

