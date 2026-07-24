#ifndef BACKGROUND_SHADER_MANAGER_H
#define BACKGROUND_SHADER_MANAGER_H

struct RenderSettings;

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IBackgroundShaderManager.h"

class BackgroundShaderManager : public IBackgroundShaderManager
{
public:
    BackgroundShaderManager(const RenderSettings& r);
    virtual ~BackgroundShaderManager() = default;

    virtual const char* GetVertexShaderSource() const override;
    virtual const char* GetPixelShaderSource() const override;

    virtual void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
    void BindUniforms() override;
   
    void SetShape(BackgroundSettings::Shape shape, float radius = 0.0f);

    
    virtual void FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
        const AbstractRenderer::Vertex& maxVertex,
        const AbstractRenderer::RenderParams& rp,
        std::vector<float>& vec) override;
  
protected:
    GLint positionLocation;
    GLint colorLocation;
    GLint aabbLocation;
    
    float roundCornerRadius;
   
    float min_x;
    float min_y;
    float max_x;
    float max_y;
   

    void FillRoundCornersQuad(float cx, float cy, float dx, float dy, float rx, float ry, 
        const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;

    void FillCircle(float cx, float cy, float rx, float ry, 
        const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;

    void AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;
};



#endif
