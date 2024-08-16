#ifndef BACKEND_OPENGL_H
#define BACKEND_OPENGL_H

class BackendBackgroundOpenGL;
class IShaderManager;

#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>

#include "../FontStructures.h"
#include "./BackendBase.h"

#include "../Externalncludes.h"


class BackendOpenGL : public BackendBase
{
public:
	
	BackendOpenGL(const RenderSettings& r, int glVersion = 3);
	BackendOpenGL(const RenderSettings& r, int glVersion,
                  const char * vSource, const char * pSource, std::shared_ptr<IShaderManager> sm);
   	
	virtual ~BackendOpenGL();

	void SetBackground() override;
	void SetMainRenderer(AbstractRenderer* mainRenderer) override;
	
	void SetFontTextureLinearFiler(bool val);
	
	std::shared_ptr<IShaderManager> GetShaderManager() const;
	
	void Clear() override;
	void OnFinishQuadGroup() override;

	void FillFontTexture() override;
	void FillGeometry() override;
	
	void Render() override;
    virtual void Render(std::function<void(GLuint)> preDrawCallback, std::function<void()> postDrawCallback);
	
	friend class AbstractRenderer;
	friend class StringRenderer;
	friend class NumberRenderer;
	
protected:
	 	
	struct Shader
	{
        GLuint program;

		const char * vSource;
		const char * pSource;
        bool isDefault;        
	};
	
    std::shared_ptr<IShaderManager> sm;
	
	std::unique_ptr<BackendBase> background;

	GLuint vbo;
	GLuint vao;
	GLuint texture;
	Shader shader;
	int glVersion;
	
	float tW; //1.0 / pixel size in width
	float tH; //1.0 / pixel size in height

	void InitGL();
	
	void InitTexture(const char* uniformName);
	void InitVAO();
	
	void OnCanvasSizeChanges() override;

	void AddQuad(AbstractRenderer::Vertex& vmin, AbstractRenderer::Vertex& vmax, const AbstractRenderer::RenderParams& rp) override;
};

#endif
