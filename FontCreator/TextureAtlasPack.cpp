#include "./TextureAtlasPack.h"


//http://www.blackpawn.com/texts/lightmaps/default.html

TextureAtlasPack::TextureAtlasPack(int w, int h, int border) : 
	fontInfos(nullptr), 
	unused(nullptr), 
	w(w), 
	h(h), 
	border(border), 
	method(PACKING_METHOD::TIGHT),
	freePixels(w * h),
	averageGlyphSize(2500),
	gridBinW(0), gridBinH(0)
{
		
	std::random_device rd;
	this->mt = std::mt19937(rd());
	this->uniDist01 = std::uniform_int_distribution<int>(0, 1);

	this->rawPackedData = new uint8_t[w * h];
	memset(this->rawPackedData, 0, sizeof(uint8_t) * w * h);

		
	this->freeSpace.emplace_back(0, 0, w, h);
}


TextureAtlasPack::~TextureAtlasPack()
{
	SAFE_DELETE_ARRAY(this->rawPackedData);
}


std::unordered_map<CHAR_CODE, TextureAtlasPack::PackedInfo> & TextureAtlasPack::GetPackedInfos()
{
	return this->packedInfo;
}

void TextureAtlasPack::SaveToFile(const std::string & path)
{
	//save image as PNG
	
	lodepng::encode(path.c_str(), this->rawPackedData, this->w, this->h, 
		LodePNGColorType::LCT_GREY, 8 * sizeof(uint8_t));	
}


void TextureAtlasPack::SetTightPacking()
{
	this->method = PACKING_METHOD::TIGHT;
	this->Clear();
}

void TextureAtlasPack::SetGridPacking(int binW, int binH)
{
	this->gridBinW = binW;
	this->gridBinH = binH;

	this->method = PACKING_METHOD::GRID;
	this->Clear();

	this->freeSpace.clear();

	binH += 2 * this->border;
	binW += 2 * this->border;
	

	int gridedH = this->h - this->h % binH;
	int gridedW = this->w - this->w % binW;

	
	for (int y = 0; y < gridedH; y += binH)
	{
		for (int x = 0; x < gridedW; x += binW)
		{			
			this->freeSpace.emplace_back(x, y, binW, binH);
		}
	}

}

//======================== Add textures to atlas ===========================================

void TextureAtlasPack::SetAllFontInfos(std::vector<FontInfo> * fontInfos)
{
	this->fontInfos = fontInfos;
}

void TextureAtlasPack::SetUnusedGlyphs(std::list<FontInfo::GlyphLutIterator> * unused)
{
	this->unused = unused;
}

const std::unordered_map<CHAR_CODE, FontInfo::GlyphLutIterator> & TextureAtlasPack::GetErasedGlyphs()
{
	return this->erased;
}

int TextureAtlasPack::GetTextureWidth() const
{
	return this->w;
}

int TextureAtlasPack::GetTextureHeight() const
{
	return this->h;
}


const uint8_t * TextureAtlasPack::GetTextureData() const
{
	return this->rawPackedData;
}

//======================== Create atlas ===========================================

void TextureAtlasPack::Clear()
{
	this->freePixels = w * h;
	
	this->freeSpace.clear();
		

	this->freeSpace.emplace_back(0, 0, w, h);
	
	this->packedInfo.clear();

	if (this->unused)
	{
		this->unused->clear();
	}
	this->erased.clear();
}


/// <summary>
/// Pack glyphs to texture
/// </summary>
/// <returns></returns>
bool TextureAtlasPack::Pack()
{	
	this->RemoveErasedGlyphsFromFontInfo();

	bool res = false;
	if (this->method == PACKING_METHOD::GRID)
	{
		res = this->PackGrid();
	}
	else
	{
		res = this->PackTight();
	}

	this->CopyDataToTexture();

	return res;
}

