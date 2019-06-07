#include "./StringRenderer.h"

#include <limits>

#include "./FontBuilder.h"

#include "./Externalncludes.h"

StringRenderer::StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion)
	: AbstractRenderer(fs, r, glVersion), 
	isBidiEnabled(true), 
	nlOffsetPx(0),
	spaceSizeExist(false),
	spaceSize(10)
{
}

StringRenderer::StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion,
               const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm)
    : AbstractRenderer(fs, r, glVersion, vSource, pSource, sm), 
	isBidiEnabled(true), 
	nlOffsetPx(0),
	spaceSizeExist(false), 
	spaceSize(10)
{
    
}

StringRenderer::~StringRenderer()
{
}



/// <summary>
/// Remove all added strings
/// </summary>
void StringRenderer::Clear()
{
	AbstractRenderer::Clear();
	this->strs.clear();
}

size_t StringRenderer::GetStringsCount() const
{
	return strs.size();
}

void StringRenderer::SetNewLineOffset(int offsetInPixels)
{
	this->nlOffsetPx = offsetInPixels;
}

void StringRenderer::SetBidiEnabled(bool val)
{
	this->isBidiEnabled = val;
}

void StringRenderer::AddStringCaption(const UnicodeString & str,
	double x, double y, Color color)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	this->AddStringCaption(str, xx, yy, color);
}

void StringRenderer::AddStringCaption(const UnicodeString & str,
	int x, int y, Color color)
{
    this->AddStringInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
    this->AddStringInternal(str, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
}

/// <summary>
/// Add new string to be rendered - string coordinates
/// are in percents
/// If string already exist - do not add
/// Existing string => same x, y, align, anchor, content
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void StringRenderer::AddString(const UnicodeString & str,
	double x, double y, Color color,
	TextAnchor anchor, TextAlign align)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	this->AddStringInternal(str, xx, yy, color, anchor, align, TextType::TEXT);
}

/// <summary>
/// Add new string to be rendered
/// If string already exist - do not add
/// Existing string => same x, y, align, anchor, content
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void StringRenderer::AddString(const UnicodeString & str,
	int x, int y, Color color,
	TextAnchor anchor, TextAlign align)
{
	this->AddStringInternal(str, x, y, color, anchor, align, TextType::TEXT);
}


bool StringRenderer::AddStringInternal(const UnicodeString & str,
	int x, int y, Color color,
	TextAnchor anchor, TextAlign align, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::DOWN)
	{
		y = this->rs.deviceH - y;
	}

	UnicodeString uniStr = (this->isBidiEnabled) ? BIDI(str) : str;
	

   for (StringInfo & s : this->strs)
    {
        if ((s.x == x) && (s.y == y) &&
            (s.align == align) && (s.anchor == anchor) && (s.type == type))
        {
                        
            if (s.str == uniStr)
            {
                //same string on the same position and with same align
                //already exist - do not add it again
                return false;
            }
        }
    }
   
   //this->CalcStringAABB(uniStr, x, y, nullptr);

	AbstractRenderer::AABB estimAABB = this->EstimateStringAABB(uniStr, x, y);

	//test if entire string is outside visible area	

	if (anchor == TextAnchor::CENTER)
	{
		int w = estimAABB.maxX - estimAABB.minX;
		int h = estimAABB.maxY - estimAABB.minY;

		estimAABB.minX -= (w / 2);
		estimAABB.maxX -= (w / 2);
		estimAABB.minY -= (h / 2);
		estimAABB.maxY -= (h / 2);
	}

	if (str != ci.mark)
	{
		if ((estimAABB.maxX <= 0) || 
			(estimAABB.maxY <= 0) ||
			(estimAABB.minX > this->rs.deviceW) || 
			(estimAABB.minY > this->rs.deviceH))
		{			
			return false;
		}		
	}

	//new visible string - add it
		
	this->strChanged = true;
	
	this->fb->AddString(uniStr);

	this->strs.emplace_back(uniStr, x, y, 
		color, anchor, align, type);
		    
    return true;
}



