#ifndef _NUMBER_RENDERER_H_
#define _NUMBER_RENDERER_H_

#include <type_traits>

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"

#include "./FontStructures.h"

#define IS_FLOAT typename std::enable_if<std::is_floating_point<T>::value, void>::type
#define IS_INTEGRAL typename std::enable_if<std::is_integral<T>::value, void>::type

class NumberRenderer : public AbstractRenderer
{
public:

	static const std::string NUMBERS_STRING;

	NumberRenderer(Font fs, RenderSettings r, int glVersion = 3);
	~NumberRenderer();

	void SetExistenceCheck(bool val);
	void SetDecimalPrecission(int digits);

	void Clear();
	size_t GetNumbersCount() const;

	template <typename T>
	IS_FLOAT AddNumberCaption(T val,
		int x, int y, Color color = DEFAULT_COLOR);

	template <typename T>
	IS_FLOAT AddNumber(T val,
		double x, double y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

	template <typename T>
	IS_FLOAT AddNumber(T val,
		int x, int y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP);


	template <typename T>
	IS_INTEGRAL AddNumberCaption(T val,
		int x, int y, Color color = DEFAULT_COLOR);

	template <typename T>
	IS_INTEGRAL AddNumber(T val,
		double x, double y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

	template <typename T>
	IS_INTEGRAL AddNumber(T val,
		int x, int y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

			
protected:

	
	typedef struct NumberInfo
	{
		double val;
		bool negative;
		unsigned long intPart;
		unsigned long fractPartReverse;
			
		int x;
		int y;
		Color color;
		bool isDefaultColor;
		TextAnchor anchor;		
		TextType type;
		
		int anchorX;
		int anchorY;		
		AABB aabb;

	} StringInfo;

	bool checkIfExist;

	int decimalPlaces;
	double decimalMult;
	std::vector<NumberInfo> nmbrs;
	GlyphInfo gi[65];
	GlyphInfo captionMark;

	char digits[20];

	void AddFloatNumberInternal(double value,
		int x, int y, Color color = { 1,1,1,1 },
		TextAnchor anchor = TextAnchor::LEFT_TOP,		
		TextType type = TextType::TEXT);

	void AddIntegralNumberInternal(long value,
		int x, int y, Color color = { 1,1,1,1 },
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextType type = TextType::TEXT);

	void AddNumber(NumberInfo & n, int x, int y, Color color,
		TextAnchor anchor, TextType type);

	bool GenerateGeometry() override;

	AbstractRenderer::AABB CalcNumberAABB(double val, int x, int y, 
		bool negative, unsigned long intPart, unsigned long fractPartReversed);

	
	void CalcAnchoredPosition();
	
	void AddQuad(const GlyphInfo & gi, int x, int y, const NumberInfo & ni);

	unsigned long GetFractPartReversed(double val, unsigned long intPart);
	unsigned long ReversDigits(unsigned long num);
};


/// <summary>
/// Add new number as caption
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
template <typename T>
IS_FLOAT NumberRenderer::AddNumberCaption(T val,
	int x, int y, Color color)
{
	//this->AddNumberInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	this->AddFloatNumberInternal(val, x, y, color, TextAnchor::CENTER, TextType::CAPTION);
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
template <typename T>
IS_FLOAT NumberRenderer::AddNumber(T val,
	double x, double y, Color color,
	TextAnchor anchor)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	this->AddFloatNumberInternal(val, xx, yy, color, anchor, TextType::TEXT);
}

/// <summary>
/// Add new number to be rendered
/// If number already exist - do not add
/// Existing => same x, y, align, value
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
template <typename T>
IS_FLOAT NumberRenderer::AddNumber(T val,
	int x, int y, Color color,
	TextAnchor anchor)
{
	this->AddFloatNumberInternal(val, x, y, color, anchor, TextType::TEXT);
}


//-------

/// <summary>
/// Add new number as caption
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
template <typename T>
IS_INTEGRAL NumberRenderer::AddNumberCaption(T val,
	int x, int y, Color color)
{
	//this->AddNumberInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	this->AddIntegralNumberInternal(static_cast<long>(val), x, y, color, TextAnchor::CENTER, TextType::CAPTION);
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
template <typename T>
IS_INTEGRAL NumberRenderer::AddNumber(T val,
	double x, double y, Color color,
	TextAnchor anchor)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	this->AddIntegralNumberInternal(static_cast<long>(val), xx, yy, color, anchor, TextType::TEXT);
}

/// <summary>
/// Add new number to be rendered
/// If number already exist - do not add
/// Existing => same x, y, align, value
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
template <typename T>
IS_INTEGRAL NumberRenderer::AddNumber(T val,
	int x, int y, Color color,
	TextAnchor anchor)
{
	this->AddIntegralNumberInternal(static_cast<long>(val), x, y, color, anchor, TextType::TEXT);
}


#endif
