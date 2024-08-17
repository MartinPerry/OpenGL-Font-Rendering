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

    void SetColor(float r, float g, float b, float a);
    void SetCornerRadius(float radius);

    int GetQuadVertices() const override;

    void FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
        const AbstractRenderer::Vertex& maxVertex,
        const AbstractRenderer::RenderParams& rp,
        std::vector<float>& vec) override;

    void PreRender() override;

    void Render(int quadsCount) override;

protected:
    GLint positionLocation;
    GLint colorUniform;

    float r, g, b, a;
    float roundCornerRadius;

    std::vector<GLint> startingElements;
    std::vector<GLint> counts;

    void FillRoundCornersQuad(float cx, float cy, float dx, float dy, float r, std::vector<float>& vec) const;
};



#endif
