#ifndef BACKGROUND_SHADOW_SHADER_MANAGER_H
#define BACKGROUND_SHADOW_SHADER_MANAGER_H

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IShaderManager.h"

class BackgroundShadowShaderManager : public IShaderManager
{
public:
    BackgroundShadowShaderManager(Shadow shadow);
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

    void Clear() override;

    void PreRender() override;

    void Render(int quadsCount) override;

protected:
    GLint positionLocation;
    GLint colorLocation;
    GLint aabbLocation;

    GLint cornerRadiusUniform;
    GLint blurRadiusUniform;
    GLint shadowDirUniform;
    GLint shadowColorUniform;

    Shadow shadow;

    BackgroundSettings::Shape shape;
    float roundCornerRadius;

    float r, g, b, a;    

    float min_x;
    float min_y;
    float max_x;
    float max_y;

    std::vector<GLint> startingElements;
    std::vector<GLint> counts;

   
    void AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;
};



#endif
