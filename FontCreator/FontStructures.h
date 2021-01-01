#ifndef FONT_STRUCTURES_H
#define FONT_STRUCTURES_H

typedef struct FT_FaceRec_*  FT_Face;

class FontBuilder;

#include <stdint.h>
#include <unordered_map>
#include <list>
#include <string>


typedef uint32_t CHAR_CODE;

/// <summary>
/// Info for single glyph
/// </summary>
typedef struct GlyphInfo
{
	CHAR_CODE code;
	int fontIndex;

	//"glyph" texture size
	int bmpW;
	int bmpH;

	//glyph offset
	int bmpX;
	int bmpY;
	

	//bitmap raw data
	uint8_t * rawData;

	//advance to next glyph
	long adv;
	//advX = adv
	//advY = 0 for horizontal

	//long ascent;
	//long descent;

	//position in FontTexture atlas
	int tx;
	int ty;

} GlyphInfo;



/// <summary>
/// Internal font info
/// contains currently used glyphs, 
/// calculated pixel size of font, faces etc.
/// </summary>
typedef struct FontInfo
{
	typedef std::list<GlyphInfo>::iterator GlyphIterator;
	typedef std::unordered_map<CHAR_CODE, GlyphIterator>::iterator GlyphLutIterator;

	std::string faceName;
	int maxPixelsWidth;
	int maxPixelsHeight;

	
	int newLineOffset;

	//FontTexture texture;
	std::unordered_map<CHAR_CODE, GlyphIterator> glyphsLut;
	std::list<GlyphInfo> glyphs;
	

	FT_Face fontFace;
	int index;
	
	bool onlyBitmapGlyphs;
	double scaleFactor;
	

} FontInfo;


/// <summary>
/// Font size helper struct
/// It holds size itself (auto int conversion operator)
/// and also size type (px, pt, em). This is used inside FontBuilder to calculate
/// real font size in pixels (eg. pt are recalculated using DPI to pixels) 
/// 
/// See some info:
/// https://css-tricks.com/css-font-size/
/// </summary>
struct FontSize
{
	enum class SizeType { px, pt, em };
	
	FontSize() noexcept : size(12), sizeType(SizeType::px) {};
	FontSize(double value, SizeType type) noexcept  : size(value), sizeType(type) {};
			
	operator int() const noexcept { return static_cast<int>(size); };
	operator double() const noexcept { return size; };

	double size;
	SizeType sizeType;
};

inline FontSize operator "" _px(long double value) noexcept
{
	return FontSize(value, FontSize::SizeType::px);
};

inline FontSize operator "" _px(unsigned long long value) noexcept
{
	return FontSize(static_cast<double>(value), FontSize::SizeType::px);
};

inline FontSize operator "" _pt(long double value) noexcept
{
	return FontSize(value, FontSize::SizeType::pt);
};

inline FontSize operator "" _pt(unsigned long long value) noexcept
{
	return FontSize(static_cast<double>(value), FontSize::SizeType::pt);
};

inline FontSize operator "" _em(long double value) noexcept
{
	return FontSize(value, FontSize::SizeType::em);
};

inline FontSize operator "" _em(unsigned long long value) noexcept
{
	return FontSize(static_cast<double>(value), FontSize::SizeType::em);
};




/// <summary>
/// Settings of single font
/// </summary>
typedef struct Font
{
	std::string name;
	FontSize size;
	int defaultFontSizeInPx; //default size of font - used if FontSize is in em

	Font(std::string_view name, FontSize size) : 
		name(name), 
		size(size), 
		defaultFontSizeInPx(0)
	{}

	Font() :
		name(""), 
		size(0_pt), 
		defaultFontSizeInPx(0)
	{}

} Font;



/// <summary>
/// Render settings for font renderer
/// </summary>
struct RenderSettings
{
	int deviceW;
	int deviceH;

	bool useTextureLinearFilter = false;
};


struct FontBuilderSettings
{
	std::vector<Font> fonts;

	int textureW;
	int textureH;

	int screenDpi = 0;

	//how many times is resolution bigger than displey pts units
	//used on iPhones
	float screenScale = 1.0;

};




#endif
