#ifndef FONT_SHADER_MANAGER_H
#define FONT_SHADER_MANAGER_H

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
                                const AbstractRenderer::RenderParams & rp,
                                std::vector<float> & vec) = 0;

	virtual void PreRender() {	}

protected:
    GLuint shaderProgram;
};



class DefaultFontShaderManager : public IFontShaderManager
{
public:
    DefaultFontShaderManager();
	virtual ~DefaultFontShaderManager() = default;
    
    void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
    
    void FillVertexData(const AbstractRenderer::Vertex & minVertex,
                        const AbstractRenderer::Vertex & maxVertex,
                        const AbstractRenderer::RenderParams & rp,
                        std::vector<float> & vec) override;
    	
protected:
    GLint positionLocation;
    GLint texCoordLocation;
    GLint colorLocation;        
};

class SingleColorFontShaderManager : public IFontShaderManager
{
public:
	SingleColorFontShaderManager();
	virtual ~SingleColorFontShaderManager() = default;

	void GetAttributtesUniforms() override;
	void BindVertexAtribs() override;

	void SetColor(float r, float g, float b, float a);

	void FillVertexData(const AbstractRenderer::Vertex & minVertex,
						const AbstractRenderer::Vertex & maxVertex,
						const AbstractRenderer::RenderParams & rp,
						std::vector<float> & vec) override;

	void PreRender() override;

protected:
	GLint positionLocation;
	GLint texCoordLocation;	
	GLint colorUniform;

	float r, g, b, a;
};

#endif /* FONT_SHADER_MANAGER_H */
