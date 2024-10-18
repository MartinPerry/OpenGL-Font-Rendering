#ifndef NUMBER_RENDERER_H
#define NUMBER_RENDERER_H

class BackendBase;

#include <type_traits>

#include "./AbstractRenderer.h"

#include "../Externalncludes.h"

#include "../FontStructures.h"

#define IS_FLOAT typename std::enable_if<std::is_floating_point<T>::value, bool>::type
#define IS_INTEGRAL typename std::enable_if<std::is_integral<T>::value, bool>::type

class NumberRenderer : public AbstractRenderer
{
public:

	static const std::string NUMBERS_STRING;

	static NumberRenderer * CreateSingleColor(Color color, const FontBuilderSettings& fs, std::unique_ptr<BackendBase>&& backend);
	static NumberRenderer* CreateSingleColor(Color color, std::shared_ptr<FontBuilder> fb, std::unique_ptr<BackendBase>&& backend);


	NumberRenderer(const FontBuilderSettings& fs, std::unique_ptr<BackendBase>&& backend);
	NumberRenderer(std::shared_ptr<FontBuilder> fb, std::unique_ptr<BackendBase>&& backend);
	
	~NumberRenderer();

	void SetFontSize(const FontSize& fs, int defaultFontSizeInPx = 0);

	void SetExistenceCheck(bool val) noexcept;	
	void SetOverlapCheck(bool val) noexcept;
	void SetDecimalPrecission(int digits) noexcept;

	int GetDecimalPrecission() const noexcept;

	void Clear();
	size_t GetNumbersCount() const noexcept;

	template <typename T>
	IS_FLOAT AddNumberCaption(T val,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	template <typename T>
	IS_FLOAT AddNumberCaption(T val,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS);

	template <typename T>
	IS_FLOAT AddNumber(T val,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS,
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
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS);

	template <typename T>
	IS_INTEGRAL AddNumber(T val,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

	template <typename T>
	IS_INTEGRAL AddNumber(T val,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

			
protected:

	
	struct NumberInfo
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

		NumberInfo(double value) noexcept :
			NumberInfo(value, {}, TextAnchor::CENTER, TextType::TEXT)
		{
		}

		NumberInfo(double value, const RenderParams& renderParams, 
			TextAnchor anchor, TextType type) noexcept :
			val((value < 0) ? -value : value),
			negative(value < 0),
			intPart(static_cast<uint32_t>(val)),
			intPartOrder(0),
			fractPartReverse(0),
			renderParams(renderParams),
			isDefaultColor(true),
			anchor(anchor),
			type(type),
			x(0),
			y(0),
			w(0),
			h(0)
		{}

	};

	struct Precomputed 
	{
		GlyphInfo * gi[2];
		AABB aabb;
		int xOffset;
	};

	bool checkIfExist;
	bool overlapCheck;
	int newLineOffset;

	int decimalPlaces;
	double decimalMult;
	std::vector<NumberInfo> nmbrs;
	std::vector<AABB> nmbrsAABB;

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

	bool AddNumber(NumberInfo & n, int x, int y);

	bool GenerateGeometry() override;

	AABB CalcNumberAABB(double val, int x, int y,
		bool negative, uint32_t intPart, uint64_t intPartOrder, uint32_t fractPartReversed);

	
	void GetAnchoredPosition(const NumberRenderer::NumberInfo & si, int & x, int & y);
	

	uint32_t GetFractPartReversed(double val, uint32_t intPart) const noexcept;
	uint32_t ReversDigits(uint32_t num) const noexcept;
	uint64_t GetIntDivisor(const uint32_t x) const noexcept;
};

//====================================================================================
//====================================================================================
//====================================================================================

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
	return this->AddFloatNumberInternal(val, x, y, rp, TextAnchor::CENTER, TextType::CAPTION_TEXT);
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
	float x, float y, const RenderParams & rp)
{
	int xx = static_cast<int>(x * this->GetRenderSettings().deviceW);
	int yy = static_cast<int>(y * this->GetRenderSettings().deviceH);
	
	return this->AddFloatNumberInternal(val, xx, yy, rp, TextAnchor::CENTER, TextType::CAPTION_TEXT);
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
	float x, float y, const RenderParams & rp,
	TextAnchor anchor)
{
	int xx = static_cast<int>(x * this->GetRenderSettings().deviceW);
	int yy = static_cast<int>(y * this->GetRenderSettings().deviceH);

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
	return this->AddIntegralNumberInternal(static_cast<long>(val), x, y, rp, TextAnchor::CENTER, TextType::CAPTION_TEXT);
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
	float x, float y, const RenderParams & rp)
{
	int xx = static_cast<int>(x * this->GetRenderSettings().deviceW);
	int yy = static_cast<int>(y * this->GetRenderSettings().deviceH);
	
	return this->AddIntegralNumberInternal(static_cast<long>(val), xx, yy, rp, TextAnchor::CENTER, TextType::CAPTION_TEXT);
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
	float x, float y, const RenderParams & rp,
	TextAnchor anchor)
{
	int xx = static_cast<int>(x * this->GetRenderSettings().deviceW);
	int yy = static_cast<int>(y * this->GetRenderSettings().deviceH);

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
