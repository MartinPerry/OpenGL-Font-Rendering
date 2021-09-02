#ifndef BACKEND_OPENGL_H
#define BACKEND_OPENGL_H

class IFontShaderManager;

#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>

#include "./BackendBase.h"

#include "./Externalncludes.h"


class BackendOpenGL : public BackendBase
{
public:
	
	BackendOpenGL(const RenderSettings& r, int glVersion = 3);
	BackendOpenGL(const RenderSettings& r, int glVersion,
                     const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm);
   	
	virtual ~BackendOpenGL();

	void SetMainRenderer(AbstractRenderer* mainRenderer);
	
	void SetFontTextureLinearFiler(bool val);
	
	std::shared_ptr<IFontShaderManager> GetShaderManager() const;

	void Clear() override;
	void AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp) override;
	
	void FillFontTexture() override;
	void FillGeometry() override;
	
	void Render() override;
    void Render(std::function<void(GLuint)> preDrawCallback, std::function<void()> postDrawCallback);
	
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
	
    std::shared_ptr<IFontShaderManager> sm;
    	
	GLuint vbo;
	GLuint vao;
	GLuint fontTex;
	Shader shader;
	int glVersion;
	
	void InitGL();
	
	void InitFontTexture();
	void InitVAO();

	GLuint CompileGLSLShader(GLenum target, const char* shader);
	GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader);

	void OnCanvasSizeChanges() override;
};

#endif
