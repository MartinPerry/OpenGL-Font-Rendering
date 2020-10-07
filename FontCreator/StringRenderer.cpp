#include "./StringRenderer.h"

#include <limits>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"

#include "./Externalncludes.h"

/// <summary>
/// Create optimized String renderer that will use only one color for every string
/// If we pass color parameter during AddString... method call,
/// this parameter is ignored and string is rendered with color specified here
/// </summary>
/// <param name="color">color used for all string</param>
/// <param name="fs"></param>
/// <param name="r"></param>
/// <param name="glVersion"></param>
/// <returns></returns>
StringRenderer * StringRenderer::CreateSingleColor(Color color, const std::vector<Font> & fs, RenderSettings r, int glVersion)
{
	auto sm = std::make_shared<SingleColorFontShaderManager>();
	sm->SetColor(color.r, color.g, color.b, color.a);

	return new StringRenderer(fs, r, glVersion, 
		SINGLE_COLOR_VERTEX_SHADER_SOURCE, SINGLE_COLOR_PIXEL_SHADER_SOURCE,
		sm);

}

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
#ifdef THREAD_SAFETY
	std::lock_guard<std::shared_timed_mutex> lk(m);
#endif

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

bool StringRenderer::AddStringCaption(const char * str,
	double x, double y, const RenderParams & rp)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	return this->AddStringCaption(UTF8_TEXT(str), xx, yy, rp);
}

bool StringRenderer::AddStringCaption(const UnicodeString & str,
	double x, double y, const RenderParams & rp)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	return this->AddStringCaption(str, xx, yy, rp);
}

