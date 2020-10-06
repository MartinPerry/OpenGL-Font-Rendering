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

	bool AddStringCaption(const char * str,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const UnicodeString & str,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const char * str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const UnicodeString & str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddString(const char * str,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	bool AddString(const char * str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	bool AddString(const UnicodeString & str,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	bool AddString(const UnicodeString & str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

protected:

	typedef std::vector<std::tuple<FontInfo::GlyphLutIterator, bool, FontInfo *>> UsedGlyphCache;
	
	struct LineInfo 
	{
		AABB aabb;
		RenderParams renderParams;
		float maxNewLineOffset;

		LineInfo() : 
			aabb(AABB()),
			renderParams(DEFAULT_PARAMS),
			maxNewLineOffset(0.0)
		{}

		LineInfo(const RenderParams & rp) :
			aabb(AABB()),
			renderParams(rp),
			maxNewLineOffset(0.0)
		{}

	};

	/*
	typedef struct StringAABB
	{
		std::vector<AABB> lines;
		AABB global;

		float maxNewLineOffset;
		
	} StringAABB;
	*/

	typedef struct StringInfo
	{
		UnicodeString str;
		int x;
		int y;
		//RenderParams renderParams;
		TextAnchor anchor;
		TextAlign align;
		TextType type;

		float anchorX;
		float anchorY;
		//StringAABB aabb;

		std::vector<LineInfo> lines;		
		AABB global;

		StringInfo(UnicodeString & str, int x, int y, 
			TextAnchor anchor,
			TextAlign align, TextType type) : 
			str(str), 
			x(x), 
			y(y), 
			//renderParams(rp), 
			anchor(anchor), 
			align(align),
			type(type),
			anchorX(static_cast<float>(x)), 
			anchorY(static_cast<float>(y)) 
		{}
               
	} StringInfo;


	bool isBidiEnabled;
	std::vector<StringInfo> strs;
	int nlOffsetPx;

	bool spaceSizeExist;
	long spaceSize;
	long CalcSpaceSize();

	bool CanAddString(const UnicodeString & uniStr,
		int x, int y, const RenderParams & rp,
		TextAnchor anchor, TextAlign align, TextType type) const;

	bool AddStringInternal(const UnicodeString & str,
		int x, int y, const RenderParams & rp,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT,
		TextType type = TextType::TEXT);

	bool GenerateGeometry() override;

	AABB EstimateStringAABB(const UnicodeString & str, float x, float y, float scale) const;
	void CalcStringAABB(StringInfo & str, const UsedGlyphCache * gc) const;

	void CalcAnchoredPosition();
	void CalcLineAlign(const StringInfo & si, int lineId, float & x, float & y) const;

	UsedGlyphCache ExtractGlyphs(const UnicodeString & str);
};

#endif
