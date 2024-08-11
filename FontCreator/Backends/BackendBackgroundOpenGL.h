#ifndef BACKGROUND_OPENGL_H
#define BACKGROUND_OPENGL_H

class IShaderManager;

#include "../Renderers/AbstractRenderer.h"

#include "./BackendOpenGL.h"

class BackendBackgroundOpenGL : public BackendOpenGL
{
public:
	BackendBackgroundOpenGL(const RenderSettings& r, int glVersion = 3);
	BackendBackgroundOpenGL(const RenderSettings& r, int glVersion,
		const char* vSource, const char* pSource, std::shared_ptr<IShaderManager> sm);
	virtual ~BackendBackgroundOpenGL();

	void SetMainRenderer(AbstractRenderer* mainRenderer) override;

	void AddQuad(const AbstractRenderer::Vertex& vmin, const AbstractRenderer::Vertex& vmax);

	virtual void Render(std::function<void(GLuint)> preDrawCallback, std::function<void()> postDrawCallback);

protected:
	
	
	
	
};

#endif