/// <summary>
/// Estimate AABB based on font size - each glyph will have the same size
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <returns></returns>
AbstractRenderer::AABB StringRenderer::EstimateStringAABB(const UnicodeString & str, int x, int y)
{
	AbstractRenderer::AABB aabb;
	
	int maxGlyphHeight = this->fb->GetMaxFontPixelHeight();
    
	int w = maxGlyphHeight;
	int h = maxGlyphHeight;
    int adv = maxGlyphHeight;

	int startX = x;

	int lastNewLineOffset = this->fb->GetMaxNewLineOffset();
	int newLineOffset = 0;
    
    
	FOREACH_32_CHAR_ITERATION(c, str)
	{
		if (c == '\n')
		{
			if (newLineOffset == 0)
			{
				newLineOffset = lastNewLineOffset;
			}

			x = startX;
			y += newLineOffset;

			lastNewLineOffset = newLineOffset;
			newLineOffset = 0;

			continue;
		}

		bool exist;
		FontInfo * fi = nullptr;
		auto it = this->fb->GetGlyph(c, exist, &fi);
		if (exist)
		{					
			GlyphInfo & gi = *it->second;
			w = gi.bmpW;
			h = gi.bmpH;
			adv = static_cast<int>(gi.adv >> 6);
		}
		else
		{
			w = maxGlyphHeight;
			h = maxGlyphHeight;
			adv = maxGlyphHeight;
		}

		newLineOffset = std::max(newLineOffset, fi->newLineOffset);

		int fx = x + w;
		int fy = y - h;
			   		
		if (fx < aabb.minX) aabb.minX = fx;
		if (fy < aabb.minY) aabb.minY = fy;


		if (fx + w > aabb.maxX) aabb.maxX = fx + w;
		if (fy + h > aabb.maxY) aabb.maxY = fy + h;
		

		x += adv;
	}

	return aabb;
}

/// <summary>
/// Calculate AABB of entire text and AABB for every line
/// </summary>
/// <param name="si"></param>
/// <param name="gc"></param>
/// <returns></returns>
StringRenderer::StringAABB StringRenderer::CalcStringAABB(const UnicodeString & str, int x, int y,
	const UsedGlyphCache * gc)
{		
	StringAABB aabb;
	aabb.totalLineOffset = 0;
	aabb.lastLineOffset = 0;
	aabb.maxNewLineOffset = this->fb->GetMaxNewLineOffset() + this->nlOffsetPx;
	aabb.lines.reserve(10); //reserve space for 10 lines

	AbstractRenderer::AABB lineAabb;
		

	int startX = x;
	int index = -1;

	//new line offset has default value 0
	//-> offset is calculated from glyphs
	int newLineOffset = 0;

	FOREACH_32_CHAR_ITERATION(c, str)
	{
		if (c == '\n')
		{			
			if (newLineOffset == 0)
			{		
				//no offset was calculated. Use default maximal new line offset
				newLineOffset = aabb.maxNewLineOffset;
			}

			x = startX;
			y += newLineOffset;
			
			aabb.lastLineOffset = newLineOffset;
			aabb.totalLineOffset += newLineOffset;

			aabb.lines.push_back(lineAabb);
			lineAabb = AbstractRenderer::AABB();
			newLineOffset = 0;

			continue;
		}

		index++;

		FontInfo::UsedGlyphIterator it;
		if (gc != nullptr)
		{
			auto r = (*gc)[index];
			if (!std::get<1>(r))
			{				
				continue;
			}
			it = std::get<0>(r);
			newLineOffset = std::max(newLineOffset, std::get<2>(r)->newLineOffset) + this->nlOffsetPx;
		}
		else
		{
			bool exist;
			FontInfo * fi = nullptr;
			it = this->fb->GetGlyph(c, exist, &fi);
			if (!exist)
			{
				continue;
			}

			newLineOffset = std::max(newLineOffset, fi->newLineOffset) + this->nlOffsetPx;
		}

		GlyphInfo & gi = *it->second;

		int fx = x + gi.bmpX;
		int fy = y - gi.bmpY;


		
		if (fx < lineAabb.minX) lineAabb.minX = fx;
		if (fy < lineAabb.minY) lineAabb.minY = fy;


		if (fx + gi.bmpW > lineAabb.maxX) lineAabb.maxX = fx + gi.bmpW;
		if (fy + gi.bmpH > lineAabb.maxY) lineAabb.maxY = fy + gi.bmpH;

		

		x += (gi.adv >> 6);		
	}

	aabb.lastLineOffset = newLineOffset;
	aabb.totalLineOffset += newLineOffset;

	aabb.lines.push_back(lineAabb);
	

	for (auto & a : aabb.lines)
	{
		if (a.minX < aabb.global.minX) aabb.global.minX = a.minX;
		if (a.minY < aabb.global.minY) aabb.global.minY = a.minY;

		if (a.maxX > aabb.global.maxX) aabb.global.maxX = a.maxX;
		if (a.maxY > aabb.global.maxY) aabb.global.maxY = a.maxY;
	}


	return aabb;
}


