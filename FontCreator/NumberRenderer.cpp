#include "./NumberRenderer.h"

#include <limits>
#include <algorithm>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"


const std::string NumberRenderer::NUMBERS_STRING = "0123456789,.-";

/// <summary>
/// Create optimized Number renderer that will use only one color for every number
/// If we pass color parameter during AddNumber... method call,
/// this parameter is ignored and number is rendered with color specified here
/// </summary>
/// <param name="color">color used for all numbers</param>
/// <param name="fs"></param>
/// <param name="r"></param>
/// <param name="glVersion"></param>
/// <returns></returns>
NumberRenderer * NumberRenderer::CreateSingleColor(Color color, const std::vector<Font> & fs, RenderSettings r, int glVersion)
{
	
	auto sm = std::make_shared<SingleColorFontShaderManager>();
	sm->SetColor(color.r, color.g, color.b, color.a);

	return new NumberRenderer(fs, r, glVersion,
		SINGLE_COLOR_VERTEX_SHADER_SOURCE, SINGLE_COLOR_PIXEL_SHADER_SOURCE,
		sm);

}

/// <summary>
/// ctor
/// </summary>
/// <param name="fs"></param>
/// <param name="r"></param>
/// <param name="glVersion"></param>
NumberRenderer::NumberRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion)
	: AbstractRenderer(fs, r, glVersion),
	decimalPlaces(0),
	decimalMult(1),
	checkIfExist(true)
{
	this->Init();
}

NumberRenderer::NumberRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion,
	const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm)
	: AbstractRenderer(fs, r, glVersion, vSource, pSource, sm),
	decimalPlaces(0),
	decimalMult(1),
	checkIfExist(true)
{
	this->Init();
}

/// <summary>
/// dtor
/// </summary>
NumberRenderer::~NumberRenderer()
{
}

void NumberRenderer::Init()
{	
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
/// Precompute array of double-digits glyphs
/// </summary>
void NumberRenderer::Precompute()
{
	char digits[20];
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

		this->precomputed[i].gi[0] = &this->gi['0'];
		this->precomputed[i].gi[1] = this->precomputed[i].gi[0];

		while (lastDigit > 0)
		{
			lastDigit--;			
			this->precomputed[i].gi[lastDigit] = &this->gi[digits[lastDigit] + '0'];
		}

		
		//precompute AABBs for double numbers
		int x = 0;		
		for (int j = 0; j < 2; j++)
		{
			const GlyphInfo * gi = this->precomputed[i].gi[j];
			this->precomputed[i].aabb.Update(x + gi->bmpX, -gi->bmpY, gi->bmpW, gi->bmpH);			
			x += (gi->adv >> 6);
		}
		this->precomputed[i].xOffset = x;

	}
}

/// <summary>
/// Set whether we want to check if added number already exist
/// </summary>
/// <param name="val"></param>
void NumberRenderer::SetExistenceCheck(bool val) noexcept
{
	this->checkIfExist = val;
}

/// <summary>
/// Set precision for decimal part in digits count
/// </summary>
/// <param name="digits"></param>
void NumberRenderer::SetDecimalPrecission(int digits) noexcept
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
int NumberRenderer::GetDecimalPrecission() const noexcept
{
	return this->decimalPlaces;
}

