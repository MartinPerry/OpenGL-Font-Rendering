#ifndef BACKEND_BASE_H
#define BACKEND_BASE_H

#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>

#include "./AbstractRenderer.h"


class BackendBase
{
public:
	
	BackendBase(const RenderSettings& r);
	
	virtual ~BackendBase();

	void SetMainRenderer(AbstractRenderer* mainRenderer);

	const RenderSettings& GetSettings() const;

		
	virtual void Clear() = 0;
	virtual void AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp) = 0;

	virtual void FillGeometry() = 0;
	virtual void FillFontTexture() = 0;

	virtual void Render() = 0;
  
	friend class AbstractRenderer;

protected:
	 		
	AbstractRenderer* mainRenderer;

	RenderSettings rs;

    	
	bool enabled;
	
	
	float psW; //1.0 / pixel size in width
	float psH; //1.0 / pixel size in height

	//must be hidden, because we only want to call it from actual renderer
	//since we must redraw fonts
	void SetCanvasSize(int w, int h);
	void SwapCanvasWidthHeight();

	virtual void CreateTexture() = 0;	
   
};

#endif