/// <summary>
/// Calculate start position of text using anchors
/// </summary>
void StringRenderer::CalcAnchoredPosition()
{	
	int captionMarkAnchorY = 0;

	for (StringInfo & si : this->strs)
	{		
		if (si.aabb.lines.empty() == false)
		{
			//position already computed - linesAABB filled
			continue;
		}

		StringRenderer::UsedGlyphCache gc = this->ExtractGlyphs(si.str);

		si.aabb = this->CalcStringAABB(si.str, si.x, si.y, &gc);

		if (si.anchor == TextAnchor::LEFT_TOP)
		{
			//to do newLines
			si.anchorX = si.x;
			si.anchorY = si.y + si.aabb.maxNewLineOffset; //y position is "line letter start" - move it to letter height
		}
		else if (si.anchor == TextAnchor::CENTER)
		{						
			si.anchorX = si.x - (si.aabb.global.maxX - si.aabb.global.minX) / 2;			
			si.anchorY = si.y - (si.aabb.totalLineOffset - si.aabb.lastLineOffset) / 2; //calc center from all lines and move TOP_LEFT down

		}
		else if (si.anchor == TextAnchor::LEFT_DOWN)
		{
			//to do newLines
			si.anchorX = si.x;
			si.anchorY = si.y - (si.aabb.lines.size() - 1) * si.aabb.maxNewLineOffset; //move down - default Y is at (TOP_LEFT - newLineOffset)
		}


		if (si.type == TextType::CAPTION)
		{	
			if (si.str == ci.mark)
			{				
				bool exist;
				auto it = this->fb->GetGlyph(ci.mark[0], exist);
				if (exist)
				{										
					si.anchorY += (it->second->bmpH);										
					captionMarkAnchorY = si.anchorY + (it->second->bmpH);					
				}			
				else
				{
					captionMarkAnchorY = si.anchorY;
				}
				captionMarkAnchorY += ci.offset;
			}
			else
			{								
				si.anchorY -= (captionMarkAnchorY - si.anchorY);				
			}
			
		}				
	}
}

/// <summary>
/// Calc align of each line from string
/// </summary>
/// <param name="si"></param>
/// <param name="lineId"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void StringRenderer::CalcLineAlign(const StringInfo & si, int lineId, int & x, int & y) const
{	
	if (si.align == TextAlign::ALIGN_CENTER)
	{
		int blockCenterX = (si.aabb.global.maxX - si.aabb.global.minX) / 2;
		//int blockCenterY = (si.aabb.maxY - si.aabb.minY) / 2;

		int lineCenterX = (si.aabb.lines[lineId].maxX - si.aabb.lines[lineId].minX) / 2;
		//int lineCenterY = (si.linesAABB[lineId].maxY - si.linesAABB[lineId].minY) / 2;

		x = x + (blockCenterX - lineCenterX);
	}
}

