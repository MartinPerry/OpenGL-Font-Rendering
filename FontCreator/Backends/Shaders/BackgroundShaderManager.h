#ifndef BACKGROUND_SHADER_MANAGER_H
#define BACKGROUND_SHADER_MANAGER_H

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IShaderManager.h"

class BackgroundShaderManager : public IShaderManager
{
public:
    BackgroundShaderManager(bool shadow);
    virtual ~BackgroundShaderManager() = default;

    const char* GetVertexShaderSource() const override;
    const char* GetPixelShaderSource() const override;

    void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
    void BindUniforms() override;
   
    void SetShape(BackgroundSettings::Shape shape, float radius = 0.0);

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

    GLint arWhUniform;
        
    BackgroundSettings::Shape shape;
    float roundCornerRadius;
    bool shadow;
   
    float min_x;
    float min_y;
    float max_x;
    float max_y;

    std::vector<GLint> startingElements;
    std::vector<GLint> counts;

    void FillRoundCornersQuad(float cx, float cy, float dx, float dy, float r, 
        const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;

    void FillCircle(float cx, float cy, float r, 
        const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;

    void AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;
};



#endif
