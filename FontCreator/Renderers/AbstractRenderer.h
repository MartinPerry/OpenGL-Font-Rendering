#ifndef ABSTRACT_RENDERER_H
#define ABSTRACT_RENDERER_H

class FontBuilder;
class BackendBase;

#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>
#include <optional>

#include "../FontStructures.h"

#include "../Externalncludes.h"


class AbstractRenderer
{
public:
	enum class TextAlign { ALIGN_LEFT, ALIGN_CENTER };
	enum class TextAnchor { LEFT_TOP, CENTER, LEFT_DOWN };
	enum class TextType { TEXT, CAPTION_TEXT, CAPTION_SYMBOL };
	enum class AxisYOrigin { TOP, DOWN };

	struct Vertex
	{
		float x, y;
		float u, v;

		Vertex() : x(0), y(0), u(0), v(0) {};
		Vertex(float x, float y, float u, float v) : x(x), y(y), u(u), v(v) {};

	};

	
	struct RenderParams
	{
		Color color;
		float scale = 1.0f;			
		std::optional<Color> bgColor = std::nullopt;

		RenderParams() : color({ 0, 0, 0, 1 }), scale(1.0f) {};
		RenderParams(float scale) : color({ 0, 0, 0, 1 }), scale(scale) {};
		RenderParams(Color c, float scale) : color(c), scale(scale) {};
		RenderParams(Color c, Color bgColor, float scale) : color(c), scale(scale), bgColor(bgColor) {};
	};

	static const Color DEFAULT_COLOR;
	static const RenderParams DEFAULT_PARAMS;

	static std::vector<std::string> GetFontsInDirectory(const std::string& fontDir);

	AbstractRenderer(const FontBuilderSettings& fs, std::unique_ptr<BackendBase>&& backend);


	virtual ~AbstractRenderer();
	
	void SetVisibilityCheck(bool val) noexcept;

	BackendBase* GetBackend() const;

	std::shared_ptr<FontBuilder> GetFontBuilder();
	void SetCanvasSize(int w, int h);
		
	void SetAxisYOrigin(AxisYOrigin axisY);
	void SetCaption(const UnicodeString& mark);
	void SetCaption(const UnicodeString& mark, int offsetInPixels);
	void SetCaptionOffset(int offsetInPixels);
	void SetBackgroundSettings(std::optional<BackgroundSettings> bs);

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
		UnicodeString mark;
		int offset;

	};
	
	std::shared_ptr<FontBuilder> fb;
	std::unique_ptr<BackendBase> backend;	

	CaptionInfo ci;

	AxisYOrigin axisYOrigin;
			
	bool checkVisibility;
	bool strChanged;

#ifdef THREAD_SAFETY
	std::shared_timed_mutex m;
#endif
		
	AbstractRenderer(std::shared_ptr<FontBuilder> fb, std::unique_ptr<BackendBase>&& backend);

	virtual bool GenerateGeometry() = 0;

	void AddQuad(const GlyphInfo& gi, int x, int y, const RenderParams& rp);
	virtual void AddQuad(const GlyphInfo& gi, float x, float y, const RenderParams& rp);
	void OnFinishQuadGroup(const RenderParams& rp);
};

#endif
