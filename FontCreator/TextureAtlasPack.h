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
	enum class PACKING_METHOD { TIGHT, GRID };

	struct PackedInfo
	{
		uint16_t x;
		uint16_t y;
		uint16_t width;
		uint16_t height;
		bool filled;

	};

	
	TextureAtlasPack(uint16_t w, uint16_t h, uint16_t border);
	~TextureAtlasPack();


	std::unordered_map<CHAR_CODE, PackedInfo> & GetPackedInfos();

	void SetAllFontInfos(std::vector<FontInfo> * fontInfos);
	void SetUnusedGlyphs(std::list<FontInfo::GlyphLutIterator> * unused);
	const std::unordered_map<CHAR_CODE, FontInfo::GlyphLutIterator> & GetErasedGlyphs();

	void SetTightPacking();
	void SetGridPacking(uint16_t binW, uint16_t binH);

	void SaveToFile(const std::string & path);

	//void FillBuffer(uint8_t ** buf);
	uint16_t GetTextureWidth() const;
	uint16_t GetTextureHeight() const;
	const uint8_t * GetTextureData() const;
	
	bool Pack();

	void RemoveErasedGlyphsFromFontInfo();

	friend class FontBuilder;

private:
	
	struct Node
	{
		uint16_t x;	//upper left corner X
		uint16_t y;  //upper left corner Y

		uint16_t w;
		uint16_t h;

		std::list<Node>::iterator other[2];
		std::list<Node>::iterator same;
		bool hasOthers;

		Node(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept :
			x(x), y(y), w(w), h(h), hasOthers(false)
		{};

		Node() noexcept :
			x(0), y(0), w(0), h(0), hasOthers(false)
		{};

	};

	PACKING_METHOD method;
	

	std::list<Node> freeSpace;
	std::mt19937 mt;
	std::uniform_int_distribution<int> uniDist01;
		
	//std::list<GlyphInfo> * glyphs;
	std::vector<FontInfo> * fontInfos;
	std::list<FontInfo::GlyphLutIterator> * unused;
	std::unordered_map<CHAR_CODE, FontInfo::GlyphLutIterator> erased;
	
	uint16_t gridBinW;
	uint16_t gridBinH;

	uint16_t w;
	uint16_t h;
	uint16_t border;
	float averageGlyphSize;

	int freePixels;
	uint8_t * rawPackedData;
	std::unordered_map<CHAR_CODE, PackedInfo> packedInfo;
	
	void Clear();
	void EraseAllUnused();	
	void AddToErased(int fontIndex, CHAR_CODE c);

	bool FindEmptySpace(int spaceWidth, int spaceHeight, uint16_t* px, uint16_t* py);
	void DivideNode(const Node & empty, uint16_t spaceWidth, uint16_t spaceHeight);
	
	
	void CopyDataToTexture();
	void DrawBorder(int px, int py, int pw, int ph, uint8_t borderVal);

	bool PackGrid();
	bool PackTight();
	//bool PerPixelFit(int spaceWidth, int spaceHeight, int * px, int * py);

	bool FreeSpace(int spaceWidth, int spaceHeight, CHAR_CODE * c);

	
};



#endif