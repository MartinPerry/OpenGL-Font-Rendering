#ifndef GL_RENDERER_H
#define GL_RENDERER_H

class IFontShaderManager;

#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>

#include "./AbstractRenderer.h"

#include "./Externalncludes.h"


class GLRenderer
{
public:
	
	GLRenderer(const RenderSettings& r, int glVersion = 3);
	GLRenderer(const RenderSettings& r, int glVersion,
                     const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm);
   	
	virtual ~GLRenderer();

	void SetMainRenderer(AbstractRenderer* mainRenderer);

	const RenderSettings& GetSettings() const;

	void SetCanvasSize(int w, int h);
	void SwapCanvasWidthHeight();

	void SetFontTextureLinearFiler(bool val);
	
	std::shared_ptr<IFontShaderManager> GetShaderManager() const;

	void AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp);
	void FillTexture();
	void FillVB();
	
	void Render();
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

	AbstractRenderer* mainRenderer;

	RenderSettings rs;

    std::shared_ptr<IFontShaderManager> sm;
    	
	bool enabled;
	GLuint vbo;
	GLuint vao;
	GLuint fontTex;
	Shader shader;
	int glVersion;
	
	float psW; //1.0 / pixel size in width
	float psH; //1.0 / pixel size in height

	void InitGL();
	
	void CreateTexture();	
	void CreateVAO();

	GLuint CompileGLSLShader(GLenum target, const char* shader);
	GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader);

   
};

#endif
