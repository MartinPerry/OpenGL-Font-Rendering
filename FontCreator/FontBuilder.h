#ifndef FONT_RENDERER_H
#define FONT_RENDERER_H

class TextureAtlasPack;

#ifdef _MSC_VER
#if defined(DEBUG)|defined(_DEBUG)
#pragma comment(lib, "freetype265MTd.lib")		
#else
#pragma comment(lib, "freetype265MT.lib")		
#endif	
#endif

#include <string>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>

#include <ft2build.h>
#include FT_FREETYPE_H


#include "./FontStructures.h"
#include "./TextureAtlasPack.h"

#include "./tinyutf8.h"

class FontBuilder
{
public:
	FontBuilder(const std::string & fontFacePath, int w, int h, int fontSize);
	~FontBuilder();

	void Release();

	void AddString(const utf8_string & strUTF8);
	void AddCharacter(CHAR_CODE c);
	void AddAllAsciiLetters();
	void AddAllAsciiNumbers();
	
	void SetTightPacking();
	void SetGridPacking(int binW, int binH);

	const std::string & GetFontFaceName() const;
	int GetFontSize();
	const FontInfo & GetFontInfo() const;
	int GetTextureWidth() const;
	int GetTextureHeight() const;
	const uint8_t * GetTexture() const;

	void CreateFontAtlas();
		
	void Save(const std::string & fileName);
	
	

protected:

	static const int LETTER_BORDER_SIZE = 0;

	
	FT_Library library;
	FT_Face fontFace;


	FontInfo fi;
	
	std::unordered_set<CHAR_CODE> reused; //codes that were already added and are also in current string
	std::unordered_set<CHAR_CODE> newCodes; //newly added codes

	TextureAtlasPack * texPacker;
		
	void Initialize(const std::string & fontFacePath);
	void SetFontSize(int size);
	void LoadGlyphInfo(CHAR_CODE c);

	

	
};



#endif