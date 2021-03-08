#ifndef STRING_RENDERER_H
#define STRING_RENDERER_H

class IFontShaderManager;

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"

class StringRenderer : public AbstractRenderer
{
public:

	/// <summary>
	/// Single line info	
	/// </summary>
	struct LineInfo
	{
		uint32_t start; //offset of start char within text
		uint32_t len;   //line length
		AABB aabb;		//line AABB
		RenderParams renderParams;
		float maxNewLineOffset; //offset to next new line

		LineInfo(const uint32_t & start) noexcept :
			start(start),
			len(0),
			aabb(AABB()),
			renderParams(DEFAULT_PARAMS),
			maxNewLineOffset(0.0)
		{}

		LineInfo(const uint32_t & start,
			const RenderParams & rp) noexcept :
			start(start),
			len(0),
			aabb(AABB()),
			renderParams(rp),
			maxNewLineOffset(0.0)
		{}

	};

	/// <summary>
	/// Single string info
	/// </summary>
	struct StringInfo
	{
		UnicodeString str;
		int x;
		int y;

		TextAnchor anchor;
		TextAlign align;
		TextType type;

		float anchorX;
		float anchorY;

		std::vector<LineInfo> lines;
		AABB global;

		StringInfo(const UnicodeString & str, int x, int y,
			TextAnchor anchor,
			TextAlign align, TextType type) noexcept :
			str(str),
			x(x),
			y(y),
			anchor(anchor),
			align(align),
			type(type),
			anchorX(static_cast<float>(x)),
			anchorY(static_cast<float>(y))
		{}

		StringInfo(UnicodeString && str, int x, int y,
			TextAnchor anchor,
			TextAlign align, TextType type) noexcept :
			str(std::move(str)),
			x(x),
			y(y),
			anchor(anchor),
			align(align),
			type(type),
			anchorX(static_cast<float>(x)),
			anchorY(static_cast<float>(y))
		{}

	};
	
	static StringRenderer* CreateSingleColor(Color color, const FontBuilderSettings& fs, const RenderSettings & r, int glVersion = 3);
	
	StringRenderer(const FontBuilderSettings& fs, const RenderSettings& r, int glVersion = 3);	
    StringRenderer(const FontBuilderSettings& fs, const RenderSettings& r, int glVersion,
                   const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm);
	
	~StringRenderer();
		
	void Clear();

	size_t GetStringsCount() const;
	StringInfo & GetStringInfo(size_t index);
	StringInfo & GetLastStringInfo();


	void SetNewLineOffset(int offsetInPixels);

	void SetBidiEnabled(bool val);
	
	
	//=========================================================

	bool AddStringCaption(const char * str,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const UnicodeString & str,
		double x, double y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const char * str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const UnicodeString & str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	//=========================================================

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

	//=========================================================

protected:

	typedef std::vector<std::tuple<FontInfo::GlyphLutIterator, bool, FontInfo *>> UsedGlyphCache;
	
	
	std::vector<StringInfo> strs;

	bool isBidiEnabled;		
	bool spaceSizeExist;

	int nlOffsetPx;
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
	void CalcLineAlign(const StringInfo & si, const LineInfo & li, float & x, float & y) const;

	UsedGlyphCache ExtractGlyphs(const UnicodeString & str);
};

#endif
