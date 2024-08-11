#ifndef DEFAULT_FONT_SHADER_MANAGER_H
#define DEFAULT_FONT_SHADER_MANAGER_H

#include <vector>

#include "../../Externalncludes.h"
#include "../../AbstractRenderer.h"

#include "./IShaderManager.h"

class DefaultFontShaderManager : public IShaderManager
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


#endif