/// <summary>
/// Extract all glyphs in given string
/// Glyphs are put to vector
/// </summary>
/// <param name="strUTF8"></param>
/// <returns></returns>
StringRenderer::UsedGlyphCache StringRenderer::ExtractGlyphs(const UnicodeString & str)
{
	UsedGlyphCache g;
	g.reserve(str.length());


	FOREACH_32_CHAR_ITERATION(c, str)
	{
		if (c == '\n')
		{			
			continue;
		}

		bool exist;
		FontInfo * fi = nullptr;
		auto it = this->fb->GetGlyph(c, exist, &fi);		
		g.emplace_back(it, exist, fi);
	}

	return g;
}

/// <summary>
/// Calc size of space " ". If it was already
/// calculated, return this calculated result
/// </summary>
/// <returns></returns>
long StringRenderer::CalcSpaceSize()
{
	if (spaceSizeExist)
	{
		return spaceSize;
	}

	//calc space size	
	auto it = this->fb->GetGlyph(' ', spaceSizeExist);
	if (spaceSizeExist)
	{
		GlyphInfo & gi = *it->second;
		spaceSize = (gi.adv >> 6);
	}
	else
	{
		//we want to store only real space if it exist
		bool tmpExist = false;
		auto tmp = this->fb->GetGlyph('a', tmpExist);
		if (tmpExist)
		{
			GlyphInfo & gi = *tmp->second;
			spaceSize = (gi.adv >> 6);
		}
		else
		{
			spaceSize = 10;
		}
	}

	return spaceSize;

}

/// <summary>
/// Generate geometry for all input strings
/// </summary>
/// <returns></returns>
bool StringRenderer::GenerateGeometry()
{
	if (this->strChanged == false)
	{
		return false;
	}

	//first we must build font atlas - it will load glyph infos
	if (this->fb->CreateFontAtlas())
	{
		//if font atlas changed - update texture 

		//DEBUG !!!
		//this->fb->Save("gl.png");
		//-------

		//Fill font texture
		this->FillTexture();
	}
	
	//calculate anchored position
	//it will be calculated only once - if it already is calculated
	//wont be calculated again
	this->CalcAnchoredPosition();

	long spaceSize = this->CalcSpaceSize();
	

	//Build geometry
	
    AbstractRenderer::Clear();
	
	this->geom.reserve(this->strs.size() * 80);

	for (const StringInfo & si : this->strs)
	{

		int lastNewLineOffset = si.aabb.maxNewLineOffset - this->nlOffsetPx;
		int newLineOffset = 0;

		int lineId = 0;
		int x = si.anchorX;
		int y = si.anchorY;

		this->CalcLineAlign(si, lineId, x, y);
		

		FOREACH_32_CHAR_ITERATION(cc, si.str)
		{
            if (cc <= 32)
            {
                if (cc == '\n')
                {
                    if (newLineOffset == 0)
                    {
                        newLineOffset = lastNewLineOffset;
                    }

                    x = si.anchorX;
                    y += newLineOffset + this->nlOffsetPx;
                    lineId++;

                    this->CalcLineAlign(si, lineId, x, y);

                    lastNewLineOffset = newLineOffset;
                    newLineOffset = 0;
                }
                else
                {
                    x += spaceSize;
                }
                continue;
            }

			bool exist;
			FontInfo * fi = nullptr;
			auto it = this->fb->GetGlyph(cc, exist, &fi);
			if (!exist)
			{								
				continue;
			}

			newLineOffset = std::max(newLineOffset, fi->newLineOffset);
			
			
			GlyphInfo & gi = *it->second;
			
            this->AddQuad(gi, x, y, si.color);
            
			x += (gi.adv >> 6);
		}
	}

	this->strChanged = false;

	this->FillVB();

	return true;
}
