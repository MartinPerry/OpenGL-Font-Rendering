#ifndef _NUMBER_RENDERER_H_
#define _NUMBER_RENDERER_H_

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"

#include "./FontStructures.h"

class NumberRenderer : public AbstractRenderer
{
public:

	static const utf8_string NUMBERS_STRING;

	NumberRenderer(Font fs, RenderSettings r);
	~NumberRenderer();

	void SetExistenceCheck(bool val);
	void SetDecimalPrecission(int digits);

	void Clear();
	size_t GetNumbersCount() const;

	void AddNumberCaption(double val,
		int x, int y, Color color = DEFAULT_COLOR);

	void AddNumber(double val,
		double x, double y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP);

	void AddNumber(double val,
		int x, int y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP);


	
protected:

	
	typedef struct NumberInfo
	{
		double val;
		bool negative;
		unsigned long intPart;
		unsigned long fractPart;
			

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


	void AddNumberInternal(double value,
		int x, int y, Color color = { 1,1,1,1 },
		TextAnchor anchor = TextAnchor::LEFT_TOP,		
		TextType type = TextType::TEXT);

	bool GenerateGeometry() override;

	AbstractRenderer::AABB CalcNumberAABB(double val, int x, int y);

	
	void CalcAnchoredPosition();
	
	void AddQuad(const GlyphInfo & gi, int x, int y, const NumberInfo & ni);

};

#endif
