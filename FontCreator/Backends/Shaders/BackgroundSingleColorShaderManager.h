#ifndef BACKGROUND_SINGLE_COLOR_SHADER_MANAGER_H
#define BACKGROUND_SINGLE_COLOR_SHADER_MANAGER_H

struct RenderSettings;

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IBackgroundShaderManager.h"

class BackgroundSingleColorShaderManager : public IBackgroundShaderManager
{
public:
    BackgroundSingleColorShaderManager(const RenderSettings& r);
    virtual ~BackgroundSingleColorShaderManager() = default;

    virtual const char* GetVertexShaderSource() const override;
    virtual const char* GetPixelShaderSource() const override;

    void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
    void BindUniforms() override;
   
    void SetColor(float r, float g, float b, float a);
    void SetShape(BackgroundSettings::Shape shape, float radius = 0.0);

    
    void FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
        const AbstractRenderer::Vertex& maxVertex,
        const AbstractRenderer::RenderParams& rp,
        std::vector<float>& vec) override;
    

protected:
    GLint positionLocation;
    GLint colorUniform;

    float r, g, b, a;

    float roundCornerRadius;
  
    void FillRoundCornersQuad(float cx, float cy, float dx, float dy, float rx, float ry, std::vector<float>& vec) const;
    void FillCircle(float cx, float cy, float rx, float ry, std::vector<float>& vec) const;

    void AddVertex(float x, float y, std::vector<float>& vec) const;
};



#endif
