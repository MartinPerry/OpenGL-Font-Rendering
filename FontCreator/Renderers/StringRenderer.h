#ifndef STRING_RENDERER_H
#define STRING_RENDERER_H

class BackendBase;

#include "./AbstractRenderer.h"

#include "../Externalncludes.h"

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
		float maxNewLineOffset; //offset to next new line

		LineInfo(const uint32_t& start) noexcept :
			start(start),
			len(0),
			aabb(AABB()),
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

		RenderParams renderParams;

		std::vector<LineInfo> lines;
		AABB global;

		StringInfo(const UnicodeString& str, int x, int y,
			TextAnchor anchor,
			TextAlign align, TextType type) noexcept :
			str(str),
			x(x),
			y(y),
			anchor(anchor),
			align(align),
			type(type),
			anchorX(static_cast<float>(x)),
			anchorY(static_cast<float>(y)),
			renderParams(DEFAULT_PARAMS)
		{}

		StringInfo(UnicodeString&& str, int x, int y,
			TextAnchor anchor,
			TextAlign align, TextType type,
			const RenderParams& rp) noexcept :
			str(std::move(str)),
			x(x),
			y(y),
			anchor(anchor),
			align(align),
			type(type),
			anchorX(static_cast<float>(x)),
			anchorY(static_cast<float>(y)),
			renderParams(rp)
		{}

	};
	
	static StringRenderer* CreateSingleColor(Color color, const FontBuilderSettings& fs, 
		std::unique_ptr<BackendBase>&& backend);
	
	StringRenderer(const FontBuilderSettings& fs, std::unique_ptr<BackendBase>&& backend);
	~StringRenderer();
		
	void Clear();

	size_t GetStringsCount() const noexcept;
	StringInfo* GetStringInfo(size_t index);
	StringInfo* GetLastStringInfo();


	void SetNewLineOffset(int offsetInPixels) noexcept;

	void SetBidiEnabled(bool val) noexcept;
	
    void SetStringDeadzone(int radiusPx) noexcept;
	
	//=========================================================

	bool AddStringCaption(const char * str,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const UnicodeString & str,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const char * str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const UnicodeString & str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddCaptionOnly(float x, float y, const RenderParams& rp = DEFAULT_PARAMS);
	bool AddCaptionOnly(int x, int y, const RenderParams& rp = DEFAULT_PARAMS);

	//=========================================================

	bool AddString(const char * str,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	bool AddString(const char * str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	bool AddString(const UnicodeString & str,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS,
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
	
	int deadzoneRadius2;

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
