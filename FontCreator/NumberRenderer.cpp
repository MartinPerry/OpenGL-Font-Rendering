#include "./NumberRenderer.h"

#include <limits>
#include <algorithm>


#include "./FontBuilder.h"


const utf8_string NumberRenderer::NUMBERS_STRING = u8"0123456789,.-";

NumberRenderer::NumberRenderer(Font fs, RenderSettings r)
	: AbstractRenderer({ fs }, r)
{
	this->checkIfExist = true;

	//prepare numbers
	
	this->fb->AddString(NUMBERS_STRING);
	this->fb->AddString(this->ci.mark);

	if (this->fb->CreateFontAtlas())
	{
		//if font atlas changed - update texture 

		//DEBUG !!!
		//this->fb->Save("gl.png");
		//-------

		//Fill font texture
		this->FillTexture();
	}
	
	for (auto c : NUMBERS_STRING)
	{
		bool exist;
		auto it = this->fb->GetGlyph(c, exist);
		if (!exist)
		{
			throw std::invalid_argument("Unknown number character");;
		}
		

		this->gi[it->first] = *it->second;		
	}
	

	bool exist;
	auto it = this->fb->GetGlyph(this->ci.mark[0], exist);
	if (!exist)
	{		
		it = this->fb->GetGlyph('.', exist);
		if (!exist)
		{
			throw std::invalid_argument("Unknown mark character");;
		}
		this->captionMark = *it->second;
	}
	else
	{
		this->captionMark = *it->second;
	}
		
	this->SetDecimalPrecission(2);

}

NumberRenderer::~NumberRenderer()
{
}

/// <summary>
/// Set whether we want to check if added number already exist
/// </summary>
/// <param name="val"></param>
void NumberRenderer::SetExistenceCheck(bool val)
{
	this->checkIfExist = val;
}

void NumberRenderer::SetDecimalPrecission(int digits)
{
	this->decimalPlaces = digits;
	this->decimalMult = std::pow(10, decimalPlaces);
}

size_t NumberRenderer::GetNumbersCount() const
{
	return this->nmbrs.size();
}

/// <summary>
/// Remove all added strings
/// </summary>
void NumberRenderer::Clear()
{
	this->strChanged = true;
	this->nmbrs.clear();
}

/// <summary>
/// Add new number as caption
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
void NumberRenderer::AddNumberCaption(double val,
	int x, int y, Color color)
{
	//this->AddNumberInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	this->AddNumberInternal(val, x, y, color, TextAnchor::CENTER, TextType::CAPTION);
}

/// <summary>
/// Add new number to be rendered
/// If number already exist - do not add
/// Existing => same x, y, align, value
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
/// <param name="anchor"></param>
void NumberRenderer::AddNumber(double val,
	double x, double y, Color color,
	TextAnchor anchor)
{
	int xx = static_cast<int>(x * this->deviceW);
	int yy = static_cast<int>(y * this->deviceH);

	this->AddNumberInternal(val, xx, yy, color, anchor, TextType::TEXT);
}

/// <summary>
/// Add new number to be rendered
/// If number already exist - do not add
/// Existing => same x, y, align, value
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void NumberRenderer::AddNumber(double val,
	int x, int y, Color color,
	TextAnchor anchor)
{
	this->AddNumberInternal(val, x, y, color, anchor, TextType::TEXT);
}

/// <summary>
/// 
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
/// <param name="anchor"></param>
/// <param name="type"></param>
void NumberRenderer::AddNumberInternal(double val,
	int x, int y, Color color,
	TextAnchor anchor, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::DOWN)
	{
		y = this->deviceH - y;
	}

	if (this->checkIfExist)
	{
		for (auto & s : this->nmbrs)
		{
			if ((s.x == x) && (s.y == y) &&
				(s.anchor == anchor) && (s.type == type))
			{
				if (s.val == val)
				{
					//same number on the same position and with same align
					//already exist - do not add it again
					return;
				}
			}
		}
	}

	AbstractRenderer::AABB aabb = this->CalcNumberAABB(val, x, y);

	//test if entire string is outside visible area
	
	if (anchor == TextAnchor::CENTER)
	{
		int w2 = (aabb.maxX - aabb.minX) / 2;
		int h2 = (aabb.maxY - aabb.minY) / 2;

		aabb.minX -= w2;
		aabb.maxX -= w2;
		aabb.minY -= h2;
		aabb.maxY -= h2;
	}

	if (aabb.maxX <= 0) return;
	if (aabb.maxY <= 0) return;
	if (aabb.minX > this->deviceW) return;
	if (aabb.minY > this->deviceH) return;


	//new visible number - add it

	this->strChanged = true;

	//fill basic structure info
	NumberInfo i;
	i.val = val;
	i.negative = val < 0;


	if (i.negative) val *= -1;
	i.intPart = (unsigned long)(val);
	i.fractPartReverse = this->GetFractPartReversed(val, i.intPart);
	

	i.x = x;
	i.y = y;
	i.color = color;
	i.isDefaultColor = color.IsSame(DEFAULT_COLOR);
	i.anchor = anchor;
	i.type = type;
	i.anchorX = x;
	i.anchorY = y;
	i.aabb = aabb;
	
	this->nmbrs.push_back(i);

	
}

