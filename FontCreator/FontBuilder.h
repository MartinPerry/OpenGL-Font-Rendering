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
	FontBuilder(const std::vector<Font> & fonts, RenderSettings r);
	~FontBuilder();

	void Release();
	bool IsInited() const;

	void AddString(const utf8_string & strUTF8);
	void AddCharacter(CHAR_CODE c);
	void AddAllAsciiLetters();
	void AddAllAsciiNumbers();
	
	void SetTightPacking();
	void SetGridPacking(int binW, int binH);

	
	const std::vector<FontInfo> & GetFontInfos() const;
	int GetMaxFontPixelSize() const;
	int GetMaxNewLineOffset() const;
	int GetNewLineOffsetBasedOnFirstGlyph(CHAR_CODE c) const;
	FontInfo::UsedGlyphIterator GetGlyph(CHAR_CODE c, bool & exist);

	int GetTextureWidth() const;
	int GetTextureHeight() const;
	const uint8_t * GetTexture() const;

	bool CreateFontAtlas();
		
	void Save(const std::string & fileName);
	
	

protected:

	static const int LETTER_BORDER_SIZE = 0;

	
	FT_Library library;
	

	std::vector<FontInfo> fis;
	
	std::unordered_set<CHAR_CODE> reused; //codes that were already added and are also in current string
	std::unordered_set<CHAR_CODE> newCodes; //newly added codes

	TextureAtlasPack * texPacker;
		
	int InitializeFont(const std::string & fontFacePath);
	void SetFontSizePixels(FontInfo & f, int size);
	void SetFontSizePts(FontInfo & f, int size, int dpi);

	void LoadGlyphInfo(CHAR_CODE c);
	bool LoadGlyphInfo(CHAR_CODE c, FontInfo & fi);

	

	
};



#endif