/// <summary>
/// Pack data using regular grid. 
/// Each glyph takes the same amount of space
/// </summary>
/// <returns></returns>
bool TextureAtlasPack::PackGrid()
{				
	if (this->unused->size() * this->averageGlyphSize >= (this->w * this->h) * 0.4)
	{
		//total unused space is over 40% of entire texture
		//erase all
		MY_LOG_INFO("Erasing all packed data...");
		this->EraseAllUnused();
		this->RemoveErasedGlyphsFromFontInfo();
		this->Clear();
		this->SetGridPacking(this->gridBinW, this->gridBinH);
	}

	int count = 0;

	PackedInfo info;

	for (auto & fi : *this->fontInfos)
	{		
		for (GlyphInfo & g : fi.glyphs)
		{
			if (g.code <= 32)
			{
				//do not add white-space "characters" to texture
				continue;
			}

			if (this->packedInfo.find(g.code) != this->packedInfo.end())
			{
				//glyph already in texture
				continue;
			}

			if (this->freeSpace.empty())
			{
				if (this->unused->size() <= this->erased.size())
				{					
					//all unused characters are erased
					//no more empty space
					MY_LOG_INFO("Empty space in atlas not found and cannot be freed for glyph %lu", g.code);										
					//this->AddToErased(g.fontIndex, g.code);
					return false;
				}

				//free space from unused

				CHAR_CODE removedCode;

				if (this->FreeSpace(g.bmpW, g.bmpH, &removedCode) == false)
				{
					MY_LOG_INFO("Empty space in atlas not found and cannot be freed for glyph %lu", g.code);
					//this->AddToErased(g.fontIndex, g.code);
					return false;
				}

				auto it = this->packedInfo.find(removedCode);

				info = it->second;
				this->packedInfo.erase(it);
			}
			else
			{

				const Node & empty = this->freeSpace.front();

				info.x = empty.x;
				info.y = empty.y;
				info.width = empty.w;
				info.height = empty.h;

				this->freeSpace.pop_front();
			}

			info.filled = false;

			
			g.tx = info.x + this->border;
			g.ty = info.y + this->border;

			
			count++;
			this->averageGlyphSize += g.bmpW * g.bmpH;

			this->packedInfo[g.code] = info;
		}
	}

	if (count != 0)
	{
		this->averageGlyphSize /= count;
	}

	return true;
}

/// <summary>
/// Pack glyphs to texture using "tight" packing
/// Sort data by its required "space" -> fill texture
/// </summary>
/// <returns></returns>
bool TextureAtlasPack::PackTight()
{
	for (auto & fi : *this->fontInfos)
	{
		fi.glyphs.sort(
			[](const GlyphInfo &a, GlyphInfo &b) { return a.bmpW * a.bmpH > b.bmpW * b.bmpH; }
		);
	}
	

	PackedInfo info;

	int b = (2 * this->border);

	for (auto & fi : *this->fontInfos)
	{
		for (GlyphInfo & g : fi.glyphs)
		{
			if (this->packedInfo.find(g.code) != this->packedInfo.end())
			{
				//glyph already in texture
				continue;
			}

			if (g.code <= 32)
			{
				//do not add space "character"
				continue;
			}

			int px, py;
			CHAR_CODE c;

			if (this->FindEmptySpace(g.bmpW + b, g.bmpH + b, &px, &py) == false)
			{
				if (this->FreeSpace(g.bmpW + b, g.bmpH + b, &c) == false)
				{
					MY_LOG_ERROR("Empty space in atlas not found and cannot be freed for glyph %lu", g.code);
					MY_LOG_ERROR("Requested size: %d %d", g.bmpW + b, g.bmpH + b);
					//return false;
					continue;
				}

				auto it = this->packedInfo.find(c);

				info = it->second;
				this->packedInfo.erase(it);
			}
			else
			{
				info.x = px;
				info.y = py;
				info.width = g.bmpW + b;
				info.height = g.bmpH + b;
			}
			info.filled = false;

			g.tx = px + this->border;
			g.ty = py + this->border;

			this->packedInfo[g.code] = info;
		}
	}

	return true;
}

