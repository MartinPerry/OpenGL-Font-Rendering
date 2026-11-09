#ifndef ABSTRACT_RENDERER_H
#define ABSTRACT_RENDERER_H

class IFontBuilder;
class BackendBase;

#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <shared_mutex>
#include <algorithm>
#include <optional>

#include "../FontStructures.h"

#include "../Externalncludes.h"


class AbstractRenderer
{
public:
	enum class TextAlign : uint8_t { ALIGN_LEFT, ALIGN_CENTER };
	enum class TextAnchor : uint8_t { LEFT_TOP, CENTER, LEFT_DOWN };
	enum class TextType : uint8_t { TEXT, CAPTION_TEXT, CAPTION_SYMBOL };
	enum class AxisYOrigin : uint8_t { TOP, DOWN };

	struct Vertex
	{
		float x, y;
		float u, v;

		Vertex() : x(0), y(0), u(0), v(0) {};
		Vertex(float x, float y, float u, float v) : x(x), y(y), u(u), v(v) {};

	};

	
	struct RenderParams
	{		
		mutable Color color;
		float scale = 1.0f;			
		int layerId = 0;
		std::optional<BackgroundInfo> bgInfo = std::nullopt;
        std::optional<std::string> textureName = std::nullopt;
		
		RenderParams() : color(0, 0, 0, 1), scale(1.0f) {};
		RenderParams(float scale) : color(0, 0, 0, 1), scale(scale) {};
		RenderParams(const Color& c, float scale) : color(c), scale(scale) {};
		RenderParams(const Color& c, const Color& bgColor, float scale) : color(c), scale(scale), bgInfo(bgColor) {};
	};
    

	static const Color DEFAULT_COLOR;
	static const RenderParams DEFAULT_PARAMS;

	static std::vector<std::string> GetFontsInDirectory(const std::string& fontDir);

	AbstractRenderer(const FontBuilderSettings& fs, std::unique_ptr<BackendBase>&& backend);


	virtual ~AbstractRenderer();
	
	void SetVisibilityCheck(bool val) noexcept;

	BackendBase* GetBackend() const;

	std::shared_ptr<IFontBuilder> GetFontBuilder();
	void SetCanvasSize(int w, int h);
		
	void SetExistenceCheck(bool val) noexcept;
	void SetAxisYOrigin(AxisYOrigin axisY);
	void SetCaption(const StringUtf8& mark);
	void SetCaption(const StringUtf8& mark, int offsetInPixels);
	void SetCaptionOffset(int offsetInPixels);
	void SetBackgroundSettings(std::optional<BackgroundSettings> bs);
	void SetExtraGlyphSpacingSize(int sizeInPixels);

	const RenderSettings& GetRenderSettings() const;
	int GetCanvasWidth() const;
	int GetCanvasHeight() const;
	int GetCaptionOffset() const;

	
	void SwapCanvasWidthHeight();
	
	void Clear();

	virtual void Render();
	
	friend class BackendBase;
	friend class BackendImage;
	friend class BackendOpenGL;

protected:

	struct CaptionInfo
	{
		StringUtf8 mark;
		int offset;
	};
	
	std::shared_ptr<IFontBuilder> fb;
	std::unique_ptr<BackendBase> backend;	

	CaptionInfo ci;

	bool checkIfExist;
	bool checkVisibility;
	bool strChanged;

	AxisYOrigin axisYOrigin;
	int extraGlyphSpacingSize;
				
	std::unordered_map<uint64_t, std::vector<size_t>> zonesGrid;
	int32_t zoneCellSize;

#ifdef THREAD_SAFETY
	std::shared_timed_mutex m;
#endif
		
	AbstractRenderer(std::shared_ptr<IFontBuilder> fb, std::unique_ptr<BackendBase>&& backend);

	uint64_t GetZoneCell(int32_t x, int32_t y) const;
	uint64_t GetCellKey(int32_t x, int32_t y) const;
	void AddToZone(int x, int y, size_t index);
	virtual void RecalcZonesGrid() = 0;

	void ClearGeom();
	virtual bool GenerateGeometry() = 0;

	void AddEmptyQuad(float x, float y, float w, float h, const RenderParams& rp);

	void AddQuad(const GlyphInfo& gi, int x, int y, const RenderParams& rp);
	virtual void AddQuad(const GlyphInfo& gi, float x, float y, const RenderParams& rp);
	void OnFinishQuadGroup(const RenderParams& rp);
};

#endif
