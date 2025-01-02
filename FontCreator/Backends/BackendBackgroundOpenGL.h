#ifndef BACKGROUND_OPENGL_H
#define BACKGROUND_OPENGL_H

class IShaderManager;

#include "../Renderers/AbstractRenderer.h"

#include "./BackendOpenGL.h"

class BackendBackgroundOpenGL : public BackendOpenGL
{
public:
	BackendBackgroundOpenGL(const BackgroundSettings& bs, const RenderSettings& r, int glVersion = 3);
	BackendBackgroundOpenGL(const BackgroundSettings& bs, const RenderSettings& r, int glVersion,
		const char* vSource, const char* pSource, std::shared_ptr<IShaderManager> sm);
	virtual ~BackendBackgroundOpenGL();

	void SetBackgroundSettings(const BackgroundSettings& bs);

	void OnFinishQuadGroup(const AbstractRenderer::RenderParams& rp) override;
	
	virtual void Render(std::function<void(GLuint)> preDrawCallback, std::function<void()> postDrawCallback);
	
protected:
	
	BackgroundSettings bs;
	AABB curQuadAabb;
		
	void AddQuad(AbstractRenderer::Vertex& vmin, AbstractRenderer::Vertex& vmax, const AbstractRenderer::RenderParams& rp) override;

};

#endif