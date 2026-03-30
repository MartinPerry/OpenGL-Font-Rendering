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

	//todo
	this->texPacker = new TextureAtlasPack(256, 256, LETTER_BORDER_SIZE);

}

CustomImageFontBuilder::~CustomImageFontBuilder()
{
	this->Release();
}

void CustomImageFontBuilder::Release()
{
	for (auto& [c, gi] : glyphs)
	{
		SAFE_DELETE_ARRAY(gi.rawData);
	}
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
	
	if (glyphs.find(c) != glyphs.end())
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
	return 0;
}

int16_t CustomImageFontBuilder::GetMaxNewLineOffset() const
{
	return 0;
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

	auto it = glyphs.find(c);
	if (it != glyphs.end())
	{
		*usedFi = nullptr;
		exist = true;
		return it;
	}

	*usedFi = nullptr;
	return glyphs.end();
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

	//todo

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
	auto it = glyphs.find(c);
	if (it != glyphs.end())
	{
		//glyph already exist
		return &it->second;
	}


	std::vector<unsigned char> png;
	std::vector<unsigned char> buffer; //the raw pixels
	unsigned width, height;
	lodepng::State state; //optionally customize this one
	
	lodepng::load_file(png, g.fileName); //load the image file with given filename
	auto error = lodepng::decode(buffer, width, height, state, png);

	//if there's an error, display it
	if (error)
	{
		MY_LOG_ERROR("decoder error %d: %s", error,  lodepng_error_text(error));
		return nullptr;
	}


	
	//glyphLeft is the horizontal distance from the current pen position 
	//to the left - most border of the glyph bitmap, 
	//glyphTop is the vertical distance from the pen position (on the baseline) 
	//to the top - most border of the glyph bitmap.
	//It is positive to indicate an upwards distance.

	GlyphInfo gInfo;
	gInfo.code = c;
	gInfo.fontIndex = -1;
	gInfo.bmpX = 0;
	gInfo.bmpY = 0;
	gInfo.bmpW = width;
	gInfo.bmpH = height;
	gInfo.adv = 0;
	gInfo.rawData = nullptr;

	if (c > 32)
	{
		//add texture for only "non whitespace chars"
		/*
		if (fi.scaleFactor != 1.0)
		{
			gInfo.rawData = this->ResizeBitmapHermite(glyphBmp, fi);
		}
		else
		*/
		{
			int bitmapSize = width * height;
			uint8_t* textureData = new uint8_t[bitmapSize];

			std::copy(buffer.data(),
				buffer.data() + bitmapSize,
				textureData);
			
			gInfo.rawData = textureData;
		}
	}

	auto tmp = glyphs.try_emplace(c, std::move(gInfo));

	return &tmp.first->second;

}