bool StringRenderer::AddStringCaption(const char * str,
	int x, int y, const RenderParams & rp)
{
	this->AddStringInternal(ci.mark, x, y, rp, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	return this->AddStringInternal(UTF8_TEXT(str), x, y, rp, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
}

bool StringRenderer::AddStringCaption(const UnicodeString & str,
	int x, int y, const RenderParams & rp)
{
    this->AddStringInternal(ci.mark, x, y, rp, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
    return this->AddStringInternal(str, x, y, rp, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);	
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
bool StringRenderer::AddString(const char * str,
	double x, double y, const RenderParams & rp,
	TextAnchor anchor, TextAlign align)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	return this->AddStringInternal(UTF8_TEXT(str), xx, yy, rp, anchor, align, TextType::TEXT);
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
bool StringRenderer::AddString(const UnicodeString & str,
	double x, double y, const RenderParams & rp,
	TextAnchor anchor, TextAlign align)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	return this->AddStringInternal(str, xx, yy, rp, anchor, align, TextType::TEXT);
}

/// <summary>
/// Add new string to be rendered
/// If string already exist - do not add
/// Existing string => same x, y, align, anchor, content
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
bool StringRenderer::AddString(const char * str,
	int x, int y, const RenderParams & rp,
	TextAnchor anchor, TextAlign align)
{
	return this->AddStringInternal(UTF8_TEXT(str), x, y, rp, anchor, align, TextType::TEXT);
}

/// <summary>
/// Add new string to be rendered
/// If string already exist - do not add
/// Existing string => same x, y, align, anchor, content
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
bool StringRenderer::AddString(const UnicodeString & str,
	int x, int y, const RenderParams & rp,
	TextAnchor anchor, TextAlign align)
{
	return this->AddStringInternal(str, x, y, rp, anchor, align, TextType::TEXT);
}

bool StringRenderer::AddStringInternal(const UnicodeString & str,
	int x, int y, const RenderParams & rp,
	TextAnchor anchor, TextAlign align, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::AxisYOrigin::DOWN)
	{
		y = this->rs.deviceH - y;
	}

	UnicodeString uniStr = (this->isBidiEnabled) ? BIDI(str) : str;
	
	if (this->CanAddString(uniStr, x, y, rp, anchor, align, type) == false)
	{
		return false;
	}
  
	//new visible string - add it
		
#ifdef THREAD_SAFETY
	std::lock_guard<std::shared_timed_mutex> lk(m);
#endif
		
	this->fb->AddString(uniStr);

	this->strs.emplace_back(uniStr, x, y, anchor, align, type);	
	auto & lines = this->strs.back().lines;

	lines.emplace_back(0, rp);

	int len = 0;
	int start = 0;
	FOREACH_32_CHAR_ITERATION(c, uniStr)
	{			
		if (c == '\n')
		{
			lines.back().len = len;
			lines.emplace_back(start + 1, rp);
			len = 0;
		}
		else
		{
			len++;
		}
		start++;
	}

	lines.back().len = len;

	this->strChanged = true;

    return true;
}

/// <summary>
/// test if string uniStr can be added to renderer
/// </summary>
/// <param name="uniStr"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
/// <param name="anchor"></param>
/// <param name="align"></param>
/// <param name="type"></param>
/// <returns></returns>
bool StringRenderer::CanAddString(const UnicodeString & uniStr,
	int x, int y, const RenderParams & rp,
	TextAnchor anchor, TextAlign align, TextType type) const
{
	for (const StringInfo & s : this->strs)
	{
		if ((s.x == x) && (s.y == y) &&
			//(s.renderParams.scale == rp.scale) &&
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

	AbstractRenderer::AABB estimAABB = this->EstimateStringAABB(uniStr,
		static_cast<float>(x), static_cast<float>(y), rp.scale);

	//test if entire string is outside visible area	
	if (anchor == TextAnchor::CENTER)
	{
		float w = estimAABB.maxX - estimAABB.minX;
		float h = estimAABB.maxY - estimAABB.minY;

		estimAABB.minX -= (w / 2);
		estimAABB.maxX -= (w / 2);
		estimAABB.minY -= (h / 2);
		estimAABB.maxY -= (h / 2);
	}

	if (uniStr != ci.mark)
	{
		if ((estimAABB.maxX <= 0) ||
			(estimAABB.maxY <= 0) ||
			(estimAABB.minX > this->rs.deviceW) ||
			(estimAABB.minY > this->rs.deviceH))
		{
			return false;
		}
	}

	return true;
}

/// <summary>
/// Estimate AABB based on font size - each glyph will have the same size
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <returns></returns>
AbstractRenderer::AABB StringRenderer::EstimateStringAABB(const UnicodeString & str, 
	float x, float y, float scale) const
{
	AbstractRenderer::AABB aabb;
	
	float maxGlyphHeight = this->fb->GetMaxFontPixelHeight() * scale;
    
	float w = maxGlyphHeight;
	float h = maxGlyphHeight;
	float adv = maxGlyphHeight;

	float startX = x;

	float lastNewLineOffset = this->fb->GetMaxNewLineOffset() * scale;
	float newLineOffset = 0;
    

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
			const GlyphInfo & gi = *it->second;
			w = gi.bmpW * scale;
			h = gi.bmpH * scale;
			adv = (gi.adv >> 6) * scale;
		}
		else
		{
			w = maxGlyphHeight;
			h = maxGlyphHeight;
			adv = maxGlyphHeight;
		}

		newLineOffset = std::max(newLineOffset, fi->newLineOffset * scale);

		float fx = x + w;
		float fy = y - h;
		aabb.Update(fx, fy, w, h);


		x += adv;
	}

	return aabb;
}

/// <summary>
/// Calculate AABB of entire text and AABB for every line
/// it is not positioned
/// </summary>
/// <param name="str"></param>
/// <param name="s"></param>
/// <param name="gc"></param>
/// <returns></returns>
void StringRenderer::CalcStringAABB(StringInfo & si, const UsedGlyphCache * gc) const
{			
	float maxNewLineOffset = (this->fb->GetMaxNewLineOffset() + this->nlOffsetPx);
				
	float x = 0;
	float y = 0;

	float startX = x;
	int index = -1;

	//new line offset has default value 0
	//-> offset is calculated from glyphs
	float newLineOffset = 0;
		
	for (LineInfo & li : si.lines)
	{
		FOREACH_32_CHAR_ITERATION_FROM(c, si.str, li.start, li.len)
		{			
			index++;

			FontInfo::GlyphLutIterator it;
			if (gc != nullptr)
			{
				auto r = (*gc)[index];
				if (!std::get<1>(r))
				{
					continue;
				}
				it = std::get<0>(r);
				newLineOffset = std::max(newLineOffset, static_cast<float>(std::get<2>(r)->newLineOffset + this->nlOffsetPx));
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

				newLineOffset = std::max(newLineOffset, static_cast<float>(fi->newLineOffset + this->nlOffsetPx));
			}

			const GlyphInfo & gi = *it->second;

			float fx = x + gi.bmpX;
			float fy = y - gi.bmpY;
			li.aabb.Update(fx, fy, gi.bmpW, gi.bmpH);

			x += (gi.adv >> 6);
		}

		if (newLineOffset == 0)
		{
			//no offset was calculated. Use default maximal new line offset
			newLineOffset = maxNewLineOffset;
		}

		newLineOffset *= li.renderParams.scale;

		li.maxNewLineOffset = newLineOffset;

		x = startX;
		y += newLineOffset;

		newLineOffset = 0;
	}

	/*
	int lineId = 0;

	FOREACH_32_CHAR_ITERATION(c, si.str)
	{
		if (c == '\n')
		{			
			if (newLineOffset == 0)
			{		
				//no offset was calculated. Use default maximal new line offset
				newLineOffset = maxNewLineOffset;
			}

			newLineOffset *= si.lines[lineId].renderParams.scale;

			si.lines[lineId].maxNewLineOffset = newLineOffset;

			x = startX;
			y += newLineOffset;// *str.lines[lineId].renderParams.scale;
		
			newLineOffset = 0;
			lineId++;

			continue;
		}

		index++;

		FontInfo::GlyphLutIterator it;
		if (gc != nullptr)
		{
			auto r = (*gc)[index];
			if (!std::get<1>(r))
			{				
				continue;
			}
			it = std::get<0>(r);
			newLineOffset = std::max(newLineOffset, static_cast<float>(std::get<2>(r)->newLineOffset + this->nlOffsetPx));
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

			newLineOffset = std::max(newLineOffset, static_cast<float>(fi->newLineOffset + this->nlOffsetPx));
		}
				
		const GlyphInfo & gi = *it->second;

		float fx = x + gi.bmpX;
		float fy = y - gi.bmpY;			   		
		si.lines[lineId].aabb.Update(fx, fy, gi.bmpW, gi.bmpH);

		x += (gi.adv >> 6);
	}
	*/
	//aabb.lines.push_back(lineAabb);
	

	for (auto & a : si.lines)
	{		
		a.aabb.maxX *= a.renderParams.scale;
		a.aabb.maxY *= a.renderParams.scale;
		a.aabb.minX *= a.renderParams.scale;
		a.aabb.minY *= a.renderParams.scale;

		si.global.UnionWithOffset(a.aabb, 0);
	}
}


/// <summary>
/// Calculate start position of text using anchors
/// </summary>
void StringRenderer::CalcAnchoredPosition()
{	
	float captionMarkAnchorY = 0;

	for (StringInfo & si : this->strs)
	{		
		if (si.lines.front().aabb.IsEmpty() == false)		
		{
			//position already computed - linesAABB filled
			continue;
		}

		StringRenderer::UsedGlyphCache gc = this->ExtractGlyphs(si.str);

		this->CalcStringAABB(si, &gc);

		if (si.anchor == TextAnchor::LEFT_TOP)
		{			
			si.anchorX = static_cast<float>(si.x);
			si.anchorY = si.y - std::min(0.0f, si.global.minY);
		}
		else if (si.anchor == TextAnchor::CENTER)
		{						
			si.anchorX = static_cast<float>(si.x - static_cast<int>(si.global.maxX - si.global.minX) / 2);			
			si.anchorY = static_cast<float>(si.y - std::min(0.0f, si.global.minY) - static_cast<int>(si.global.maxY - si.global.minY) / 2);
		}
		else if (si.anchor == TextAnchor::LEFT_DOWN)
		{		
			si.anchorX = static_cast<float>(si.x);
			si.anchorY = si.y - (si.global.maxY - si.global.minY);
		}


		if (si.type == TextType::CAPTION)
		{	
			if (si.str == ci.mark)
			{				
				bool exist;
				auto it = this->fb->GetGlyph(ci.mark[0], exist);
				if (exist)
				{										
					//si.anchorY += (it->second->bmpH);										
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
void StringRenderer::CalcLineAlign(const StringInfo & si, const LineInfo & li, float & x, float & y) const
{	
	if (si.align == TextAlign::ALIGN_CENTER)
	{
		float blockCenterX = (si.global.maxX - si.global.minX) / 2;		
		float lineCenterX = (li.aabb.maxX - li.aabb.minX) / 2;
		
		x += (blockCenterX - lineCenterX);
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
		const GlyphInfo & gi = *it->second;
		spaceSize = (gi.adv >> 6);
	}
	else
	{
		//we want to store only real space if it exist
		bool tmpExist = false;
		auto tmp = this->fb->GetGlyph('a', tmpExist);
		if (tmpExist)
		{
			const GlyphInfo & gi = *tmp->second;
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
		float y = si.anchorY;
		
		for (const LineInfo & li : si.lines)
		{
			float x = si.anchorX;

			this->CalcLineAlign(si, li, x, y);

			FOREACH_32_CHAR_ITERATION_FROM(c, si.str, li.start, li.len)
			{				
				if (c <= 32)
				{
					x += spaceSize;					
					continue;
				}

				bool exist;
				FontInfo * fi = nullptr;
				auto it = this->fb->GetGlyph(c, exist, &fi);
				if (!exist)
				{
					continue;
				}

				const GlyphInfo & gi = *it->second;

				/*
				//test for single scaled chars
				if (cc == 'A')
				{
					auto pp = si.renderParams;
					pp.scale = 5.0;
					this->AddQuad(gi, x, y, pp);
				}
				else
				{
					this->AddQuad(gi, x, y, si.renderParams);
				}
				*/

				this->AddQuad(gi, x, y, li.renderParams);

				x += (gi.adv >> 6) * li.renderParams.scale;
			}

			//x = si.anchorX;
			//y += newLineOffset + this->nlOffsetPx * li->renderParams.scale;
			y += li.maxNewLineOffset;
						
			//this->CalcLineAlign(si, li, x, y);
		}


		/*
		int lineId = 0;
		const LineInfo * li = &(si.lines[lineId]);

		//float lastNewLineOffset = li->maxNewLineOffset - this->nlOffsetPx;
		//float newLineOffset = 0;

		FOREACH_32_CHAR_ITERATION(cc, si.str)
		{
            if (cc <= 32)
            {
                if (cc == '\n')
                {
                   // if (newLineOffset == 0)
                    //{
                    //    newLineOffset = lastNewLineOffset;
                    //}
					
                    x = si.anchorX;
                    //y += newLineOffset + this->nlOffsetPx * li->renderParams.scale;
					y += si.lines[lineId].maxNewLineOffset;
                    lineId++;

					li = &(si.lines[lineId]);

                    this->CalcLineAlign(si, lineId, x, y);

                    //lastNewLineOffset = newLineOffset;
                    //newLineOffset = 0;
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

			//newLineOffset = std::max(newLineOffset, fi->newLineOffset * li->renderParams.scale);
			
			
			const GlyphInfo & gi = *it->second;
						
			this->AddQuad(gi, x, y, li->renderParams);
            
			x += (gi.adv >> 6) * li->renderParams.scale;
		}
		*/
	}

	this->strChanged = false;

	this->FillVB();

	return true;
}