unsigned long NumberRenderer::GetFractPartReversed(double val, unsigned long intPart)
{
	unsigned long fractPartReverse = this->ReversDigits((unsigned long)((val - intPart) * decimalMult));
	if (fractPartReverse == 0)
	{
		return fractPartReverse;
	}

	int fractLeadingZeros = 0;
	float tmp = val - intPart;
	while (tmp < 1)
	{
		fractLeadingZeros++;
		tmp *= 10;
	}
	fractLeadingZeros--;

	
	while (fractLeadingZeros > 0)
	{
		fractPartReverse *= 10;
		fractLeadingZeros--;
	}

	return fractPartReverse;
}

/// <summary>
/// Reverse digits in input number
/// Input: 123
/// Output: 321
/// </summary>
/// <param name="num"></param>
/// <returns></returns>
unsigned long NumberRenderer::ReversDigits(unsigned long num)
{
	unsigned long rev_num = 0;
	while (num > 0)
	{
		rev_num = rev_num * 10 + num % 10;
		num = num / 10;
	}
	return rev_num;
}

/// <summary>
/// Calculate AABB of number
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="globalAABB"></param>
/// <returns></returns>
AbstractRenderer::AABB NumberRenderer::CalcNumberAABB(double val, int x, int y)
{

	AbstractRenderer::AABB aabb;
	aabb.minX = std::numeric_limits<int>::max();
	aabb.minY = std::numeric_limits<int>::max();

	aabb.maxX = std::numeric_limits<int>::min();
	aabb.maxY = std::numeric_limits<int>::min();


	bool negative = val < 0;

	if (negative) val *= -1;
	unsigned long intPart = (unsigned long)(val);
	unsigned long fractPartReverse = this->GetFractPartReversed(val, intPart);
	
	if (negative)
	{
		GlyphInfo & gi = this->gi['-'];

		int fx = x + gi.bmpX;
		int fy = y - gi.bmpY;

		
		if (fx < aabb.minX) aabb.minX = fx;
		if (fy < aabb.minY) aabb.minY = fy;


		if (fx + gi.bmpW > aabb.maxX) aabb.maxX = fx + gi.bmpW;
		if (fy + gi.bmpH > aabb.maxY) aabb.maxY = fy + gi.bmpH;
		
		x += (gi.adv >> 6);
	}


	int lastDigit = 0;

	do
	{
		digits[lastDigit] = (intPart % 10);
		lastDigit++;
		intPart /= 10;
	} while (intPart);

	while (lastDigit > 0)
	{
		lastDigit--;
		const GlyphInfo & gi = this->gi[digits[lastDigit] + '0'];
				
		int fx = x + gi.bmpX;
		int fy = y - gi.bmpY;


		if (fx < aabb.minX) aabb.minX = fx;
		if (fy < aabb.minY) aabb.minY = fy;


		if (fx + gi.bmpW > aabb.maxX) aabb.maxX = fx + gi.bmpW;
		if (fy + gi.bmpH > aabb.maxY) aabb.maxY = fy + gi.bmpH;

		x += (gi.adv >> 6);
	}




	if (fractPartReverse)
	{
		GlyphInfo & gi = this->gi['.'];

		int fx = x + gi.bmpX;
		int fy = y - gi.bmpY;

		if (fx < aabb.minX) aabb.minX = fx;
		if (fy < aabb.minY) aabb.minY = fy;


		if (fx + gi.bmpW > aabb.maxX) aabb.maxX = fx + gi.bmpW;
		if (fy + gi.bmpH > aabb.maxY) aabb.maxY = fy + gi.bmpH;

		x += (gi.adv >> 6);

		
		while (fractPartReverse)
		{			
			int cc = (fractPartReverse % 10);
			fractPartReverse /= 10;
			const GlyphInfo & gi = this->gi[cc + '0'];

			int fx = x + gi.bmpX;
			int fy = y - gi.bmpY;


			if (fx < aabb.minX) aabb.minX = fx;
			if (fy < aabb.minY) aabb.minY = fy;


			if (fx + gi.bmpW > aabb.maxX) aabb.maxX = fx + gi.bmpW;
			if (fy + gi.bmpH > aabb.maxY) aabb.maxY = fy + gi.bmpH;


			x += (gi.adv >> 6);
		}
	}
	

	return aabb;
}

