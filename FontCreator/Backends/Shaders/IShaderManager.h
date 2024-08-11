#ifndef ISHADER_MANAGER_H
#define ISHADER_MANAGER_H

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"


class IShaderManager
{
public:
    IShaderManager() : shaderProgram(0) {}
    virtual ~IShaderManager() = default;

    void SetShaderProgram(GLuint program)
    {
        this->shaderProgram = program;
    };

    GLuint BuildFromSources(const char* vSource, const char* pSource);

    virtual void GetAttributtesUniforms() = 0;
    virtual void BindVertexAtribs() = 0;

    virtual void FillVertexData(const AbstractRenderer::Vertex& minVertex,
        const AbstractRenderer::Vertex& maxVertex,
        const AbstractRenderer::RenderParams& rp,
        std::vector<float>& vec) = 0;

    virtual void PreRender() {	}

protected:
    GLuint shaderProgram;

    GLuint CompileGLSLShader(GLenum target, const char* shader) const;
    GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader) const;
};




#endif