/// <summary>
/// Real copy of glyph data to texture allocated spaces
/// </summary>
void TextureAtlasPack::CopyDataToTexture()
{
	const uint8_t BORDER_DEBUG_VALUE = 125;
	const uint8_t BORDER_EMPTY_VALUE = 0;

	for (FontInfo & fi : *this->fontInfos)
	{
		for (GlyphInfo & g : fi.glyphs)
		{
			auto it = this->packedInfo.find(g.code);
			if (it == this->packedInfo.end())
			{
				continue;
			}

			if (it->second.filled)
			{
				continue;
			}

			if ((it->second.x == -1) && (it->second.y == -1))
			{
				continue;
			}

			int px = it->second.x + this->border;
			int py = it->second.y + this->border;

			
			int origW = g.bmpW;
			
			//safety update - sometimes glyphs are bigger that bin - change size by removing
			//right / bottom lines from data - during copy, texture will reside in its bin
			if (g.bmpH > this->gridBinH)
			{
				g.bmpH = this->gridBinH;				
			}

			if (g.bmpW > this->gridBinW)
			{
				g.bmpW = this->gridBinW;				
			}

			//draw "border around letter"
			//if there was some previous letter - it will remove its remains
			this->DrawBorder(it->second.x, it->second.y,
				g.bmpW + 2 * this->border, g.bmpH + 2 * this->border, BORDER_EMPTY_VALUE);

			//copy letter data			
			int yEnd = py + g.bmpH;
			int xEnd = px + g.bmpW;
			for (int y = py, gy = 0; y < yEnd; y++, gy++)
			{
				int gyW = gy * origW;

				
				//copy line from g.rawData in range [0 - g.bmpW] to 
				//rawPackedData to range [px - px + g.bmpW]
				std::copy(g.rawData + gyW,
					g.rawData + (g.bmpW + gyW),
					this->rawPackedData + (px + y * w));

				this->freePixels -= g.bmpW;												
			}

			it->second.filled = true;

#ifdef _DEBUG			
			//debug - draw "visible borders" around letter
			this->DrawBorder(it->second.x, it->second.y,
				it->second.width, it->second.height, BORDER_DEBUG_VALUE);			
#endif

		}
	}
	
}

/// <summary>
/// Draw border around glyph
/// </summary>
/// <param name="px"></param>
/// <param name="py"></param>
/// <param name="pw"></param>
/// <param name="ph"></param>
/// <param name="borderVal"></param>
void TextureAtlasPack::DrawBorder(int px, int py, int pw, int ph, uint8_t borderVal)
{	
	if (this->border == 0)
	{
		return;
	}

	//erase top border
	for (int y = py; y < py + this->border; y++)
	{
		for (int x = px; x < px + pw; x++)
		{
			this->rawPackedData[x + y * w] = borderVal;
		}
	}

	//erase bottom border
	for (int y = py + ph - this->border; y < py + ph; y++)
	{
		for (int x = px; x < px + pw; x++)
		{
			this->rawPackedData[x + y * w] = borderVal;
		}
	}

	//erase left border
	for (int y = py; y < py + ph; y++)
	{
		for (int x = px; x < px + this->border; x++)
		{
			this->rawPackedData[x + y * w] = borderVal;
		}
	}

	//erase right border
	for (int y = py; y < py + ph; y++)
	{
		for (int x = px + pw - this->border; x < px + pw; x++)
		{
			this->rawPackedData[x + y * w] = borderVal;
		}
	}
}

/*
/// <summary>
/// Fill external buffer with current texture data
/// </summary>
/// <param name="memory"></param>
void TextureAtlasPack::FillBuffer(uint8_t ** memory)
{
	int index = 0;
	for (int y = 0; y < this->h; y++)
	{		
		for (int x = 0; x < this->w; x++)
		{
			(*memory)[index] = this->rawPackedData[index];			
			index++;
		}
	}
}
*/

