#ifndef CUSTOM_IMAGE_FONT_BUILDER_H
#define CUSTOM_IMAGE_FONT_BUILDER_H

class TextureAtlasPack;

#include <unordered_set>
#include <vector>
#include <string>
#include <optional>

#include "../FontStructures.h"
#include "../Externalncludes.h"

#include "./IFontBuilder.h"

struct CustomGlyph
{	
	CHAR_CODE c;
	std::string fileName;	

	uint16_t w = 0;
	uint16_t h = 0;
	std::optional<Font> referenceFont = std::nullopt;
};

class CustomImageFontBuilder : public IFontBuilder
{
public:
	CustomImageFontBuilder(const std::vector<CustomGlyph>& glyphsData, 
		const IFontBuilderSettings& fs);
	~CustomImageFontBuilder();

	void Release() override;

	void SetAllFontSize(const FontSize& fs, uint16_t defaultFontSizeInPx = 0) override;

	bool AddString(const StringUtf8& str) override;
	bool AddCharacter(CHAR_CODE c) override;

	uint16_t GetMaxFontPixelHeight() const override;

	int16_t GetMaxNewLineOffset() const override;

	GlyphInfo* GetGlyph(CHAR_CODE c) override;
	GlyphInfo* GetGlyph(CHAR_CODE c, FontInfo** usedFi) override;

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

	void InitializeFont(const IFontBuilderSettings& fs);
	void BuildSizes(const IFontBuilderSettings& fs);

	GlyphInfo* LoadGlyphInfo(CHAR_CODE c);
	GlyphInfo* FillGlyphInfo(CHAR_CODE c, CustomGlyph& g);

	uint8_t* ResizeBitmap(const std::vector<uint8_t>& buffer, uint16_t bufW, uint16_t bufH,
		uint16_t finalW, uint16_t finalH) const;
	uint8_t* ResizeBitmapHermite(const std::vector<uint8_t>& buffer, uint16_t bufW, uint16_t bufH,
		uint16_t finalW, uint16_t finalH) const;
};

#endif