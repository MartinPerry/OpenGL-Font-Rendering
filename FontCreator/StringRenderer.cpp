#include "./StringRenderer.h"

#include <limits>

#include "./FontBuilder.h"

#include "./Externalncludes.h"

StringRenderer::StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion)
	: AbstractRenderer(fs, r, glVersion), isBidiEnabled(true)
{
	this->SetNewLineOffset(0);
}

StringRenderer::StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion,
               const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm)
    : AbstractRenderer(fs, r, glVersion, vSource, pSource, sm), isBidiEnabled(true)
{
    this->SetNewLineOffset(0);
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

	UnicodeString bidiStr = "";
	bool isBidiEmpty = true;

	if (this->isBidiEnabled == false)
	{
		bidiStr = str;
		isBidiEmpty = false;
	}

   for (auto & s : this->strs)
    {
        if ((s.x == x) && (s.y == y) &&
            (s.align == align) && (s.anchor == anchor) && (s.type == type))
        {
            
            if (isBidiEmpty)
            {
                //we need to compare string... create bidi only for first comparison
                bidiStr = BIDI(str);
                isBidiEmpty = false;
            }

            if (s.str == bidiStr)
            {
                //same string on the same position and with same align
                //already exist - do not add it again
                return false;
            }
        }
    }

	//fill basic structure info
	StringInfo i;

	if (isBidiEmpty)
	{		
		//no bidi yet - create it
		i.str = BIDI(str);
	}
	else
	{
		//bidi was already constructed
		i.str = bidiStr;
	}

	AbstractRenderer::AABB estimAABB = this->EstimateStringAABB(i.str, x, y);

	//test if entire string is outside visible area

	int w = estimAABB.maxX - estimAABB.minX;
	int h = estimAABB.maxY - estimAABB.minY;

	if (anchor == TextAnchor::CENTER)
	{
		estimAABB.minX -= (w / 2);
		estimAABB.maxX -= (w / 2);
		estimAABB.minY -= (h / 2);
		estimAABB.maxY -= (h / 2);
	}

	if (str != ci.mark)
	{
		if ((estimAABB.maxX <= 0) || (estimAABB.maxY <= 0) ||
			(estimAABB.minX > this->rs.deviceW) || (estimAABB.minY > this->rs.deviceH))
		{			
			return false;
		}		
	}

	//new visible string - add it

	this->strChanged = true;
	
	i.x = x;
	i.y = y;
	i.color = color;
	//i.isDefaultColor = color.IsSame(DEFAULT_COLOR);
	i.anchor = anchor;
	i.align = align;
	i.type = type;
	i.anchorX = x;
	i.anchorY = y;
	i.linesCount = this->CalcStringLines(i.str);
	


	this->strs.push_back(i);

	this->fb->AddString(i.str);
    
    return true;
}

