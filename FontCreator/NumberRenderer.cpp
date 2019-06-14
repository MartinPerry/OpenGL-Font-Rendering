#include "./NumberRenderer.h"

#include <limits>
#include <algorithm>


#include "./FontBuilder.h"


const std::string NumberRenderer::NUMBERS_STRING = "0123456789,.-";

/// <summary>
/// ctor
/// </summary>
/// <param name="fs"></param>
/// <param name="r"></param>
/// <param name="glVersion"></param>
NumberRenderer::NumberRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion)
	: AbstractRenderer(fs, r, glVersion),
	decimalPlaces(0), 
	decimalMult(1)
{
	this->checkIfExist = true;

	//prepare numbers
	
	for (auto c : NUMBERS_STRING)
	{
		this->fb->AddString(c);
	}
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
			throw std::invalid_argument("Unknown number character");			
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
			throw std::invalid_argument("Unknown mark character");			
		}
		else
		{
			this->SetCaption(UTF8_TEXT(u8"."), 10);
			this->captionMark = *it->second;			
		}
	}
	else
	{
		this->captionMark = *it->second;
	}
		
	this->SetDecimalPrecission(2);

	this->newLineOffset = this->fb->GetMaxNewLineOffset();

	this->Precompute();
}

/// <summary>
/// dtor
/// </summary>
NumberRenderer::~NumberRenderer()
{
}

/// <summary>
/// Precompute array of double-digits glyphs
/// </summary>
void NumberRenderer::Precompute()
{
	for (int i = 0; i < 100; i++)
	{
		int lastDigit = 0;
		int intPart = i;

		do
		{
			digits[lastDigit] = (intPart % 10);
			lastDigit++;
			intPart /= 10;
		} while (intPart);

		this->precompGi[i][0] = &this->gi['0'];
		this->precompGi[i][1] = this->precompGi[i][0];

		while (lastDigit > 0)
		{
			lastDigit--;			
			this->precompGi[i][lastDigit] = &this->gi[digits[lastDigit] + '0'];			
		}
	}
}

/// <summary>
/// Set whether we want to check if added number already exist
/// </summary>
/// <param name="val"></param>
void NumberRenderer::SetExistenceCheck(bool val)
{
	this->checkIfExist = val;
}

/// <summary>
/// Set precision for decimal part in digits count
/// </summary>
/// <param name="digits"></param>
void NumberRenderer::SetDecimalPrecission(int digits)
{
    if (this->decimalPlaces == digits)
    {
        return;
    }
	this->decimalPlaces = digits;
	this->decimalMult = std::pow(10, decimalPlaces);
}


/// <summary>
/// Get precision for decimal part in digits count
/// </summary>
/// <returns></returns>
int NumberRenderer::GetDecimalPrecission() const
{
	return this->decimalPlaces;
}

/// <summary>
/// Get number of numbers
/// </summary>
/// <returns></returns>
size_t NumberRenderer::GetNumbersCount() const
{
	return this->nmbrs.size();
}

/// <summary>
/// Remove all added strings
/// </summary>
void NumberRenderer::Clear()
{
#ifdef THREAD_SAFETY
	std::lock_guard<std::shared_timed_mutex> lk(m);
#endif

	AbstractRenderer::Clear();
	this->nmbrs.clear();
}


/// <summary>
/// Add integer number - internal method
/// This is called from template method based on type
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
/// <param name="anchor"></param>
/// <param name="type"></param>
void NumberRenderer::AddIntegralNumberInternal(long val,
	int x, int y, Color color,
	TextAnchor anchor, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::DOWN)
	{
		y = this->rs.deviceH - y;
	}

	if (this->checkIfExist)
	{
		for (NumberInfo & s : this->nmbrs)
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

	NumberInfo i;
	i.val = val;
	i.negative = val < 0;

	if (i.negative) val *= -1;
	i.intPart = static_cast<unsigned long>(val);
	i.fractPartReverse = 0;


	this->AddNumber(i, x, y, color, anchor, type);	
}

