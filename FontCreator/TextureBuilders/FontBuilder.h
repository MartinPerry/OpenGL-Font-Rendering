#ifndef FONT_BUILDER_H
#define FONT_BUILDER_H

class TextureAtlasPack;


#include <string>
#include <stdint.h>
#include <unordered_set>
#include <vector>

#include <ft2build.h>
#include <freetype/ftstroke.h>
#include FT_FREETYPE_H

#include "../FontStructures.h"
#include "../Externalncludes.h"

#include "./IFontBuilder.h"
#include "./TextureAtlasPack.h"

class FontBuilder : public IFontBuilder
{
public:
	FontBuilder(const FontBuilderSettings& fs);
	~FontBuilder();

	void Release() override;
	bool IsInited() const;
	
	void SetStrokeSize(int strokeSize);
	void SetFontSize(const std::string & fontName, const FontSize & fs, uint16_t defaultFontSizeInPx = 0);
	void SetAllFontSize(const FontSize & fs, uint16_t defaultFontSizeInPx = 0) override;

	bool AddString(const StringUtf8& str) override;
	bool AddCharacter(CHAR_CODE c) override;
	void AddAllAsciiLetters();
	void AddAllAsciiNumbers();
	
	void SetTightPacking();
	void SetGridPacking(uint16_t binW, uint16_t binH);

	
	const std::vector<FontInfo> & GetFontInfos() const;
	uint16_t GetMaxFontPixelHeight() const override;
	uint16_t GetMaxFontPixelWidth() const;
	uint16_t GetMaxEmSize() const;
	float GetScreenScale() const;

	int16_t GetMaxNewLineOffset() const override;
	int16_t GetNewLineOffsetBasedOnGlyph(CHAR_CODE c);
	FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool & exist) override;
	FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool & exist, FontInfo ** usedFi) override;

	uint16_t GetTextureWidth() const override;
	uint16_t GetTextureHeight() const override;
	const uint8_t * GetTextureData() const override;

	bool CreateFontAtlas() override;
		
	void Save(const std::string & fileName);
	
	

protected:

	static const int LETTER_BORDER_SIZE = 0;

	float screenScale;
	uint16_t screenDpi;
	int sdfSpread; //if SDF is used, value > 0

	FT_Library library;
	FT_Stroker stroker;
	
	std::vector<FontInfo> fis;
		

	std::unordered_set<CHAR_CODE> reused; //codes that were already added and are also in current string
	std::unordered_set<CHAR_CODE> newCodes; //newly added codes

	TextureAtlasPack * texPacker;
		
	int InitializeFont(const std::string & fontFacePath);	
	bool SetFontSizePixels(FontInfo & f, uint16_t size);
	bool SetFontSizePts(FontInfo & f, uint16_t size, uint16_t dpi);
	bool SetClosestFontSizeForBitmaps(FontInfo & f, uint16_t size);
	void UpdateBitmapFontsSizes(uint16_t maxEmSize);

	GlyphInfo* LoadGlyphInfo(CHAR_CODE c);
	GlyphInfo* FillGlyphInfo(CHAR_CODE c, FontInfo & fi) const;

	bool FillGlyphGraphics(FontInfo& fi, FT_UInt ci,
		FT_Bitmap& glyphBmp, int& glyphLeft, int& glyphTop, int& advanceX) const;
	
	uint8_t * ResizeBitmap(FT_Bitmap glyphBmp, FontInfo & fi) const;
	uint8_t * ResizeBitmapHermite(FT_Bitmap glyphBmp, FontInfo & fi) const;

};



#endif