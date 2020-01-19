#ifndef STRING_RENDERER_H
#define STRING_RENDERER_H

class IFontShaderManager;

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"

class StringRenderer : public AbstractRenderer
{
public:

	static StringRenderer * CreateSingleColor(Color color, const std::vector<Font> & fs, RenderSettings r, int glVersion = 3);

	StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion = 3);
    StringRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion,
                   const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm);
	~StringRenderer();
		
	void Clear();
	size_t GetStringsCount() const;
	
	void SetNewLineOffset(int offsetInPixels);

	void SetBidiEnabled(bool val);

	bool AddStringCaption(const UnicodeString & str,
		double x, double y, Color color = DEFAULT_COLOR);

	bool AddStringCaption(const UnicodeString & str,
		int x, int y, Color color = DEFAULT_COLOR);

	bool AddString(const UnicodeString & str,
		double x, double y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	bool AddString(const UnicodeString & str,
		int x, int y, Color color = DEFAULT_COLOR,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

protected:

	typedef std::vector<std::tuple<FontInfo::GlyphLutIterator, bool, FontInfo *>> UsedGlyphCache;
	
	typedef struct StringAABB
	{
		std::vector<AABB> lines;
		AABB global;

		int maxNewLineOffset;
		int lastLineOffset;
		int totalLineOffset;

	} StringAABB;

	typedef struct StringInfo
	{
		UnicodeString str;
		int x;
		int y;
		Color color;
		TextAnchor anchor;
		TextAlign align;
		TextType type;

		int anchorX;
		int anchorY;
		StringAABB aabb;

		StringInfo(UnicodeString & str, int x, int y, 
			Color c, TextAnchor anchor, 
			TextAlign align, TextType type) : 
		str(str), x(x), y(y), color(c), anchor(anchor), align(align),
		type(type), anchorX(x), anchorY(y) {}
               
	} StringInfo;


	bool isBidiEnabled;
	std::vector<StringInfo> strs;
	int nlOffsetPx;

	bool spaceSizeExist;
	long spaceSize;
	long CalcSpaceSize();

	bool CanAddString(const UnicodeString & uniStr,
		int x, int y, Color color,
		TextAnchor anchor, TextAlign align, TextType type) const;

	bool AddStringInternal(const UnicodeString & str,
		int x, int y, Color color = { 1,1,1,1 },
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT,
		TextType type = TextType::TEXT);

	bool GenerateGeometry() override;

	AABB EstimateStringAABB(const UnicodeString & str, int x, int y) const;
	StringAABB CalcStringAABB(const UnicodeString & str, int x, int y, const UsedGlyphCache * gc) const;

	void CalcAnchoredPosition();
	void CalcLineAlign(const StringInfo & si, int lineId, int & x, int & y) const;

	UsedGlyphCache ExtractGlyphs(const UnicodeString & str);
};

#endif