/// <summary>
/// Add float number - internal method
/// This is called from template method based on type
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
/// <param name="anchor"></param>
/// <param name="type"></param>
void NumberRenderer::AddFloatNumberInternal(double val,
	int x, int y, Color color,
	TextAnchor anchor, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::DOWN)
	{
		y = this->rs.deviceH - y;
	}

	if (this->checkIfExist)
	{
		for (NumberInfo & s : this->nmbrs)
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

	NumberInfo i;
	i.val = val;
	i.negative = val < 0;

	if (i.negative) val *= -1;
	i.intPart = static_cast<unsigned long>(val);
	i.fractPartReverse = this->GetFractPartReversed(val, i.intPart);


	this->AddNumber(i, x, y, color, anchor, type);
}

/// <summary>
/// Add new number 
/// </summary>
/// <param name="n"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
/// <param name="anchor"></param>
/// <param name="type"></param>
void NumberRenderer::AddNumber(NumberInfo & n, int x, int y, Color color,
	TextAnchor anchor, TextType type)
{
	AbstractRenderer::AABB aabb = this->CalcNumberAABB(n.val, x, y, n.negative, n.intPart, n.fractPartReverse);

	//test if entire string is outside visible area

	int w = (aabb.maxX - aabb.minX);
	int h = (aabb.maxY - aabb.minY);

	if (anchor == TextAnchor::CENTER)
	{		
		aabb.minX -= (w / 2);
		aabb.maxX -= (w / 2);
		aabb.minY -= (h / 2);
		aabb.maxY -= (h / 2);
	}

	if (aabb.maxX <= 0) return;
	if (aabb.maxY <= 0) return;
	if (aabb.minX > this->rs.deviceW) return;
	if (aabb.minY > this->rs.deviceH) return;


	//new visible number - add it
	
	//fill basic structure info

	n.x = x;
	n.y = y;
	n.color = color;	
	n.anchor = anchor;
	n.type = type;
	n.anchorX = x;
	n.anchorY = y;
	n.w = w;
	n.h = h;

#ifdef THREAD_SAFETY
	std::lock_guard<std::shared_timed_mutex> lk(m);
#endif

	this->nmbrs.push_back(n);
	this->strChanged = true;
}

