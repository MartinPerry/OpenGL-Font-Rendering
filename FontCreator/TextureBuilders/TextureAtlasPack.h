#ifndef TEXTURE_ATLAS_PACK_H
#define TEXTURE_ATLAS_PACK_H

#include <list>
#include <stdint.h>
#include <string.h>
#include <random>

#include "../Externalncludes.h"
#include "../FontStructures.h"

#include "./FontBuilder.h"

class TextureAtlasPack
{
public:
	enum class PACKING_METHOD : uint8_t { TIGHT, GRID };

	struct PackedInfo
	{
		uint16_t x;
		uint16_t y;
		uint32_t packed;

		//width, height -> max 1024x1024

		uint16_t width() const
		{
			return packed & 0x3FF;
		}

		uint16_t height() const
		{
			return (packed >> 10) & 0x3FF;
		}

		bool filled() const
		{
			return (packed >> 20) & 1;
		}

		void setWidth(uint16_t v)
		{
			packed = (packed & ~(0x3FFu)) | (v & 0x3FFu);
		}

		void setHeight(uint16_t v)
		{
			packed = (packed & ~(0x3FFu << 10)) | ((v & 0x3FFu) << 10);
		}

		void setFilled(bool v)
		{
			packed = (packed & ~(1u << 20)) | (uint32_t(v) << 20);
		}
	};

	
	TextureAtlasPack(uint16_t w, uint16_t h, uint16_t border, uint8_t channelsCount = 1);	
	~TextureAtlasPack();

	//const HashMap<CHAR_CODE, FontInfo*> & GetErasedGlyphs();
	//HashMap<CHAR_CODE, PackedInfo> & GetPackedInfos();

	//void SetAllFontInfos(std::vector<FontInfo> * fontInfos);
	void AddFontInfos(std::vector<FontInfo>& fontInfos);
	void AddFontInfo(FontInfo* fontInfo);
	void RemoveFontInfo(FontInfo* fontInfo);
	void SetUnusedGlyphs(std::list<FontInfo::GlyphIterator> * unused);
	
	void SetTightPacking();
	void SetGridPacking(uint16_t binW, uint16_t binH);

	void SaveToFile(const std::string & path);

	//void FillBuffer(uint8_t ** buf);
	uint16_t GetTextureWidth() const;
	uint16_t GetTextureHeight() const;
	uint16_t GetFontBorder() const;
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
	using CHAR_ID = uint64_t;

	std::list<Node> freeSpace;
	std::mt19937 mt;
	std::uniform_int_distribution<int> uniDist01;
			
	std::vector<FontInfo *> fontInfos;
	std::list<FontInfo::GlyphIterator>* unused;
	HashMap<CHAR_ID, FontInfo*> erased;
	
	uint16_t gridBinW;
	uint16_t gridBinH;

	uint16_t w;
	uint16_t h;
	uint16_t border;
	uint8_t channelsCount;
	float averageGlyphSize;

	int freePixels;
	uint8_t * rawPackedData;	
	HashMap<CHAR_ID, PackedInfo> packedInfo;
	
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

	std::optional<PackedInfo> FreeSpace(int spaceWidth, int spaceHeight);

	
};



#endif