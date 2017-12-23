#include "./StringRenderer.h"

#include <limits>

#include "./FontBuilder.h"


StringRenderer::StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion)
	: AbstractRenderer(fs, r, glVersion)
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


void StringRenderer::AddStringCaption(const utf8_string & strUTF8,
	double x, double y, Color color)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	this->AddStringCaption(strUTF8, xx, yy, color);
}

void StringRenderer::AddStringCaption(const utf8_string & strUTF8,
	int x, int y, Color color)
{
	this->AddStringInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	this->AddStringInternal(strUTF8, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
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
void StringRenderer::AddString(const utf8_string & strUTF8,
	double x, double y, Color color,
	TextAnchor anchor, TextAlign align)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	this->AddStringInternal(strUTF8, xx, yy, color, anchor, align, TextType::TEXT);
}

/// <summary>
/// Add new string to be rendered
/// If string already exist - do not add
/// Existing string => same x, y, align, anchor, content
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void StringRenderer::AddString(const utf8_string & strUTF8,
	int x, int y, Color color,
	TextAnchor anchor, TextAlign align)
{
	this->AddStringInternal(strUTF8, x, y, color, anchor, align, TextType::TEXT);
}


void StringRenderer::AddStringInternal(const utf8_string & strUTF8,
	int x, int y, Color color,
	TextAnchor anchor, TextAlign align, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::DOWN)
	{
		y = this->rs.deviceH - y;
	}

	for (auto & s : this->strs)
	{
		if ((s.x == x) && (s.y == y) &&
			(s.align == align) && (s.anchor == anchor) && (s.type == type))
		{
			if (s.strUTF8 == strUTF8)
			{
				//same string on the same position and with same align
				//already exist - do not add it again
				return;
			}
		}
	}

	AbstractRenderer::AABB estimAABB = this->EstimateStringAABB(strUTF8, x, y);

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

	if (estimAABB.maxX <= 0) return;
	if (estimAABB.maxY <= 0) return;
	if (estimAABB.minX > this->rs.deviceW) return;
	if (estimAABB.minY > this->rs.deviceH) return;


	//new visible string - add it

	this->strChanged = true;

	//fill basic structure info
	StringInfo i;
	i.strUTF8 = strUTF8;
	i.x = x;
	i.y = y;
	i.color = color;
	i.isDefaultColor = color.IsSame(DEFAULT_COLOR);
	i.anchor = anchor;
	i.align = align;
	i.type = type;
	i.anchorX = x;
	i.anchorY = y;
	i.linesCount = this->CalcStringLines(strUTF8);
	


	this->strs.push_back(i);

	this->fb->AddString(strUTF8);
}