/// <summary>
/// Calculate number of lines in input string
/// </summary>
/// <param name="strUTF8"></param>
/// <returns></returns>
int StringRenderer::CalcStringLines(const UnicodeString & str) const
{
	int count = 1;
	FOREACH_32_CHAR_ITERATION(c, str)
	{
		if (c == '\n')
		{
			count++;
		}
	}
	return count;
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
	aabb.minX = std::numeric_limits<int>::max();
	aabb.minY = std::numeric_limits<int>::max();

	aabb.maxX = std::numeric_limits<int>::min();
	aabb.maxY = std::numeric_limits<int>::min();

    int maxFontSize = this->fb->GetMaxFontPixelSize();
    
	int w = maxFontSize;
	int h = maxFontSize;
    int adv = maxFontSize;;

	int startX = x;

	int lastNewLineOffset = this->fb->GetMaxNewLineOffset();
	int newLineOffset = 0;// this->fb->GetNewLineOffsetBasedOnFirstGlyph(strUTF8.at(0));
    
    
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
			w = maxFontSize;
			h = maxFontSize;
			adv = maxFontSize;
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
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="globalAABB"></param>
/// <param name="gc"></param>
/// <returns></returns>
std::vector<AbstractRenderer::AABB> StringRenderer::CalcStringAABB(const UnicodeString & str,
	int x, int y, AbstractRenderer::AABB & globalAABB, const UsedGlyphCache * gc)
{

	AbstractRenderer::AABB aabb;
	aabb.minX = std::numeric_limits<int>::max();
	aabb.minY = std::numeric_limits<int>::max();

	aabb.maxX = std::numeric_limits<int>::min();
	aabb.maxY = std::numeric_limits<int>::min();

	aabb.newLineOffset = 0;

	int maxNewLineOffset = this->fb->GetMaxNewLineOffset() + this->nlOffsetPx;

	AbstractRenderer::AABB lineAabb = aabb;
	std::vector<AABB> aabbs;
	
	aabbs.reserve(10); //reserve space for 10 lines
	

	int startX = x;
	int index = -1;

	FOREACH_32_CHAR_ITERATION(c, str)
	{
		if (c == '\n')
		{			
			if (lineAabb.newLineOffset == 0)
			{
				lineAabb.newLineOffset = maxNewLineOffset;
			}

			x = startX;
			y += lineAabb.newLineOffset;
						
			aabbs.push_back(lineAabb);
			lineAabb = aabb;
			
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
			lineAabb.newLineOffset = std::max(lineAabb.newLineOffset, std::get<2>(r)->newLineOffset) + this->nlOffsetPx;
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

			lineAabb.newLineOffset = std::max(lineAabb.newLineOffset, fi->newLineOffset) + this->nlOffsetPx;
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


	aabbs.push_back(lineAabb);
	globalAABB = aabb;

	for (auto & a : aabbs)
	{
		if (a.minX < globalAABB.minX) globalAABB.minX = a.minX;
		if (a.minY < globalAABB.minY) globalAABB.minY = a.minY;

		if (a.maxX > globalAABB.maxX) globalAABB.maxX = a.maxX;
		if (a.maxY > globalAABB.maxY) globalAABB.maxY = a.maxY;
	}


	return aabbs;
}


/// <summary>
/// Calculate start position of text using anchors
/// </summary>
void StringRenderer::CalcAnchoredPosition()
{
	//Calculate anchored position of text
	int newLineOffset = this->fb->GetMaxNewLineOffset() + this->nlOffsetPx;

	int captionMarkAnchorY = 0;

	for (auto & si : this->strs)
	{		
		if (si.linesAABB.size() != 0)
		{
			//position already computed - linesAABB filled
			continue;
		}

		StringRenderer::UsedGlyphCache gc = this->ExtractGlyphs(si.str);

		if (si.anchor == TextAnchor::LEFT_TOP)
		{
			//to do newLines
			si.anchorX = si.x;
			si.anchorY = si.y;
			si.anchorY += newLineOffset; //y position is "line letter start" - move it to letter height
		}
		else if (si.anchor == TextAnchor::CENTER)
		{
			if (si.linesAABB.size() == 0)
			{
				si.linesAABB = this->CalcStringAABB(si.str, si.x, si.y, si.aabb, &gc);
			}

			int totalLineOffset = 0;
			int lastLineOffset = 0;
			for (auto & aabb : si.linesAABB)
			{
				lastLineOffset = aabb.newLineOffset;
				totalLineOffset += lastLineOffset;
			}

			si.anchorX = si.x - (si.aabb.maxX - si.aabb.minX) / 2;

			si.anchorY = si.y;
			si.anchorY -= (totalLineOffset - lastLineOffset) / 2; //calc center from all lines and move TOP_LEFT down

		}
		else if (si.anchor == TextAnchor::LEFT_DOWN)
		{
			//to do newLines
			si.anchorX = si.x;
			si.anchorY = si.y;
			si.anchorY -= (si.linesCount - 1) * newLineOffset; //move down - default Y is at (TOP_LEFT - newLineOffset)
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
					//si.anchorY += ci.offset;
					
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
		
		si.linesAABB = this->CalcStringAABB(si.str, si.anchorX, si.anchorY, si.aabb, &gc);

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
		int blockCenterX = (si.aabb.maxX - si.aabb.minX) / 2;
		//int blockCenterY = (si.aabb.maxY - si.aabb.minY) / 2;

		int lineCenterX = (si.linesAABB[lineId].maxX - si.linesAABB[lineId].minX) / 2;
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
		g.push_back({ it, exist, fi });
	}

	return g;
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


	//calc space size
	long spaceSize = 0;
	bool exist;
	auto it = this->fb->GetGlyph(' ', exist);
	if (exist)
	{
		GlyphInfo & gi = *it->second;
		spaceSize = (gi.adv >> 6);
	}
	else 
	{
		auto tmp = this->fb->GetGlyph('a', exist);
		if (exist)
		{
			GlyphInfo & gi = *tmp->second;
			spaceSize = (gi.adv >> 6);
		}
		else 
		{
			spaceSize = 10;
		}
	}


	//Build geometry
	
    AbstractRenderer::Clear();

	int lastNewLineOffset = this->fb->GetMaxNewLineOffset();
	int newLineOffset = 0;// this->fb->GetMaxNewLineOffset();

	this->geom.reserve(this->strs.size() * 80);

	for (const StringRenderer::StringInfo & si : this->strs)
	{
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