/// <summary>
/// Get number of numbers
/// </summary>
/// <returns></returns>
size_t NumberRenderer::GetNumbersCount() const noexcept
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
bool NumberRenderer::AddIntegralNumberInternal(long val,
	int x, int y, const RenderParams & rp,
	TextAnchor anchor, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::AxisYOrigin::DOWN)
	{
		y = this->rs.deviceH - y;
	}

	if (this->checkIfExist)
	{
		for (const NumberInfo & s : this->nmbrs)
		{
			if ((s.x == x) && (s.y == y) &&
				(s.anchor == anchor) && (s.type == type))
			{
				if (s.val == val)
				{
					//same number on the same position and with same align
					//already exist - do not add it again
					return false;
				}
			}
		}
	}

	NumberInfo i;
	i.val = val;
	if (val < 0)
	{
		i.negative = true;
		val *= -1;
	}
	
	i.intPart = static_cast<unsigned long>(val);
	i.intPartOrder = this->GetIntDivisor(i.intPart);	

	return this->AddNumber(i, x, y, rp, anchor, type);	
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
bool NumberRenderer::AddFloatNumberInternal(double val,
	int x, int y, const RenderParams & rp,
	TextAnchor anchor, TextType type)
{
	if (this->axisYOrigin == AbstractRenderer::AxisYOrigin::DOWN)
	{
		y = this->rs.deviceH - y;
	}

	if (this->checkIfExist)
	{
		for (const NumberInfo & s : this->nmbrs)
		{
			if ((s.x == x) && (s.y == y) &&
				(s.anchor == anchor) && (s.type == type))
			{
				if (s.val == val)
				{
					//same number on the same position and with same align
					//already exist - do not add it again
					return false;
				}
			}
		}
	}

	NumberInfo i;
	i.val = val;
	if (val < 0)
	{
		i.negative = true;
		val *= -1;
	}	
	
	i.intPart = static_cast<uint32_t>(val);
	i.intPartOrder = this->GetIntDivisor(i.intPart);
	i.fractPartReverse = this->GetFractPartReversed(val, i.intPart);

	//remove negative zero
	if ((i.negative) && (i.fractPartReverse == 0) && (i.intPart == 0))
	{
		i.negative = false;
	}

	return this->AddNumber(i, x, y, rp, anchor, type);
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
bool NumberRenderer::AddNumber(NumberInfo & n, int x, int y, const RenderParams & rp,
	TextAnchor anchor, TextType type)
{
	AbstractRenderer::AABB aabb = this->CalcNumberAABB(n.val, x, y, n.negative, 
		n.intPart, n.intPartOrder, n.fractPartReverse);

	//test if entire string is outside visible area

	const float w = (aabb.maxX - aabb.minX);
	const float h = (aabb.maxY - aabb.minY);

	if (anchor == TextAnchor::CENTER)
	{	
		const float wHalf = w / 2.0f;
		const float hHalf = h / 2.0f;

		aabb.minX -= wHalf;
		aabb.maxX -= wHalf;
		aabb.minY -= hHalf;
		aabb.maxY -= hHalf;
	}

	if (aabb.maxX <= 0) return false;
	if (aabb.maxY <= 0) return false;
	if (aabb.minX > this->rs.deviceW) return false;
	if (aabb.minY > this->rs.deviceH) return false;


	//new visible number - add it
	
	//fill basic structure info
	
	n.renderParams = rp;
	n.anchor = anchor;
	n.type = type;	
	n.x = x;
	n.y = y;
	n.w = w;
	n.h = h;

#ifdef THREAD_SAFETY
	std::lock_guard<std::shared_timed_mutex> lk(m);
#endif

	this->nmbrs.push_back(n);
	this->strChanged = true;

	return true;
}

/// <summary>
/// Reverse fraction part
/// of positive number
/// </summary>
/// <param name="val">entire number</param>
/// <param name="intPart">integer part only</param>
/// <returns></returns>
uint32_t NumberRenderer::GetFractPartReversed(double val, uint32_t intPart) const noexcept
{
	double fractPart = val - intPart;

	//reversed number is without leading zeroes
	//eg: 0.0157 => will reverse to 751
	//but we need one leading zero
	uint32_t fractPartReverse = this->ReversDigits((uint32_t)(fractPart * decimalMult));
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
uint32_t NumberRenderer::ReversDigits(uint32_t num) const noexcept
{
	if (num < 10) return num;

	uint32_t rev_num = 0;
	while (num > 0)
	{
		rev_num = rev_num * 10 + num % 10;
		num = num / 10;
	}
	return rev_num;
}

/// <summary>
/// Get starting integer divisor if integer is divided by 100
/// to get first two digits
/// </summary>
/// <param name="x"></param>
/// <returns></returns>
uint32_t NumberRenderer::GetIntDivisor(const uint32_t x) const noexcept
{
	if (x >= 10000U) {
		if (x >= 10000000U) {
			if (x >= 100000000U) {
				if (x >= 1000000000U) return 10000000000U;
				return 1000000000U;
			}
			return 100000000U;
		}
		if (x >= 100000U) {
			if (x >= 1000000U) return 10000000U;
			return 1000000U;
		}
		return 100000U;
	}
	if (x >= 100U) {
		if (x >= 1000U) return 10000U;
		return 1000U;
	}
	if (x >= 10U) return 100U;
	return 1U;
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
	bool negative, uint32_t intPart, uint32_t intPartOrder, uint32_t fractPartReversed)
{	
	//store offsets
	int xOffset = x;
	int yOffset = y;

	//reset position to count form zero
	y = 0; //not used
	x = 0;

	AbstractRenderer::AABB aabb;
	
	if (negative)
	{
		const GlyphInfo & gi = this->gi['-'];
		
		aabb.Update(x + gi.bmpX, -gi.bmpY, gi.bmpW, gi.bmpH);		
		x += (gi.adv >> 6);		
	}

	if (intPart <= 9)
	{
		//one digit number
		const GlyphInfo & gi = this->gi[intPart + '0'];

		aabb.Update(x + gi.bmpX, -gi.bmpY, gi.bmpW, gi.bmpH);
		x += (gi.adv >> 6);		
	}
	else
	{
		int divisor = intPartOrder;
		do
		{
			divisor /= 100;
			const int tmp = intPart / divisor;
			const Precomputed & t = precomputed[tmp];
			
			aabb.UnionWithOffset(t.aabb, x);
			x += t.xOffset;

			intPart = intPart - tmp * divisor;			

		} while (divisor > 10);

		if (divisor >= 10)
		{
			//one digit remaining number
			const GlyphInfo & gi = this->gi[intPart + '0'];
			
			aabb.Update(x + gi.bmpX, -gi.bmpY, gi.bmpW, gi.bmpH);
			x += (gi.adv >> 6);
		}
	}

	
	if (fractPartReversed)
	{
		const GlyphInfo & gi = this->gi['.'];
		
		aabb.Update(x + gi.bmpX, -gi.bmpY, gi.bmpW, gi.bmpH);
		x += (gi.adv >> 6);
		
		while (fractPartReversed)
		{			
			const int cc = (fractPartReversed % 10);			
			const GlyphInfo & gi = this->gi[cc + '0'];
			
			aabb.Update(x + gi.bmpX, -gi.bmpY, gi.bmpW, gi.bmpH);
			x += (gi.adv >> 6);			

			fractPartReversed /= 10;
		}
	}
	
	//translate bounding box to original position
	aabb.minX += xOffset;
	aabb.minY += yOffset;
	aabb.maxX += xOffset;
	aabb.maxY += yOffset;

	return aabb;
}

/// <summary>
/// Calculate start position of text using anchors
/// </summary>
void NumberRenderer::GetAnchoredPosition(const NumberRenderer::NumberInfo & si, 
	int & x, int & y)
{
	//default for TextAnchor::LEFT_DOWN
	x = si.x;
	y = si.y;

	//Calculate anchored position of text	
	if (si.anchor == TextAnchor::LEFT_TOP)
	{		
		y = si.y + this->newLineOffset; //y position is "line letter start" - move it to letter height
	}
	else if (si.anchor == TextAnchor::CENTER)
	{			
		x = si.x - si.w / 2;
		y = si.y + (this->newLineOffset / 2); //move top position to TOP_LEFT
											//and calc center from all lines and move TOP_LEFT down
	}
	
	if (si.type == TextType::CAPTION)
	{				
		y -= (si.h / 2 + ci.offset);	
		y -= 2 * this->captionMark.bmpH;
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
			
	//Build geometry	
    AbstractRenderer::Clear();
	this->geom.reserve(400);
	
	for (const NumberRenderer::NumberInfo & si : this->nmbrs)
	{		
		int x, y;
		this->GetAnchoredPosition(si, x, y);
		
		if (si.type == TextType::CAPTION)
		{						
			const int xx = si.x - (this->captionMark.bmpW) / 2;
			const int yy = si.y + (this->captionMark.bmpH);
			
			this->AddQuad(this->captionMark, xx, yy, si.renderParams);
		}

		if (si.negative)
		{
			this->AddQuad(this->gi['-'], x, y, si.renderParams);
			x += (this->gi['-'].adv >> 6);
		}				
		
		//==========================================================
		//split number to double-digits
		//optimized conversion from number to "string (glyphs)"
		
		uint32_t intPart = si.intPart;	
		
		if (intPart <= 9)
		{
			//one digit number
			const GlyphInfo & gi = this->gi[intPart + '0'];
			this->AddQuad(gi, x, y, si.renderParams);
			x += (gi.adv >> 6);
		}
		else				
		{	
			//at least two digits number
			int divisor = si.intPartOrder;

			do
			{
				divisor /= 100;

				const int tmp = intPart / divisor;
				const GlyphInfo * const * t = precomputed[tmp].gi;

				this->AddQuad(*t[1], x, y, si.renderParams);
				x += (t[1]->adv >> 6);

				this->AddQuad(*t[0], x, y, si.renderParams);
				x += (t[0]->adv >> 6);

				intPart = intPart - tmp * divisor;
				
			} while (divisor > 10);

			if (divisor >= 10)
			{
				//one digit remaining number
				const GlyphInfo & gi = this->gi[intPart + '0'];
				this->AddQuad(gi, x, y, si.renderParams);
				x += (gi.adv >> 6);
			}
		}
		
		//==========================================================

		uint32_t fractPartReverse = si.fractPartReverse;
		if (fractPartReverse)
		{
			this->AddQuad(this->gi['.'], x, y, si.renderParams);
			x += (this->gi['.'].adv >> 6);
			
			while (fractPartReverse)
			{
				const int cc = (fractPartReverse % 10);				
				const GlyphInfo & gi = this->gi[cc + '0'];

				this->AddQuad(gi, x, y, si.renderParams);
				x += (gi.adv >> 6);

				fractPartReverse /= 10;
			}
		}		
	}

	this->strChanged = false;

	this->FillVB();

	return true;
}
