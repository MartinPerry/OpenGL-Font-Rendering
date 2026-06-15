#include "./BackgroundTextureShaderManager.h"

#include "./Shaders.h"

#include "../../Externalncludes.h"

BackgroundTextureShaderManager::BackgroundTextureShaderManager() :
    BackgroundShaderManager(),
    r(1),
    g(1),
    b(1),
    a(1),
    bgTextureUniform(-1)
{
    std::vector<uint8_t> dummy = {255};
    emptyTextureId = this->InitTexture(dummy, 1, 1, 1);

}

BackgroundTextureShaderManager::~BackgroundTextureShaderManager()
{    
    for (auto& texId : this->usedTextures)
    {
        if (texId == emptyTextureId)
        {
            continue;
        }
        GL_CHECK(glDeleteTextures(1, &texId));
    }

    GL_CHECK(glDeleteTextures(1, &this->emptyTextureId));
}

const char* BackgroundTextureShaderManager::GetVertexShaderSource() const
{
    return TEXTURE_BACKGROUND_VERTEX_SHADER_SOURCE;
}

const char* BackgroundTextureShaderManager::GetPixelShaderSource() const
{
    return TEXTURE_BACKGROUND_PIXEL_SHADER_SOURCE;
}

void BackgroundTextureShaderManager::SetBaseColor(float r, float g, float b, float a)
{
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void BackgroundTextureShaderManager::GetAttributtesUniforms()
{
    BackgroundShaderManager::GetAttributtesUniforms();
    
    GL_CHECK(bgTextureUniform = glGetUniformLocation(shaderProgram, "bgTex"));
    
}

void BackgroundTextureShaderManager::Clear()
{
    BackgroundShaderManager::Clear();
    
    this->usedTextures.clear();
}

void BackgroundTextureShaderManager::Render(int quadsCount)
{
    auto type = (shape == BackgroundSettings::Shape::SQUARE) ? GL_TRIANGLES : GL_TRIANGLE_FAN;
    
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    
    GLuint lastTexId = std::numeric_limits<GLuint>::max();
    
    for (size_t i = 0; i < counts.size(); i++)
    {
        GLuint texId = this->usedTextures[i];
        if (texId != lastTexId)
        {
            FONT_BIND_TEXTURE_2D(texId);
        }
        lastTexId = texId;
        
        GL_CHECK(glDrawArrays(type, startingElements[i], counts[i]));
    }
    
}

void BackgroundTextureShaderManager::FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
    const AbstractRenderer::Vertex& maxVertex,
    const AbstractRenderer::RenderParams& rp,
    std::vector<float>& vec)
{
    if (rp.bgColor.has_value())
    {
        BackgroundShaderManager::FillQuadVertexData(minVertex, maxVertex, rp, vec);
    }
    else
    {
        AbstractRenderer::RenderParams tmp = rp;
        tmp.bgColor = Color(r, g, b, a);
        BackgroundShaderManager::FillQuadVertexData(minVertex, maxVertex, tmp, vec);
    }
    
    if (rp.textureName.has_value())
    {
        this->usedTextures.emplace_back(this->LoadTextureData(*rp.textureName));
    }
    else
    {
        this->usedTextures.emplace_back(emptyTextureId);
    }
}


GLuint BackgroundTextureShaderManager::LoadTextureData(const std::string& fileName)
{
    auto it = this->texturesNames.try_emplace(fileName);
    if (it.second == false)
    {
        return it.first->second;
    }
    
    std::vector<uint8_t> buffer;
    unsigned width, height;
    lodepng::State state; //optionally customize this one
    //state.decoder.color_convert = 0; //keep input data channels count

    size_t fileDataSize = 0;
    uint8_t* fileData = LoadDataFontFromFile(fileName, &fileDataSize);
    
    auto error = lodepng::decode(buffer, width, height, state, fileData, fileDataSize);

    SAFE_DELETE_ARRAY(fileData);

    int channelsCount = 4;
    if (width * height * 3 == buffer.size()) channelsCount = 3;
    else if (width * height == buffer.size()) channelsCount = 1;
    
    GLuint texId = this->InitTexture(buffer, width, height, channelsCount);

    it.first->second = texId;

    return texId;
}

GLuint BackgroundTextureShaderManager::InitTexture(const std::vector<uint8_t>& data, int w, int h, int channelsCount)
{
    GLuint texture;
    
    
    //create texture
    GL_CHECK(glGenTextures(1, &texture));
    FONT_BIND_TEXTURE_2D(texture);
    
    if (channelsCount == 1)
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, TEXTURE_SINGLE_CHANNEL,
            w, h, 0,
            TEXTURE_SINGLE_CHANNEL, GL_UNSIGNED_BYTE, data.data()));
    }
    else if (channelsCount == 3)
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            w, h, 0,
            GL_RGB, GL_UNSIGNED_BYTE, data.data()));
    }
    else if (channelsCount == 4)
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data.data()));
    }

    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    
    return texture;
}
