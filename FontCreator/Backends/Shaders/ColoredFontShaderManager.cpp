#include "./ColoredFontShaderManager.h"

#include "./Shaders.h"

ColoredFontShaderManager::ColoredFontShaderManager(bool enableTransparency) :
    enableTransparency(enableTransparency),
    positionLocation(0),
    texCoordLocation(0)    
{
}

const char* ColoredFontShaderManager::GetVertexShaderSource() const
{
    return (enableTransparency) ? 
        COLORED_TRANSPARENT_GLYPHS_VERTEX_SHADER_SOURCE :
        DEFAULT_VERTEX_SHADER_SOURCE;
}

const char* ColoredFontShaderManager::GetPixelShaderSource() const
{
    return (enableTransparency) ?
        COLORED_TRANSPARENT_GLYPHS_PIXEL_SHADER_SOURCE : 
        COLORED_GLYPHS_PIXEL_SHADER_SOURCE;
}

uint8_t ColoredFontShaderManager::GetTextureChannels() const
{
    return 4;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void ColoredFontShaderManager::GetAttributtesUniforms()
{
    GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
    GL_CHECK(texCoordLocation = glGetAttribLocation(shaderProgram, "TEXCOORD0"));    
}

void ColoredFontShaderManager::BindVertexAtribs()
{
    const GLsizei POSITION_SIZE = 2;
    const GLsizei TEXCOORD_SIZE = (enableTransparency) ? 3 : 2;    

    const GLsizei VERTEX_SIZE = (POSITION_SIZE + TEXCOORD_SIZE) * sizeof(float);
    const size_t POSITION_OFFSET = 0;
    const size_t TEX_COORD_OFFSET = POSITION_OFFSET + POSITION_SIZE * sizeof(float);    


    GL_CHECK(glEnableVertexAttribArray(positionLocation));
    GL_CHECK(glVertexAttribPointer(positionLocation, POSITION_SIZE,
        GL_FLOAT, GL_FALSE,
        VERTEX_SIZE, (void*)(POSITION_OFFSET)));

    GL_CHECK(glEnableVertexAttribArray(texCoordLocation));
    GL_CHECK(glVertexAttribPointer(texCoordLocation, TEXCOORD_SIZE,
        GL_FLOAT, GL_FALSE,
        VERTEX_SIZE, (void*)(TEX_COORD_OFFSET)));
   

}

void ColoredFontShaderManager::BindUniforms()
{   
}

int ColoredFontShaderManager::GetQuadVertices() const
{
    return 6;
}


void ColoredFontShaderManager::FillQuadVertexData(const AbstractRenderer::Vertex& minVertex,
    const AbstractRenderer::Vertex& maxVertex,
    const AbstractRenderer::RenderParams& rp,
    std::vector<float>& vec)
{

    float minX = 2.0f * minVertex.x - 1.0f;
    float minY = -(2.0f * minVertex.y - 1.0f);

    float maxX = 2.0f * maxVertex.x - 1.0f;
    float maxY = -(2.0f * maxVertex.y - 1.0f);

    if (this->enableTransparency)
    {
        vec.push_back(minX); vec.push_back(minY);
        vec.push_back(minVertex.u); vec.push_back(minVertex.v);
        vec.push_back(rp.color.a);

        vec.push_back(maxX); vec.push_back(minY);
        vec.push_back(maxVertex.u); vec.push_back(minVertex.v);
        vec.push_back(rp.color.a);

        vec.push_back(minX); vec.push_back(maxY);
        vec.push_back(minVertex.u); vec.push_back(maxVertex.v);
        vec.push_back(rp.color.a);

        //========================================================
        //========================================================

        vec.push_back(maxX); vec.push_back(minY);
        vec.push_back(maxVertex.u); vec.push_back(minVertex.v);
        vec.push_back(rp.color.a);

        vec.push_back(maxX); vec.push_back(maxY);
        vec.push_back(maxVertex.u); vec.push_back(maxVertex.v);
        vec.push_back(rp.color.a);

        vec.push_back(minX); vec.push_back(maxY);
        vec.push_back(minVertex.u); vec.push_back(maxVertex.v);
        vec.push_back(rp.color.a);
    }
    else 
    {
        vec.push_back(minX); vec.push_back(minY);
        vec.push_back(minVertex.u); vec.push_back(minVertex.v);
        
        vec.push_back(maxX); vec.push_back(minY);
        vec.push_back(maxVertex.u); vec.push_back(minVertex.v);        

        vec.push_back(minX); vec.push_back(maxY);
        vec.push_back(minVertex.u); vec.push_back(maxVertex.v);        

        //========================================================
        //========================================================

        vec.push_back(maxX); vec.push_back(minY);
        vec.push_back(maxVertex.u); vec.push_back(minVertex.v);        

        vec.push_back(maxX); vec.push_back(maxY);
        vec.push_back(maxVertex.u); vec.push_back(maxVertex.v);        

        vec.push_back(minX); vec.push_back(maxY);
        vec.push_back(minVertex.u); vec.push_back(maxVertex.v);        
    }

    /*
    v[0] = a;
    v[1] = b;
    v[2] = d;

    v[3] = b;
    v[4] = c;
    v[5] = d;
    */
}

