#ifndef CUSTOM_IMAGE_FONT_BUILDER_H
#define CUSTOM_IMAGE_FONT_BUILDER_H

class TextureAtlasPack;

#include <unordered_set>
#include <vector>
#include <string>

#include "../FontStructures.h"
#include "../Externalncludes.h"

#include "./IFontBuilder.h"

struct CustomGlyph
{
	CHAR_CODE c;
	std::string fileName;	
};

class CustomImageFontBuilder : public IFontBuilder
{
public:
	CustomImageFontBuilder(const std::vector<CustomGlyph>& glyphsData);
	~CustomImageFontBuilder();

	void Release() override;

	void SetAllFontSize(const FontSize& fs, uint16_t defaultFontSizeInPx = 0) override;

	bool AddString(const StringUtf8& str) override;
	bool AddCharacter(CHAR_CODE c) override;

	uint16_t GetMaxFontPixelHeight() const override;

	int16_t GetMaxNewLineOffset() const override;

	FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool& exist) override;
	FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool& exist, FontInfo** usedFi) override;

	uint16_t GetTextureWidth() const override;
	uint16_t GetTextureHeight() const override;
	const uint8_t* GetTextureData() const override;

	bool CreateFontAtlas() override;

protected:
	
	static const int LETTER_BORDER_SIZE = 0;

	HashMap<CHAR_CODE, CustomGlyph> glyphsData;
	std::vector<FontInfo> customFi;

	std::unordered_set<CHAR_CODE> reused; //codes that were already added and are also in current string
	std::unordered_set<CHAR_CODE> newCodes; //newly added codes

	TextureAtlasPack* texPacker;

	void InitializeFont();

	GlyphInfo* LoadGlyphInfo(CHAR_CODE c);
	GlyphInfo* FillGlyphInfo(CHAR_CODE c, CustomGlyph& g);

	uint8_t* ResizeBitmap(const std::vector<uint8_t>& buffer, uint16_t w, uint16_t h) const;
	uint8_t* ResizeBitmapHermite(const std::vector<uint8_t>& buffer, uint16_t w, uint16_t h) const;
};

#endif