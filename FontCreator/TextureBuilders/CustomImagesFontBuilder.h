#ifndef CUSTOM_IMAGE_FONT_BUILDER_H
#define CUSTOM_IMAGE_FONT_BUILDER_H

#include <unordered_set>
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

	bool AddString(const StringUtf8& str) override;
	bool AddCharacter(CHAR_CODE c) override;

	bool CreateFontAtlas() override;

protected:

	std::unordered_map<CHAR_CODE, CustomGlyph> glyphsData;
	std::unordered_map<CHAR_CODE, GlyphInfo> glyphs;

	std::unordered_set<CHAR_CODE> reused; //codes that were already added and are also in current string
	std::unordered_set<CHAR_CODE> newCodes; //newly added codes


	GlyphInfo* LoadGlyphInfo(CHAR_CODE c);
	GlyphInfo* FillGlyphInfo(CHAR_CODE c, CustomGlyph& g);
};

#endif