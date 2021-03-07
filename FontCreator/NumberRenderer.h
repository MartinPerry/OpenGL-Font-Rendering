#ifndef NUMBER_RENDERER_H
#define NUMBER_RENDERER_H

#include <type_traits>

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"

#include "./FontStructures.h"

#define IS_FLOAT typename std::enable_if<std::is_floating_point<T>::value, bool>::type
#define IS_INTEGRAL typename std::enable_if<std::is_integral<T>::value, bool>::type

class NumberRenderer : public AbstractRenderer
{
public:

	static const std::string NUMBERS_STRING;

	static NumberRenderer * CreateSingleColor(Color color, const FontBuilderSettings& fs, const RenderSettings& r, int glVersion = 3);
	static NumberRenderer* CreateSingleColor(Color color, std::shared_ptr<FontBuilder> fb, const RenderSettings& r, int glVersion = 3);


	NumberRenderer(const FontBuilderSettings& fs, const RenderSettings& r, int glVersion = 3);
	NumberRenderer(std::shared_ptr<FontBuilder> fb, const RenderSettings& r, int glVersion = 3);
	NumberRenderer(const FontBuilderSettings& fs, const RenderSettings& r, int glVersion,
		const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm);
	NumberRenderer(std::shared_ptr<FontBuilder> fb, const RenderSettings& r, int glVersion,
		const char* vSource, const char* pSource, std::shared_ptr<IFontShaderManager> sm);
	~NumberRenderer();

	void SetExistenceCheck(bool val) noexcept;
	void SetDecimalPrecission(int digits) noexcept;

	int GetDecimalPrecission() const noexcept;

	void Clear();
	size_t GetNumbersCount() const noexcept;

	template <typename T>
	IS_FLOAT AddNumberCaption(T val,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	template <typename T>
	IS_FLOAT AddNumberCaption(T val,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS);

	template <typename T>
	IS_FLOAT AddNumber(T val,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

	template <typename T>
	IS_FLOAT AddNumber(T val,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP);


	template <typename T>
	IS_INTEGRAL AddNumberCaption(T val,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	template <typename T>
	IS_INTEGRAL AddNumberCaption(T val,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS);

	template <typename T>
	IS_INTEGRAL AddNumber(T val,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

	template <typename T>
	IS_INTEGRAL AddNumber(T val,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

			
protected:

	
	typedef struct NumberInfo
	{
		double val;
		bool negative;
		uint64_t intPartOrder;
		uint32_t intPart;		
		uint32_t fractPartReverse;
					
		RenderParams renderParams;
		bool isDefaultColor;
		TextAnchor anchor;		
		TextType type;
			
		int x;
		int y;
		int w;
		int h;	

		NumberInfo() noexcept :
			NumberInfo(0.0)
		{
		}

		NumberInfo(double val) noexcept :
			val((val < 0) ? -val : val),
			negative(val < 0),
			intPartOrder(static_cast<unsigned long>(val)),
			intPart(0),
			fractPartReverse(0),
			isDefaultColor(true),
			anchor(TextAnchor::CENTER),
			type(TextType::TEXT),
			x(0),
			y(0),
			w(0),
			h(0)
		{}

	} NumberInfo;

	struct Precomputed 
	{
		GlyphInfo * gi[2];
		AbstractRenderer::AABB aabb;
		int xOffset;
	};

	bool checkIfExist;
	int newLineOffset;

	int decimalPlaces;
	double decimalMult;
	std::vector<NumberInfo> nmbrs;
	GlyphInfo gi[65];
	GlyphInfo captionMark;
	Precomputed precomputed[100];
	

	void Init();
	void Precompute();

	bool AddFloatNumberInternal(double value,
		int x, int y, const RenderParams & rp,
		TextAnchor anchor = TextAnchor::LEFT_TOP,		
		TextType type = TextType::TEXT);

	bool AddIntegralNumberInternal(long value,
		int x, int y, const RenderParams & rp,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextType type = TextType::TEXT);

	bool AddNumber(NumberInfo & n, int x, int y, const RenderParams & rp,
		TextAnchor anchor, TextType type);

	bool GenerateGeometry() override;

	AbstractRenderer::AABB CalcNumberAABB(double val, int x, int y, 
		bool negative, uint32_t intPart, uint64_t intPartOrder, uint32_t fractPartReversed);

	
	void GetAnchoredPosition(const NumberRenderer::NumberInfo & si, int & x, int & y);
	

	uint32_t GetFractPartReversed(double val, uint32_t intPart) const noexcept;
	uint32_t ReversDigits(uint32_t num) const noexcept;
	uint64_t GetIntDivisor(const uint32_t x) const noexcept;
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
	int x, int y, const RenderParams & rp)
{
	//this->AddNumberInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	return this->AddFloatNumberInternal(val, x, y, rp, TextAnchor::CENTER, TextType::CAPTION);
}

/// <summary>
/// Add new number as caption
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
template <typename T>
IS_FLOAT NumberRenderer::AddNumberCaption(T val,
	double x, double y, const RenderParams & rp)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	//this->AddNumberInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	return this->AddFloatNumberInternal(val, xx, yy, rp, TextAnchor::CENTER, TextType::CAPTION);
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
	double x, double y, const RenderParams & rp,
	TextAnchor anchor)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	return this->AddFloatNumberInternal(val, xx, yy, rp, anchor, TextType::TEXT);
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
	int x, int y, const RenderParams & rp,
	TextAnchor anchor)
{
	return this->AddFloatNumberInternal(val, x, y, rp, anchor, TextType::TEXT);
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
	int x, int y, const RenderParams & rp)
{
	//this->AddNumberInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	return this->AddIntegralNumberInternal(static_cast<long>(val), x, y, rp, TextAnchor::CENTER, TextType::CAPTION);
}

/// <summary>
/// Add new number as caption
/// </summary>
/// <param name="val"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
template <typename T>
IS_INTEGRAL NumberRenderer::AddNumberCaption(T val,
	double x, double y, const RenderParams & rp)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	//this->AddNumberInternal(ci.mark, x, y, color, TextAnchor::CENTER, TextAlign::ALIGN_CENTER, TextType::CAPTION);
	return this->AddIntegralNumberInternal(static_cast<long>(val), xx, yy, rp, TextAnchor::CENTER, TextType::CAPTION);
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
	double x, double y, const RenderParams & rp,
	TextAnchor anchor)
{
	int xx = static_cast<int>(x * this->rs.deviceW);
	int yy = static_cast<int>(y * this->rs.deviceH);

	return this->AddIntegralNumberInternal(static_cast<long>(val), xx, yy, rp, anchor, TextType::TEXT);
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
	int x, int y, const RenderParams & rp,
	TextAnchor anchor)
{
	return this->AddIntegralNumberInternal(static_cast<long>(val), x, y, rp, anchor, TextType::TEXT);
}


#endif
