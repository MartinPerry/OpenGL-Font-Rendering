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
		std::optional<RenderParams> renderParams;

		LineInfo(const uint32_t& start) noexcept :
			start(start),
			len(0),
			aabb(AABB()),						
			maxNewLineOffset(0.0f),
			renderParams(std::nullopt)
		{}

		LineInfo(const uint32_t& start,
			const RenderParams& rp) noexcept :
			start(start),
			len(0),
			aabb(AABB()),			
			maxNewLineOffset(0.0f),
			renderParams(rp)
		{}

	};

	/// <summary>
	/// Single string info
	/// </summary>
	struct StringInfo
	{
		StringUtf8 str;
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

		StringInfo(const StringUtf8& str, int x, int y,
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

		StringInfo(StringUtf8&& str, int x, int y,
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
	
	float GetMaxLineHeight() const;

	//=========================================================

	bool AddStringCaption(const char * str,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const StringUtf8& str,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const char * str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS);

	bool AddStringCaption(const StringUtf8& str,
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

	bool AddString(const StringUtf8& str,
		float x, float y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	bool AddString(const StringUtf8& str,
		int x, int y, const RenderParams & rp = DEFAULT_PARAMS,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

	//=========================================================

protected:

	typedef std::vector<std::tuple<FontInfo::GlyphIterator, bool, FontInfo *>> UsedGlyphCache;
	
	
	std::vector<StringInfo> strs;

	bool isBidiEnabled;		
	bool spaceSizeExist;
	
	int deadzoneRadius2;

	int nlOffsetPx;
	long spaceSize;

	long CalcSpaceSize();

	bool CanAddString(const StringUtf8& uniStr,
		int x, int y, const RenderParams & rp,
		TextAnchor anchor, TextAlign align, TextType type) const;

	bool DeadzoneCheck(int x, int y) const;

	bool AddStringInternal(const StringUtf8& str,
		int x, int y, const RenderParams & rp,
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT,
		TextType type = TextType::TEXT);

	bool GenerateGeometry() override;

	AABB EstimateStringAABB(const StringUtf8& str, float x, float y, float scale) const;
	void CalcStringAABB(StringInfo & str, const UsedGlyphCache * gc) const;

	void CalcAnchoredPosition();
	void CalcAnchoredPosition(StringInfo& si, float& captionMarkHeight);
	void CalcLineAlign(const StringInfo & si, const LineInfo & li, float & x, float & y) const;

	UsedGlyphCache ExtractGlyphs(const StringUtf8& str);
};

#endif
