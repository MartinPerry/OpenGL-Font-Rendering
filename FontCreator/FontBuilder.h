#ifndef FONT_RENDERER_H
#define FONT_RENDERER_H

class TextureAtlasPack;


#include <string>
#include <stdint.h>
#include <unordered_set>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H


#include "./FontStructures.h"
#include "./TextureAtlasPack.h"

#include "./Externalncludes.h"

class FontBuilder
{
public:
	FontBuilder(const FontBuilderSettings& fs);
	~FontBuilder();

	void Release();
	bool IsInited() const;

	void SetFontSize(const std::string & fontName, const FontSize & fs, uint16_t defaultFontSizeInPx = 0);
	void SetAllFontSize(const FontSize & fs, uint16_t defaultFontSizeInPx = 0);

	bool AddString(const StringUtf8& str);
	bool AddCharacter(CHAR_CODE c);
	void AddAllAsciiLetters();
	void AddAllAsciiNumbers();
	
	void SetTightPacking();
	void SetGridPacking(uint16_t binW, uint16_t binH);

	
	const std::vector<FontInfo> & GetFontInfos() const;
	uint16_t GetMaxFontPixelHeight() const;
	uint16_t GetMaxFontPixelWidth() const;
	uint16_t GetMaxEmSize() const;
	float GetScreenScale() const;

	int16_t GetMaxNewLineOffset() const;
	int16_t GetNewLineOffsetBasedOnGlyph(CHAR_CODE c);
	FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool & exist);
	FontInfo::GlyphIterator GetGlyph(CHAR_CODE c, bool & exist, FontInfo ** usedFi);

	uint16_t GetTextureWidth() const;
	uint16_t GetTextureHeight() const;
	const uint8_t * GetTextureData() const;

	bool CreateFontAtlas();
		
	void Save(const std::string & fileName);
	
	

protected:

	static const int LETTER_BORDER_SIZE = 0;

	float screenScale;
	uint16_t screenDpi;

	FT_Library library;
	
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

	
	uint8_t * ResizeBitmap(FT_GlyphSlot glyph, FontInfo & fi) const;
	uint8_t * ResizeBitmapHermite(FT_GlyphSlot glyph, FontInfo & fi) const;

};



#endif