/// <summary>
/// Calculate start position of text using anchors
/// </summary>
void NumberRenderer::CalcAnchoredPosition()
{
	//Calculate anchored position of text
	int newLineOffset = this->fb->GetMaxNewLineOffset();

	for (auto & si : this->nmbrs)
	{		
		if (si.anchor == TextAnchor::LEFT_TOP)
		{
			si.anchorX = si.x;
			si.anchorY = si.y;
			si.anchorY += newLineOffset; //y position is "line letter start" - move it to letter height
		}
		else if (si.anchor == TextAnchor::CENTER)
		{			
			si.anchorX = si.x - (si.aabb.maxX - si.aabb.minX) / 2;

			si.anchorY = si.y;
			si.anchorY += newLineOffset; //move top position to TOP_LEFT
			si.anchorY -= (newLineOffset) / 2; //calc center from all lines and move TOP_LEFT down

		}
		else if (si.anchor == TextAnchor::LEFT_DOWN)
		{
			si.anchorX = si.x;
			si.anchorY = si.y;			
		}

		if (si.type == TextType::CAPTION)
		{						
			int h = (si.aabb.maxY - si.aabb.minY);
			si.anchorY -= (h / 2 + ci.offset);						
		}
		
		si.aabb = this->CalcNumberAABB(si.val, si.anchorX, si.anchorY);

	}
}



/// <summary>
/// Generate geometry for all input numbers
/// </summary>
/// <returns></returns>
bool NumberRenderer::GenerateGeometry()
{
	if (this->strChanged == false)
	{
		return false;
	}

		
	//calculate anchored position
	this->CalcAnchoredPosition();


	//Build geometry
	
	this->geom.clear();

	int newLineOffset = this->fb->GetMaxNewLineOffset();

	for (const NumberRenderer::NumberInfo & si : this->nmbrs)
	{
		int lineId = 0;
		int x = si.anchorX;
		int y = si.anchorY;


		unsigned long intPart = si.intPart;
		unsigned long fractPartReverse = si.fractPartReverse;

		if (si.type == TextType::CAPTION)
		{			
			int xx = si.x - (this->captionMark.bmpW) / 2;

			int yy = si.y + newLineOffset; //move top position to TOP_LEFT
			yy -= (newLineOffset) / 2; //calc center from all lines and move TOP_LEFT down
			yy -= (this->captionMark.bmpH);

			this->AddQuad(this->captionMark, xx, yy, si);
		}



		if (si.negative)
		{
			this->AddQuad(this->gi['-'], x, y, si);
			x += (this->gi['-'].adv >> 6);
		}

		
		int lastDigit = 0;

		do
		{			
			digits[lastDigit] = (intPart % 10);
			lastDigit++;
			intPart /= 10;
		} while (intPart);

		while (lastDigit > 0)
		{
			lastDigit--;
			const GlyphInfo & gi = this->gi[digits[lastDigit] + '0'];

			this->AddQuad(gi, x, y, si);

			x += (gi.adv >> 6);
			
		}

		if (fractPartReverse)
		{
			this->AddQuad(this->gi['.'], x, y, si);
			x += (this->gi['.'].adv >> 6);

			
			while (fractPartReverse)
			{
				int cc = (fractPartReverse % 10);
				fractPartReverse /= 10;
				const GlyphInfo & gi = this->gi[cc + '0'];

				this->AddQuad(gi, x, y, si);

				x += (gi.adv >> 6);
			}
		}		
	}

	this->strChanged = false;

	if (this->geom.size() != 0)
	{
		this->FillVB();
	}

	return true;
}


/// <summary>
/// Add single "letter" quad to geom buffer
/// </summary>
/// <param name="gi"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="ni"></param>
void NumberRenderer::AddQuad(const GlyphInfo & gi, int x, int y, const NumberInfo & ni)
{
	float psW = 1.0f / static_cast<float>(deviceW);	//pixel size in width
	float psH = 1.0f / static_cast<float>(deviceH); //pixel size in height

	float tW = 1.0f / static_cast<float>(this->fb->GetTextureWidth());	//pixel size in width
	float tH = 1.0f / static_cast<float>(this->fb->GetTextureHeight()); //pixel size in height



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
	l.SetColor(ni.color);
	this->geom.push_back(l);
}