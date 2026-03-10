#ifndef DEFAULT_FONT_SHADER_MANAGER_H
#define DEFAULT_FONT_SHADER_MANAGER_H

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IShaderManager.h"

class DefaultFontShaderManager : public IShaderManager
{
public:
    DefaultFontShaderManager();
	virtual ~DefaultFontShaderManager() = default;
    
    const char* GetVertexShaderSource() const override;
    const char* GetPixelShaderSource() const override;

    void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
    
    int GetQuadVertices() const override;

    void FillQuadVertexData(const AbstractRenderer::Vertex & minVertex,
                        const AbstractRenderer::Vertex & maxVertex,
                        const AbstractRenderer::RenderParams & rp,
                        std::vector<float> & vec) override;
    	
protected:
    GLint positionLocation;
    GLint texCoordLocation;
    GLint colorLocation;        
};


#endif
