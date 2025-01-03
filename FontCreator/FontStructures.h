#ifndef FONT_STRUCTURES_H
#define FONT_STRUCTURES_H

typedef struct FT_FaceRec_*  FT_Face;

class FontBuilder;

#include <stdint.h>
#include <unordered_map>
#include <list>
#include <string>
#include <limits>
#include <optional>

typedef uint32_t CHAR_CODE;

/// <summary>
/// Info for single glyph
/// </summary>
struct GlyphInfo
{
	CHAR_CODE code;
	int fontIndex;

	//"glyph" texture size
	uint16_t bmpW;
	uint16_t bmpH;

	//glyph offset
	int16_t bmpX;
	int16_t bmpY;
	

	//bitmap raw data
	uint8_t * rawData;

	//advance to next glyph
	long adv;
	//advX = adv
	//advY = 0 for horizontal

	//long ascent;
	//long descent;

	//position in FontTexture atlas
	uint16_t tx;
	uint16_t ty;

};



/// <summary>
/// Internal font info
/// contains currently used glyphs, 
/// calculated pixel size of font, faces etc.
/// </summary>
struct FontInfo
{
	typedef std::list<GlyphInfo>::iterator GlyphIterator;
	typedef std::unordered_map<CHAR_CODE, GlyphIterator>::iterator GlyphLutIterator;

	std::string faceName;
	uint16_t maxPixelsWidth;
	uint16_t maxPixelsHeight;

	
	int16_t newLineOffset;

	//FontTexture texture;
	std::unordered_map<CHAR_CODE, GlyphIterator> glyphsLut;
	std::list<GlyphInfo> glyphs;
	

	FT_Face fontFace;
	int index;
	
	bool onlyBitmapGlyphs;
	float scaleFactor;
	

};


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
	FontSize(float value, SizeType type) noexcept  : size(value), sizeType(type) {};
			
	operator int() const noexcept { return static_cast<int>(size); };
	operator double() const noexcept { return size; };

	float size;
	SizeType sizeType;
};

inline FontSize operator "" _px(long double value) noexcept
{
	return FontSize(static_cast<float>(value), FontSize::SizeType::px);
};

inline FontSize operator "" _px(unsigned long long value) noexcept
{
	return FontSize(static_cast<float>(value), FontSize::SizeType::px);
};

inline FontSize operator "" _pt(long double value) noexcept
{
	return FontSize(static_cast<float>(value), FontSize::SizeType::pt);
};

inline FontSize operator "" _pt(unsigned long long value) noexcept
{
	return FontSize(static_cast<float>(value), FontSize::SizeType::pt);
};

inline FontSize operator "" _em(long double value) noexcept
{
	return FontSize(static_cast<float>(value), FontSize::SizeType::em);
};

inline FontSize operator "" _em(unsigned long long value) noexcept
{
	return FontSize(static_cast<float>(value), FontSize::SizeType::em);
};




/// <summary>
/// Settings of single font
/// </summary>
struct Font
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

};

/// <summary>
/// Color representation
/// </summary>
struct Color
{
	float r, g, b, a;

	Color() noexcept :
		Color(1.0f, 1.0f, 1.0f, 1.0f)
	{}

	Color(float r, float g, float b, float a) noexcept :
		r(r), g(g), b(b), a(a)
	{}

	bool IsSame(const Color& c) const noexcept
	{
		return (c.r == r) && (c.g == g) &&
			(c.b == b) && (c.a == a);
	}
};

/// <summary>
/// Render settings for font renderer
/// </summary>
struct RenderSettings
{
	uint16_t deviceW;
	uint16_t deviceH;

	bool useTextureLinearFilter = false;
};


struct FontBuilderSettings
{
	std::vector<Font> fonts;

	uint16_t textureW;
	uint16_t textureH;

	uint16_t screenDpi = 0;

	//how many times is resolution bigger than display pts units
	//used on iPhones
	float screenScale = 1.0f;

};

/// <summary>
/// If background color is set to value,
/// entire rendering has one same background with this color
/// If std::nullopt is provided, each string can have a different colot
/// </summary>
struct BackgroundSettings 
{
	std::optional<Color> color = std::nullopt;
	float padding = 0.0f;
	float cornerRadius = 0.0f;
	bool shadow = false;
};

/// <summary>
/// Simple AABB
/// </summary>
struct AABB
{
	float minX;
	float maxX;

	float minY;
	float maxY;


	AABB() noexcept :
		minX(static_cast<float>(std::numeric_limits<int>::max())),
		minY(static_cast<float>(std::numeric_limits<int>::max())),
		maxX(static_cast<float>(std::numeric_limits<int>::min())),
		maxY(static_cast<float>(std::numeric_limits<int>::min()))
	{}

	bool IsEmpty() const noexcept
	{
		return (minX == static_cast<float>(std::numeric_limits<int>::max()));
	}

	float GetWidth() const noexcept
	{
		return maxX - minX;
	}

	float GetHeight() const noexcept
	{
		return maxY - minY;
	}

	void GetCenter(float& x, float& y) const noexcept
	{
		x = minX + this->GetWidth() * 0.5f;
		y = minY + this->GetHeight() * 0.5f;
	}


	void Update(int x, int y, int w, int h) noexcept
	{
		this->Update(static_cast<float>(x), static_cast<float>(y),
			static_cast<float>(w), static_cast<float>(h));
	}

	void Update(float x, float y, float w, float h) noexcept
	{
		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (x + w > maxX) maxX = x + w;
		if (y + h > maxY) maxY = y + h;
	}

	void UnionWithOffset(const AABB& b, float xOffset) noexcept
	{
		minX = std::min(minX, b.minX + xOffset);
		minY = std::min(minY, b.minY);

		maxX = std::max(maxX, b.maxX + xOffset);
		maxY = std::max(maxY, b.maxY);
	}

	bool Intersect(const AABB& bb) const noexcept
	{
		if (bb.minX > maxX) return false;
		if (bb.minY > maxY) return false;

		if (bb.maxX < minX) return false;
		if (bb.maxY < minY) return false;

		return true;
	};

	bool IsInside(float x, float y) const noexcept
	{
		return ((x > minX) && (x < maxX) &&
			(y > minY) && (y < maxY));
	};

};


#endif
