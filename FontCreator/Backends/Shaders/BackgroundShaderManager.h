#ifndef BACKGROUND_SHADER_MANAGER_H
#define BACKGROUND_SHADER_MANAGER_H

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IShaderManager.h"

class BackgroundShaderManager : public IShaderManager
{
public:
    BackgroundShaderManager();
    virtual ~BackgroundShaderManager() = default;

    void GetAttributtesUniforms() override;
    void BindVertexAtribs() override;
   
    void SetCornerRadius(float radius);
    void SetShadowEnabled(bool shadow);

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

    void AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const;
};



#endif
