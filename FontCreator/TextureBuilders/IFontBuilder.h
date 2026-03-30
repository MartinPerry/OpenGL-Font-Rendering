#ifndef IFONT_BUILDER_H
#define IFONT_BUILDER_H


#include "../FontStructures.h"
#include "../Externalncludes.h"

class IFontBuilder
{
public:
	IFontBuilder() = default;
	virtual ~IFontBuilder() = default;

	virtual void Release() = 0;

	virtual void SetAllFontSize(const FontSize& fs, uint16_t defaultFontSizeInPx = 0) = 0;


	virtual uint16_t GetMaxFontPixelHeight() const = 0;

	virtual int16_t GetMaxNewLineOffset() const = 0;

	virtual bool AddString(const StringUtf8& str) = 0;
	virtual bool AddCharacter(CHAR_CODE c) = 0;

	virtual FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool& exist) = 0;
	virtual FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool& exist, FontInfo** usedFi) = 0;

	virtual uint16_t GetTextureWidth() const = 0;
	virtual uint16_t GetTextureHeight() const = 0;
	virtual const uint8_t* GetTextureData() const = 0;

	virtual bool CreateFontAtlas() = 0;

};

#endif
