#ifndef TEXTURE_ATLAS_PACK_H
#define TEXTURE_ATLAS_PACK_H

#include <list>
#include <unordered_map>
#include <stdint.h>
#include <string.h>
#include <random>

#include "./FontBuilder.h"
#include "./FontStructures.h"

class TextureAtlasPack
{
public:
	typedef enum PACKING_METHOD { TIGHT, GRID } PACKING_METHOD;

	typedef struct PackedInfo
	{
		int x;
		int y;
		int width;
		int height;
		bool filled;

	} PackedInfo;

	typedef struct ErasedInfo 
	{
		FontInfo::UsedGlyphIterator gi;
		int fontIndex;
	} ErasedInfo;

	TextureAtlasPack(int w, int h, int border);
	~TextureAtlasPack();


	std::unordered_map<CHAR_CODE, PackedInfo> & GetPackedInfos();

	void SetAllGlyphs(std::vector<FontInfo> * fontInfos);
	void SetUnusedGlyphs(std::list<UnusedGlyphInfo> * unused);
	const std::unordered_map<CHAR_CODE, TextureAtlasPack::ErasedInfo> & GetErasedGlyphs();

	void SetTightPacking();
	void SetGridPacking(int binW, int binH);

	void SaveToFile(const std::string & path);

	void FillBuffer(uint8_t ** buf);
	int GetTextureWidth() const;
	int GetTextureHeight() const;
	const uint8_t * GetTexture() const;
	
	bool Pack();

	void RemoveUnusedGlyphsFromFontInfo();

private:
	
	typedef struct Node
	{
		int x;	//upper left corner X
		int y;  //upper left corner Y

		int w;
		int h;

		std::list<Node>::iterator other[2];
		std::list<Node>::iterator same;
		bool hasOthers;

	} Node;

	PACKING_METHOD method;
	

	std::list<Node> freeSpace;
	
		
	//std::list<GlyphInfo> * glyphs;
	std::vector<FontInfo> * fontInfos;
	std::list<UnusedGlyphInfo> * unused;	
	std::unordered_map<CHAR_CODE, ErasedInfo> erased;
	
	int gridBinW;
	int gridBinH;

	int w;
	int h;
	int border;
	float averageGlyphSize;

	int freePixels;
	uint8_t * rawPackedData;
	std::unordered_map<CHAR_CODE, PackedInfo> packedInfo;
	
	void Clear();
	void EraseAllUnused();
	
	bool FindEmptySpace(int spaceWidth, int spaceHeight, int * px, int * py);
	void DivideNode(const Node & empty, int spaceWidth, int spaceHeight);
	
	
	void CopyDataToTexture();
	void DrawBorder(int px, int py, int pw, int ph, uint8_t borderVal);

	bool PackGrid();
	bool PackTight();
	//bool PerPixelFit(int spaceWidth, int spaceHeight, int * px, int * py);

	bool FreeSpace(int spaceWidth, int spaceHeight, CHAR_CODE * c);

	
};



#endif