/// <summary>
/// Find empty space to fit texture in
/// </summary>
/// <param name="spaceWidth"></param>
/// <param name="spaceHeight"></param>
/// <param name="px"></param>
/// <param name="py"></param>
/// <returns></returns>
bool TextureAtlasPack::FindEmptySpace(int spaceWidth, int spaceHeight, int * px, int * py)
{
	
	*px = -1;
	*py = -1;

	if (this->freePixels < spaceWidth * spaceHeight)
	{		
		return false;
	}
	
	//this->PackFreeSpace();

	size_t size = this->freeSpace.size();
	size_t index = 0;

	
	while (index < size)
	{
		index++;

		const Node & empty = this->freeSpace.front();	
		
		if ((empty.w >= spaceWidth) && (empty.h >= spaceHeight))
		{
			//remove "other division" of the "parent" tile
			//we have used one current division already
			if (empty.hasOthers)
			{				
				this->freeSpace.erase(empty.other[0]);
				this->freeSpace.erase(empty.other[1]);

				empty.same->hasOthers = false;
			}

			this->DivideNode(empty, spaceWidth, spaceHeight);

			*px = empty.x;
			*py = empty.y;
			
						
			this->freeSpace.pop_front();

			return true;
		}
		
				
		//put back empty node that was pop out						
		this->freeSpace.splice(this->freeSpace.end(), this->freeSpace, this->freeSpace.begin());								
	}
	
	return false;
}

/// <summary>
/// Divide node to three parts - one is of desired size
/// other two are remained
/// A:
/// *--------*------
/// |findNode|right|
/// |        |     |
/// *---------------
/// | down         |
/// ----------------
/// B:
/// *---------*-----
/// |findNode|right|
/// |        |     |
/// *--------|     |
/// | down   |     |
/// ----------------
/// </summary>
/// <param name="empty"></param>
/// <param name="spaceWidth"></param>
/// <param name="spaceHeight"></param>
void TextureAtlasPack::DivideNode(const Node & empty, int spaceWidth, int spaceHeight)
{
	//empty space of desired size found
	//divide space to 3 parts
	//one is desired space, rest are 2 left spaces


	std::list<Node>::iterator downItA;
	std::list<Node>::iterator rightItA;

	std::list<Node>::iterator downItB;
	std::list<Node>::iterator rightItB;

	Node nDown = { empty.x, empty.y + spaceHeight, 0, empty.h - spaceHeight };
	Node nRight = { empty.x + spaceWidth, empty.y, empty.w - spaceWidth, 0 };
	
	nDown.hasOthers = true;
	nRight.hasOthers = true;

	//random division of the space - 
	//either  down is wider is first or right is taller is first
	if (this->uniDist01(this->mt) == 0)
	{
		//append A than B to list

		nDown.w = empty.w;				
		nRight.h = spaceHeight;

		this->freeSpace.push_back(nDown);
		downItA = std::prev(this->freeSpace.end());
		this->freeSpace.push_back(nRight);
		rightItA = std::prev(this->freeSpace.end());
		
		nDown.w = spaceWidth;						
		nRight.h = empty.h;
		
		this->freeSpace.push_back(nDown);
		downItB = std::prev(this->freeSpace.end());
		this->freeSpace.push_back(nRight);
		rightItB = std::prev(this->freeSpace.end());
		
	}
	else 
	{
		//append B than A to list

		nDown.w = spaceWidth;
		nRight.h = empty.h;

		this->freeSpace.push_back(nDown);
		downItB = std::prev(this->freeSpace.end());
		this->freeSpace.push_back(nRight);
		rightItB = std::prev(this->freeSpace.end());

		nDown.w = empty.w;
		nRight.h = spaceHeight;

		this->freeSpace.push_back(nDown);
		downItA = std::prev(this->freeSpace.end());
		this->freeSpace.push_back(nRight);
		rightItA = std::prev(this->freeSpace.end());		
	}
		

	//create "tree-like" structure
	//if node from A or B is later used
	//remove the other nodes (e.g. used A_left => remove B_left, B_right 
	//and also set A_right hasOthers = false)
	downItA->same = rightItA;
	rightItA->same = downItA;

	downItB->same = rightItB;
	rightItB->same = downItB;

	downItA->other[0] = downItB;
	downItA->other[1] = rightItB;
	rightItA->other[0] = downItB;
	rightItA->other[1] = rightItB;
	
	downItB->other[0] = downItA;
	downItB->other[1] = rightItA;
	rightItB->other[0] = downItA;
	rightItB->other[1] = rightItA;

}

