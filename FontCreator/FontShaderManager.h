#ifndef _FONT_SHADER_MANAGER_H_
#define _FONT_SHADER_MANAGER_H_

#include <vector>

#include "./Externalncludes.h"
#include "./AbstractRenderer.h"

class IFontShaderManager
{
public:
	IFontShaderManager() : shaderProgram(0) {}
    virtual ~IFontShaderManager() = default;
    
    void SetShaderProgram(GLuint program)
    {
        this->shaderProgram = program;
    };
    
    virtual void GetAttributtesUniforms() = 0;
    virtual void BindVertexAtribs() = 0;
    
    virtual void FillVertexData(const AbstractRenderer::Vertex & minVertex,
                                const AbstractRenderer::Vertex & maxVertex,
                                const AbstractRenderer::Color & color,
                                std::vector<float> & vec) = 0;

protected:
    GLuint shaderProgram;
};



class DefaultFontShaderManager : public IFontShaderManager
{
public:
    DefaultFontShaderManager();
    virtual ~DefaultFontShaderManager();
    
    void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
    
    void FillVertexData(const AbstractRenderer::Vertex & minVertex,
                        const AbstractRenderer::Vertex & maxVertex,
                        const AbstractRenderer::Color & color,
                        std::vector<float> & vec) override;
    
protected:
    GLuint positionLocation;
    GLuint texCoordLocation;
    GLuint colorLocation;
    
    
};

#endif /* _FONT_SHADER_MANAGER_H_ */
