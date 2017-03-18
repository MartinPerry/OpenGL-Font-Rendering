#ifndef _FONT_STRUCTURES_H_
#define _FONT_STRUCTURES_H_

#include <stdint.h>
#include <unordered_map>


typedef unsigned long CHAR_CODE;

typedef struct GlyphInfo
{
	CHAR_CODE code;

	//"glyph" texture size
	int bmpW;
	int bmpH;

	//glyph offset
	int bmpX;
	int bmpY;
	

	//bitmap raw data
	uint8_t * rawData;

	//advance to next glyph
	long adv;
	//advX = adv
	//advY = 0 for horizontal

	//long ascent;
	//long descent;

	//position in FontTexture atlas
	int tx;
	int ty;

} GlyphInfo;




typedef struct FontInfo
{
	typedef std::list<GlyphInfo>::iterator GlyphIterator;
	typedef std::unordered_map<CHAR_CODE, GlyphIterator>::iterator UsedGlyphIterator;

	std::string fontFaceName;
	int fontSize;

	int maxAscent;
	int maxDescent;
	int newLineOffset;

	//FontTexture texture;
	std::unordered_map<CHAR_CODE, GlyphIterator> usedGlyphs;
	std::list<GlyphInfo> glyphs;
	

} FontInfo;

#endif