/// <summary>
/// Try to find free space by removing existing glyphs 
/// that are currently unused
/// </summary>
/// <param name="spaceWidth">requested width</param>
/// <param name="spaceHeight">requested height</param>
/// <param name="c">"char code" of glyph, which space will be replaced</param>
/// <returns></returns>
bool TextureAtlasPack::FreeSpace(int spaceWidth, int spaceHeight, CHAR_CODE * c)
{	
	*c = 0;

	int b = (2 * this->border);
	
	for (auto & it : *this->unused)	
	{		
		if (it->second->bmpW + b < spaceWidth) continue;
		if (it->second->bmpH + b < spaceHeight) continue;
				
		*c = it->first;

		auto jt = this->erased.find(*c);
		if (jt != this->erased.end())
		{
			//already erased - space is not free anymore
			continue;
		}

		//space find				
		//add glyph to erased				

		jt->second = it;
		//this->erased[*c] = it;
								
		return true;
	}

	return false;
}

void TextureAtlasPack::EraseAllUnused()
{	
	if (this->unused == nullptr)
	{
		return;
	}

	for (auto & it : *this->unused)
	{				
		//add glyph to erased				
		this->erased[it->first] = it;
	}

	this->unused->clear();
}

void TextureAtlasPack::AddToErased(int fontIndex, CHAR_CODE c)
{
	FontInfo & fi = (*this->fontInfos)[fontIndex];
	
	auto it = fi.glyphsLut.find(c);
	if (it != fi.glyphsLut.end())
	{
		this->erased[c] = it;
	}			
}

void TextureAtlasPack::RemoveErasedGlyphsFromFontInfo()
{	
	//remove unused, that were removed from texture	
	for (auto & r : this->erased)
	{		
		FontInfo::GlyphIterator gi = r.second->second;

		SAFE_DELETE_ARRAY(gi->rawData);
				

		auto & fi = (*this->fontInfos)[gi->fontIndex];

		fi.glyphs.erase(gi);
		fi.glyphsLut.erase(r.second);
	}

	this->erased.clear();

}

/*
bool TextureAtlasPack::PerPixelFit(int spaceWidth, int spaceHeight, int * px, int * py)
{
	*px = -1;
	*py = -1;

	if (this->freePixels < spaceWidth * spaceHeight)
	{		
		return false;
	}

	for (int y = 0; y <= (this->h - spaceHeight); y++)
	{
		for (int x = 0; x <= (this->w - spaceWidth); x++)
		{
			if (this->atlasLUT[y][x])
			{
				continue;
			}
			
			int foundFreePixels = 0;
			bool skip = false;
			//test if space is empty for texture
			for (int yy = y; (skip == false) && (yy < y + spaceHeight); yy++)
			{			
				for (int xx = x; xx < x + spaceWidth; xx++)
				{
					if (this->atlasLUT[yy][xx])
					{		
						skip = true;
						break;
					}
					foundFreePixels++;
				}
				
			}
			
		
			if (foundFreePixels == (spaceWidth * spaceHeight))			
			{
				*px = x;
				*py = y;
				return true;
			}

		}
	}


	return false;
	
}
*/
