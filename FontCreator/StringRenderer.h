#ifndef _STRING_RENDERER_H_
#define _STRING_RENDERER_H_

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"

class StringRenderer : public AbstractRenderer
{
public:

	StringRenderer(int deviceW, int deviceH, Font f);
	~StringRenderer();

	void SetCaptionInfo(const utf8_string & mark, int offset);

	void ClearStrings();

	
	void AddStringCaption(const utf8_string & strUTF8,
		double x, double y, Color color = DEFAULT_COLOR);

	void AddStringCaption(const utf8_string & strUTF8,
		int x, int y, Color color = DEFAULT_COLOR);

	void AddString(const utf8_string & strUTF8,
		double x, double y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	void AddString(const utf8_string & strUTF8,
		int x, int y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

protected:

	
	typedef struct StringInfo
	{
		utf8_string strUTF8;
		int x;
		int y;
		Color color;
		bool isDefaultColor;
		TextAnchor anchor;
		TextAlign align;
		TextType type;

		int linesCount;
		int anchorX;
		int anchorY;
		std::vector<AABB> linesAABB;
		AABB aabb;

	} StringInfo;


	std::vector<StringInfo> strs;
	


	void AddStringInternal(const utf8_string & strUTF8,
		int x, int y, Color color = { 1,1,1,1 },
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT,
		TextType type = TextType::TEXT);

	bool GenerateGeometry() override;

	AABB EstimateStringAABB(const utf8_string & strUTF8,
		int x, int y);
	std::vector<AABB> CalcStringAABB(const utf8_string & strUTF8,
		int x, int y, AABB & globalAABB);
	int CalcStringLines(const utf8_string & strUTF8) const;
	void CalcAnchoredPosition();
	void CalcLineAlign(const StringInfo & si, int lineId, int & x, int & y) const;

};

#endif