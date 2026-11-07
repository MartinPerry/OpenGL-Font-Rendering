#ifndef ISHADER_MANAGER_H
#define ISHADER_MANAGER_H

struct RenderSettings;

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"


class IShaderManager
{
public:
    IShaderManager(const RenderSettings& r);
    virtual ~IShaderManager() = default;

    void SetShaderProgram(GLuint program);
    virtual uint8_t GetTextureChannels() const;

    virtual const char* GetVertexShaderSource() const = 0;
    virtual const char* GetPixelShaderSource() const = 0;

    GLuint BuildFromSources(const char* vSource, const char* pSource);

    virtual void GetAttributtesUniforms() = 0;
    virtual void BindVertexAtribs() = 0;
    virtual void BindUniforms() = 0;
    
    virtual void SetCanvasSize(int w, int h);

    virtual int GetQuadVertices() const = 0;

    virtual void FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
        const AbstractRenderer::Vertex& maxVertex,
        const AbstractRenderer::RenderParams& rp,
        std::vector<float>& vec) = 0;

    virtual void Clear();

    virtual void PreRender() {	}

    virtual void Render(int quadsCount);
    virtual void RenderLayer(int layerId);

protected:
    GLuint shaderProgram;

    float canvasW;
    float canvasH;

    std::vector<int> quadsCount;

    GLuint CompileGLSLShader(GLenum target, const char* shader) const;
    GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader) const;
};




#endif
