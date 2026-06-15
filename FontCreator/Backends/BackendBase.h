#ifndef BACKEND_BASE_H
#define BACKEND_BASE_H


#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>
#include <optional>

#include "../Renderers/AbstractRenderer.h"


class BackendBase
{
public:
	

	BackendBase(const RenderSettings& r);
	
	virtual ~BackendBase();

	virtual void SetBackground(std::optional<BackgroundSettings> bs);
	virtual void SetMainRenderer(AbstractRenderer* mainRenderer);

	const RenderSettings& GetSettings() const;

	void SetEnabled(bool val);
	bool IsEnabled() const;
		
	void SetRenderSizeThreshold(float heightPx, bool keepBackground = false);

	virtual void Clear();	
	virtual void AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp);
	virtual void OnFinishQuadGroup(const AbstractRenderer::RenderParams& rp);

	virtual void FillGeometry() = 0;
	virtual void FillFontTexture() = 0;

	virtual void Render() = 0;
  
	friend class AbstractRenderer;
	friend class BackendOpenGL;

protected:
	 	
	int quadsCount;
	std::vector<float> geom;

	AbstractRenderer* mainRenderer;

	RenderSettings rs;

    	
	bool enabled;
	float heightPx;
	bool heightThresholdKeepBackground;
	
	
	float psW; //1.0 / pixel size in width
	float psH; //1.0 / pixel size in height

	virtual void AddEmptyQuad(float x, float y, float w, float h, const AbstractRenderer::RenderParams& rp);
	virtual void AddQuad(AbstractRenderer::Vertex& vmin, AbstractRenderer::Vertex& vmax, const AbstractRenderer::RenderParams& rp) = 0;

	//must be hidden, because we only want to call it from actual renderer
	//since we must redraw fonts
	void SetCanvasSize(int w, int h);
	void SwapCanvasWidthHeight();
	virtual void OnCanvasChanges() = 0;
	
   
};

#endif