/// <summary>
/// Calculate number of lines in input string
/// </summary>
/// <param name="strUTF8"></param>
/// <returns></returns>
int StringRenderer::CalcStringLines(const utf8_string & strUTF8) const
{
	int count = 1;
	for (auto c : strUTF8)
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
AbstractRenderer::AABB StringRenderer::EstimateStringAABB(const utf8_string & strUTF8, int x, int y)
{
	AbstractRenderer::AABB aabb;
	aabb.minX = std::numeric_limits<int>::max();
	aabb.minY = std::numeric_limits<int>::max();

	aabb.maxX = std::numeric_limits<int>::min();
	aabb.maxY = std::numeric_limits<int>::min();

	
	int w = this->fb->GetMaxFontPixelSize();
	int h = this->fb->GetMaxFontPixelSize();
	int adv = this->fb->GetMaxFontPixelSize();

	int startX = x;

	int lastNewLineOffset = this->fb->GetMaxNewLineOffset();
	int newLineOffset = 0;// this->fb->GetNewLineOffsetBasedOnFirstGlyph(strUTF8.at(0));

	for (auto c : strUTF8)
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
			w = this->fb->GetMaxFontPixelSize();
			h = this->fb->GetMaxFontPixelSize();
			adv = this->fb->GetMaxFontPixelSize();
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
std::vector<AbstractRenderer::AABB> StringRenderer::CalcStringAABB(const utf8_string & strUTF8,
	int x, int y, AbstractRenderer::AABB & globalAABB, const UsedGlyphCache * gc)
{

	AbstractRenderer::AABB aabb;
	aabb.minX = std::numeric_limits<int>::max();
	aabb.minY = std::numeric_limits<int>::max();

	aabb.maxX = std::numeric_limits<int>::min();
	aabb.maxY = std::numeric_limits<int>::min();

	AbstractRenderer::AABB lineAabb = aabb;
	std::vector<AABB> aabbs;
	
	aabbs.reserve(10); //reserve space for 10 lines

	int lastNewLineOffset = this->fb->GetMaxNewLineOffset();
	int newLineOffset = 0; // this->fb->GetNewLineOffsetBasedOnFirstGlyph(strUTF8.at(0));

	int startX = x;
	int index = -1;

	for (auto c : strUTF8)
	{
		if (c == '\n')
		{
			if (newLineOffset == 0)
			{
				newLineOffset = lastNewLineOffset;
			}

			x = startX;
			y += newLineOffset;

			aabbs.push_back(lineAabb);
			lineAabb = aabb;

			lastNewLineOffset = newLineOffset;
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
			newLineOffset = std::max(newLineOffset, std::get<2>(r)->newLineOffset);
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

			newLineOffset = std::max(newLineOffset, fi->newLineOffset);
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
	int newLineOffset = this->fb->GetMaxNewLineOffset();

	for (auto & si : this->strs)
	{		
		if (si.linesAABB.size() != 0)
		{
			//position already computed - linesAABB filled
			continue;
		}

		StringRenderer::UsedGlyphCache gc = this->ExtractGlyphs(si.strUTF8);

		if (si.anchor == TextAnchor::LEFT_TOP)
		{
			si.anchorX = si.x;
			si.anchorY = si.y;
			si.anchorY += newLineOffset; //y position is "line letter start" - move it to letter height
		}
		else if (si.anchor == TextAnchor::CENTER)
		{
			if (si.linesAABB.size() == 0)
			{
				si.linesAABB = this->CalcStringAABB(si.strUTF8, si.x, si.y, si.aabb, &gc);
			}

			si.anchorX = si.x - (si.aabb.maxX - si.aabb.minX) / 2;

			si.anchorY = si.y;
			si.anchorY += newLineOffset; //move top position to TOP_LEFT
			si.anchorY -= (si.linesCount * newLineOffset) / 2; //calc center from all lines and move TOP_LEFT down

		}
		else if (si.anchor == TextAnchor::LEFT_DOWN)
		{
			si.anchorX = si.x;
			si.anchorY = si.y;
			si.anchorY -= (si.linesCount - 1) * newLineOffset; //move down - default Y is at (TOP_LEFT - newLineOffset)
		}

		if (si.type == TextType::CAPTION)
		{
			if (si.strUTF8 == ci.mark)
			{
				bool exist;
				auto it = this->fb->GetGlyph(ci.mark[0], exist);
				if (exist)
				{
					//si.anchorX -= (it->second->bmpW);
					//si.anchorY -= (it->second->bmpH);
					//si.anchorY -= this->ci.offset;

					int yy = si.y + this->ci.offset; //move top position to TOP_LEFT
					yy -= (this->ci.offset) / 2; //calc center from all lines and move TOP_LEFT down
					yy -= (it->second->bmpH);
					si.anchorY = yy;
				}				

			}
			else
			{
				si.linesAABB = this->CalcStringAABB(si.strUTF8, si.anchorX, si.anchorY, si.aabb, &gc);

				int h = (si.aabb.maxY - si.aabb.minY);
				si.anchorY -= (h / 2 + ci.offset);
			}
		}

		si.linesAABB = this->CalcStringAABB(si.strUTF8, si.anchorX, si.anchorY, si.aabb, &gc);

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
StringRenderer::UsedGlyphCache StringRenderer::ExtractGlyphs(const utf8_string & strUTF8)
{
	UsedGlyphCache g;
	g.reserve(strUTF8.length());


	for (auto c : strUTF8)
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
	
	float psW = 1.0f / static_cast<float>(rs.deviceW);	//pixel size in width
	float psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

	float tW = 1.0f / static_cast<float>(this->fb->GetTextureWidth());	//pixel size in width
	float tH = 1.0f / static_cast<float>(this->fb->GetTextureHeight()); //pixel size in height

	this->geom.clear();

	int lastNewLineOffset = this->fb->GetMaxNewLineOffset();
	int newLineOffset = 0;// this->fb->GetMaxNewLineOffset();

	this->geom.reserve(this->strs.size() * 10);

	for (const StringRenderer::StringInfo & si : this->strs)
	{
		int lineId = 0;
		int x = si.anchorX;
		int y = si.anchorY;

		int startX = x;
		int startY = y;

		this->CalcLineAlign(si, lineId, x, y);
		

		for (auto cc : si.strUTF8)
		{
			if (cc == '\n')
			{
				if (newLineOffset == 0)
				{
					newLineOffset = lastNewLineOffset;
				}

				x = startX;
				y += newLineOffset;
				lineId++;

				this->CalcLineAlign(si, lineId, x, y);

				lastNewLineOffset = newLineOffset;
				newLineOffset = 0;

				continue;
			}

			if (cc <= 32)
			{
				x += spaceSize;
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
			
			int fx = x + gi.bmpX;
			int fy = y - gi.bmpY;

			//build geometry		
			Vertex a, b, c, d;
			a.x = static_cast<float>(fx);
			a.y = static_cast<float>(fy);
			a.u = static_cast<float>(gi.tx);
			a.v = static_cast<float>(gi.ty);

			b.x = static_cast<float>(fx + gi.bmpW);
			b.y = static_cast<float>(fy);
			b.u = static_cast<float>(gi.tx + gi.bmpW);
			b.v = static_cast<float>(gi.ty);

			c.x = static_cast<float>(fx + gi.bmpW);
			c.y = static_cast<float>(fy + gi.bmpH);
			c.u = static_cast<float>(gi.tx + gi.bmpW);
			c.v = static_cast<float>(gi.ty + gi.bmpH);

			d.x = static_cast<float>(fx);
			d.y = static_cast<float>(fy + gi.bmpH);
			d.u = static_cast<float>(gi.tx);
			d.v = static_cast<float>(gi.ty + gi.bmpH);

			a.Mul(psW, psH, tW, tH);
			b.Mul(psW, psH, tW, tH);
			c.Mul(psW, psH, tW, tH);
			d.Mul(psW, psH, tW, tH);

			LetterGeom l;
			l.AddQuad(a, b, c, d);
			l.SetColor(si.color);
			this->geom.push_back(l);

			x += (gi.adv >> 6);
		}
	}

	this->strChanged = false;

	if (this->geom.size() != 0)
	{
		this->FillVB();
	}

	return true;
}
