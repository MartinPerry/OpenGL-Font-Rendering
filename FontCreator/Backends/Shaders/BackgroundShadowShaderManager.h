#ifndef BACKGROUND_SHADOW_SHADER_MANAGER_H
#define BACKGROUND_SHADOW_SHADER_MANAGER_H

struct RenderSettings;

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IBackgroundShaderManager.h"

class BackgroundShadowShaderManager : public IBackgroundShaderManager
{
public:
    BackgroundShadowShaderManager(const RenderSettings& r, Shadow shadow);
    virtual ~BackgroundShadowShaderManager() = default;

    virtual const char* GetVertexShaderSource() const override;
    virtual const char* GetPixelShaderSource() const override;

    void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
    void BindUniforms() override;
    
    void SetShape(BackgroundSettings::Shape shape, float radius = 0.0);
    void SetColor(float r, float g, float b, float a);

    int GetQuadVertices() const override;

    void FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
        const AbstractRenderer::Vertex& maxVertex,
        const AbstractRenderer::RenderParams& rp,
        std::vector<float>& vec) override;

      
protected:
    GLint positionLocation;
    GLint colorLocation;
    GLint aabbLocation;

    GLint cornerRadiusUniform;
    GLint blurRadiusUniform;
    GLint shadowDirUniform;
    GLint shadowPaddingSizeUniform;
    GLint shadowColorUniform;

    Shadow shadow;
   
    BackgroundSettings::Shape shape;
    float roundCornerRadius;
    
    float r, g, b, a;    

    float min_x;
    float min_y;
    float max_x;
    float max_y;

      
    void AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;
};



#endif
