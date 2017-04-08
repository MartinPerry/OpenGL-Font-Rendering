#include "./FontBuilder.h"


#include "./TextureAtlasPack.h"


#include "./Externalncludes.h"



//http://www.freetype.org/freetype2/documentation.html
//http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

FontBuilder::FontBuilder(Font f)
{
	
	this->texPacker = new TextureAtlasPack(f.textureWidth, f.textureHeight, LETTER_BORDER_SIZE);
	this->texPacker->SetTightPacking();
	//this->texPacker->SetGridPacking(fontSize, fontSize);


	this->library = nullptr;
	this->fontFace = nullptr;
	this->inited = false;

	this->Initialize(f.name);

	if (f.screenDpi == 0)
	{
		this->SetFontSizePixels(f.size);
	}
	else 
	{
		this->SetFontSizePts(f.size, f.screenDpi);
	}
}


FontBuilder::~FontBuilder()
{
	SAFE_DELETE(this->texPacker);

	this->Release();
}

void FontBuilder::Release()
{	
	FT_Done_Face(this->fontFace);
	this->fontFace = nullptr;

	FT_Done_FreeType(this->library);
	this->library = nullptr;
	
	for (auto & c : this->fi.glyphs)
	{
		SAFE_DELETE_ARRAY(c.rawData);		
	}

	this->inited = false;
}

//================================================================
// Initialization
//================================================================

bool FontBuilder::IsInited() const
{
	return this->inited;
}


void FontBuilder::Initialize(const std::string & fontFacePath)
{
	this->Release();

	if (FT_Init_FreeType(&this->library))
	{
		printf("Failed to initialize FreeType library.");
		return;
	}

	this->fi.fontFaceName = fontFacePath.substr(fontFacePath.find_last_of("/\\") + 1);
	std::string::size_type const p(this->fi.fontFaceName.find_last_of('.'));
	this->fi.fontFaceName = this->fi.fontFaceName.substr(0, p);


	//FT_Error error = FT_New_Memory_Face(this->library, this->fontData, bufSize, 0, &this->fontFace);
	FT_Error error = FT_New_Face(this->library, fontFacePath.c_str(), 0, &this->fontFace);
	if (error == FT_Err_Unknown_File_Format)
	{
		printf("Failed to initialize Font Face %s. File not supported", fi.fontFaceName.c_str());
		return;
	}
	else if (error)
	{
		printf("Failed to initialize Font Face %s.", fi.fontFaceName.c_str());
		return;
	}

	FT_Select_Charmap(this->fontFace, FT_ENCODING_UNICODE);	
	this->inited = true;
}


void FontBuilder::SetFontSizePts(int size, int dpi)
{
	//https://www.freetype.org/freetype2/docs/glyphs/glyphs-2.html
	
	if (FT_Set_Char_Size(this->fontFace, 0, size * 64, dpi, dpi))
	{
		printf("Failed to set font size in points\n");
		return;
	}

	this->fi.fontSizePixels = (size * dpi / 72); // this->fontFace->size->metrics.y_ppem;

	this->fi.newLineOffset = this->fontFace->size->metrics.height / 64;
}

void FontBuilder::SetFontSizePixels(int size)
{
	
	//https://www.freetype.org/freetype2/docs/tutorial/step1.html

	

	if (FT_Set_Pixel_Sizes(this->fontFace, 0, size))
	{
		printf("Failed to set font size in pixels\n");
		return;
	}

	//http://stackoverflow.com/questions/28009564/new-line-pixel-distance-in-freetype

	this->fi.fontSizePixels = size;

	// get the scaled line spacing (for 48pt), also measured in 64ths of a pixel
	this->fi.newLineOffset = this->fontFace->size->metrics.height / 64;

}


//================================================================

const std::string & FontBuilder::GetFontFaceName() const
{
	return this->fi.fontFaceName;
}

int FontBuilder::GetFontSizePixels()
{
	return this->fi.fontSizePixels;
}

const FontInfo & FontBuilder::GetFontInfo() const
{
	return this->fi;
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


void FontBuilder::AddString(const utf8_string & strUTF8)
{
	for (auto c : strUTF8)
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

	if (this->fi.usedGlyphs.find(c) != this->fi.usedGlyphs.end())
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
	std::list<FontInfo::UsedGlyphIterator> unused;
	for (FontInfo::UsedGlyphIterator it = this->fi.usedGlyphs.begin();
		it != this->fi.usedGlyphs.end();
		it++)
	{
		if (this->reused.find(it->first) == this->reused.end())
		{
			unused.push_back(it);
		}
	}


	//try to add new codes to texture
	this->texPacker->SetAllGlyphs(&this->fi.glyphs);
	this->texPacker->SetUnusedGlyphs(unused);


	if (this->texPacker->Pack() == false)
	{
		printf("Problem - no space, but we need all characters\n");
		
		//here do something if packing failed

		//BUT we still need to continue and erase "unused" glyphs from this->fi
		//because before packing run out of space, it could remove glyphs from
		//texture
	}
	
	
	unused = this->texPacker->GetErasedGlyphs();

	//remove unused, that were removed from texture
	for (auto r : unused)
	{
		SAFE_DELETE_ARRAY(r->second->rawData);

		this->fi.glyphs.erase(r->second);
		this->fi.usedGlyphs.erase(r);
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
	if (this->fi.usedGlyphs.find(c) != this->fi.usedGlyphs.end())
	{
		//glyph already exist
		return;
	}

	FT_UInt ci = FT_Get_Char_Index(this->fontFace, c);

	if (ci == 0)
	{
		printf("Character %lu not found\n", c);
		return;
	}

	if (FT_Load_Glyph(this->fontFace, ci, FT_LOAD_RENDER))
	{
		printf("Character %lu not found\n", c);
		return;
	}

	FT_GlyphSlot glyph = this->fontFace->glyph;


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
	

	this->fi.glyphs.push_back(gInfo);

	FontInfo::GlyphIterator lastIter = std::prev(this->fi.glyphs.end());
	
	this->fi.usedGlyphs[gInfo.code] = lastIter;
		

}