/// <summary>
/// Reverse fraction part
/// of positive number
/// </summary>
/// <param name="val">entire number</param>
/// <param name="intPart">integer part only</param>
/// <returns></returns>
unsigned long NumberRenderer::GetFractPartReversed(double val, unsigned long intPart) const
{
	double fractPart = val - intPart;

	//reversed number is without leading zeroes
	//eg: 0.0157 => will reverse to 751
	//but we need one leading zero
	unsigned long fractPartReverse = this->ReversDigits((unsigned long)(fractPart * decimalMult));
	if (fractPartReverse == 0)
	{
		return fractPartReverse;
	}

	//calculate number of leading zeroes
	int fractLeadingZeros = 0;	
	while (fractPart < 1)
	{
		fractLeadingZeros++;
		fractPart *= 10;
	}
	fractLeadingZeros--;

	//move reversed part 
	//751 => 7510
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
unsigned long NumberRenderer::ReversDigits(unsigned long num) const
{
	if (num < 10) return num;

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
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="negative"></param>
/// <param name="intPart"></param>
/// <param name="fractPartReversed"></param>
/// <returns></returns>
AbstractRenderer::AABB NumberRenderer::CalcNumberAABB(double val, int x, int y,
	bool negative, unsigned long intPart, unsigned long fractPartReversed)
{

	AbstractRenderer::AABB aabb;
		
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
		++lastDigit;
		intPart /= 10;
	} while (intPart);

	while (lastDigit > 0)
	{
		--lastDigit;
		const GlyphInfo & gi = this->gi[digits[lastDigit] + '0'];
				
		int fx = x + gi.bmpX;
		int fy = y - gi.bmpY;


		if (fx < aabb.minX) aabb.minX = fx;
		if (fy < aabb.minY) aabb.minY = fy;


		if (fx + gi.bmpW > aabb.maxX) aabb.maxX = fx + gi.bmpW;
		if (fy + gi.bmpH > aabb.maxY) aabb.maxY = fy + gi.bmpH;

		x += (gi.adv >> 6);
	}




	if (fractPartReversed)
	{
		GlyphInfo & gi = this->gi['.'];

		int fx = x + gi.bmpX;
		int fy = y - gi.bmpY;

		if (fx < aabb.minX) aabb.minX = fx;
		if (fy < aabb.minY) aabb.minY = fy;


		if (fx + gi.bmpW > aabb.maxX) aabb.maxX = fx + gi.bmpW;
		if (fy + gi.bmpH > aabb.maxY) aabb.maxY = fy + gi.bmpH;

		x += (gi.adv >> 6);

		
		while (fractPartReversed)
		{			
			int cc = (fractPartReversed % 10);
			fractPartReversed /= 10;
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
	for (auto & si : this->nmbrs)
	{		
		if (si.anchor == TextAnchor::LEFT_TOP)
		{
			si.anchorX = si.x;
			si.anchorY = si.y + this->newLineOffset; //y position is "line letter start" - move it to letter height
		}
		else if (si.anchor == TextAnchor::CENTER)
		{			
			si.anchorX = si.x - si.w / 2;
			si.anchorY = si.y + (this->newLineOffset / 2); //move top position to TOP_LEFT
													//and calc center from all lines and move TOP_LEFT down
		}
		else if (si.anchor == TextAnchor::LEFT_DOWN)
		{
			si.anchorX = si.x;
			si.anchorY = si.y;			
		}

		if (si.type == TextType::CAPTION)
		{									
			si.anchorY -= (si.h / 2 + ci.offset);	
			si.anchorY -= 2 * (this->captionMark.bmpH);
		}				
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
	
    AbstractRenderer::Clear();
	this->geom.reserve(400);
	
	for (const NumberRenderer::NumberInfo & si : this->nmbrs)
	{		
		int x = si.anchorX;
		int y = si.anchorY;


		unsigned long intPart = si.intPart;
		unsigned long fractPartReverse = si.fractPartReverse;

		if (si.type == TextType::CAPTION)
		{			
			
			int xx = si.x - (this->captionMark.bmpW) / 2;
			int yy = si.y + (this->captionMark.bmpH);
			

			this->AddQuad(this->captionMark, xx, yy, si.color);
		}



		if (si.negative)
		{
			this->AddQuad(this->gi['-'], x, y, si.color);
			x += (this->gi['-'].adv >> 6);
		}

		
		int lastDigit = 0;

		//split number to single digits				
		/*
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

			this->AddQuad(gi, x, y, si.color);

			x += (gi.adv >> 6);			
		}
		*/
		
		//==========================================================
		//split number to double-digits
		//optimized conversion from number to "string (glyphs)"
		//first we use stack and fill it with double-digits
		//and then pop stack and append double-digits to output

		if (intPart < 9)
		{
			//one difgit number
			const GlyphInfo & gi = this->gi[intPart + '0'];
			this->AddQuad(gi, x, y, si.color);
			x += (gi.adv >> 6);
		}
		else
		{
			//number is > 9 - it has at least two digist
			while (intPart > 9)
			{
				int tmp = intPart / 100;
				digits[lastDigit] = intPart - 100 * tmp;
				++lastDigit;
				intPart = tmp;
			};

			if (intPart != 0)
			{
				const GlyphInfo & gi = this->gi[intPart + '0'];
				this->AddQuad(gi, x, y, si.color);
				x += (gi.adv >> 6);
			}

			while (lastDigit > 0)
			{
				--lastDigit;
				GlyphInfo ** t = precompGi[digits[lastDigit]];

				this->AddQuad(*t[1], x, y, si.color);
				x += (t[1]->adv >> 6);

				this->AddQuad(*t[0], x, y, si.color);
				x += (t[0]->adv >> 6);
			}
		}
		//==========================================================

		if (fractPartReverse)
		{
			this->AddQuad(this->gi['.'], x, y, si.color);
			x += (this->gi['.'].adv >> 6);
			
			while (fractPartReverse)
			{
				int cc = (fractPartReverse % 10);
				fractPartReverse /= 10;
				const GlyphInfo & gi = this->gi[cc + '0'];

				this->AddQuad(gi, x, y, si.color);

				x += (gi.adv >> 6);
			}
		}		
	}

	this->strChanged = false;

	this->FillVB();

	return true;
}
