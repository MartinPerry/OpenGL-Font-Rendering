#ifndef _STRING_RENDERER_H_
#define _STRING_RENDERER_H_

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"

class StringRenderer : public AbstractRenderer
{
public:

	StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion = 3);
	~StringRenderer();
		
	void Clear();
	size_t GetStringsCount() const;
	
	void SetNewLineOffset(int offsetInPixels);

	void SetBidiEnabled(bool val);

	void AddStringCaption(const UnicodeString & str,
		double x, double y, Color color = DEFAULT_COLOR);

	void AddStringCaption(const UnicodeString & str,
		int x, int y, Color color = DEFAULT_COLOR);

	void AddString(const UnicodeString & str,
		double x, double y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	void AddString(const UnicodeString & str,
		int x, int y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

protected:

	typedef std::vector<std::tuple<FontInfo::UsedGlyphIterator, bool, FontInfo *>> UsedGlyphCache;
	
	typedef struct StringInfo
	{
		UnicodeString str;
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


	bool isBidiEnabled;
	std::vector<StringInfo> strs;
	int nlOffsetPx;


	void AddStringInternal(const UnicodeString & str,
		int x, int y, Color color = { 1,1,1,1 },
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT,
		TextType type = TextType::TEXT);

	bool GenerateGeometry() override;

	AABB EstimateStringAABB(const UnicodeString & str, int x, int y);
	std::vector<AABB> CalcStringAABB(const UnicodeString & str,
		int x, int y, AABB & globalAABB, const UsedGlyphCache * gc);

	int CalcStringLines(const UnicodeString & str) const;
	void CalcAnchoredPosition();
	void CalcLineAlign(const StringInfo & si, int lineId, int & x, int & y) const;

	UsedGlyphCache ExtractGlyphs(const UnicodeString & str);
};

#endif
