#ifndef FONT_RENDERER_H
#define FONT_RENDERER_H

class TextureAtlasPack;


#include <string>
#include <stdint.h>
#include <unordered_map>
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

	void SetFontSize(const std::string & fontName, const FontSize & fs, int defaultFontSizeInPx = 0);
	void SetAllFontSize(const FontSize & fs, int defaultFontSizeInPx = 0);

	bool AddString(const UnicodeString & str);	
	bool AddCharacter(CHAR_CODE c);
	void AddAllAsciiLetters();
	void AddAllAsciiNumbers();
	
	void SetTightPacking();
	void SetGridPacking(int binW, int binH);

	
	const std::vector<FontInfo> & GetFontInfos() const;
	int GetMaxFontPixelHeight() const;
	int GetMaxFontPixelWidth() const;
	int GetMaxEmSize() const;

	int GetMaxNewLineOffset() const;
	int GetNewLineOffsetBasedOnGlyph(CHAR_CODE c);	
	FontInfo::GlyphLutIterator GetGlyph(CHAR_CODE c, bool & exist);
	FontInfo::GlyphLutIterator GetGlyph(CHAR_CODE c, bool & exist, FontInfo ** usedFi);

	int GetTextureWidth() const;
	int GetTextureHeight() const;
	const uint8_t * GetTexture() const;

	bool CreateFontAtlas();
		
	void Save(const std::string & fileName);
	
	

protected:

	static const int LETTER_BORDER_SIZE = 0;

	float screenScale;
	int screenDpi;

	FT_Library library;
	

	std::vector<FontInfo> fis;
	std::vector<uint8_t *> memoryFonts;

	std::unordered_set<CHAR_CODE> reused; //codes that were already added and are also in current string
	std::unordered_set<CHAR_CODE> newCodes; //newly added codes

	TextureAtlasPack * texPacker;
		
	int InitializeFont(const std::string & fontFacePath);
	uint8_t * LoadFontFromFile(const std::string & fontFacePath, size_t * bufSize);
	bool SetFontSizePixels(FontInfo & f, int size);
	bool SetFontSizePts(FontInfo & f, int size, int dpi);
	bool SetClosestFontSizeForBitmaps(FontInfo & f, int size);
	void UpdateBitmapFontsSizes(int maxEmSize);

	void LoadGlyphInfo(CHAR_CODE c);
	bool FillGlyphInfo(CHAR_CODE c, FontInfo & fi) const;

	
	uint8_t * ResizeBitmap(FT_GlyphSlot glyph, FontInfo & fi) const;
	uint8_t * ResizeBitmapHermite(FT_GlyphSlot glyph, FontInfo & fi) const;

};



#endif