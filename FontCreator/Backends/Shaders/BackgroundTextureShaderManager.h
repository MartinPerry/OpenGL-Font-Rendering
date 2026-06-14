#ifndef BACKGROUND_TEXTURE_SHADER_MANAGER_H
#define BACKGROUND_TEXTURE_SHADER_MANAGER_H

#include <vector>
#include <unordered_map>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./BackgroundShaderManager.h"

class BackgroundTextureShaderManager : public BackgroundShaderManager
{
public:
    BackgroundTextureShaderManager();
    virtual ~BackgroundTextureShaderManager() = default;

    virtual const char* GetVertexShaderSource() const override;
    virtual const char* GetPixelShaderSource() const override;

    void SetBaseColor(float r, float g, float b, float a);
    
    void GetAttributtesUniforms() override;
   
    void FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
        const AbstractRenderer::Vertex& maxVertex,
        const AbstractRenderer::RenderParams& rp,
        std::vector<float>& vec) override;

    void Clear() override;
    void Render(int quadsCount) override;

protected:
    float r, g, b, a;
    GLint bgTextureUniform;

    GLuint emptyTextureId;
    
    std::unordered_map<std::string, int> texturesNames;
    std::vector<GLuint> usedTextures;
    
    GLuint LoadTextureData(const std::string& fileName);
    GLuint InitTexture(const std::vector<uint8_t>& data, int w, int h, int channelsCount);
};



#endif
