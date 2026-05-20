#include "./SdfShaderSupport.h"


SdfShaderSupport::SdfShaderSupport(const SDF& sdf) :
    sdf(sdf),
    sdfEdgeLocation(0),
    sdfSoftnessLocation(0),
    sdfOutlineColorLocation(0),
    sdfOutlineWidthLocation(0)
{
}

const SDF& SdfShaderSupport::GetSettings() const
{
    return sdf;
}

void SdfShaderSupport::LoadUniforms(GLuint shaderProgram)
{
    // Typical setup values for FreeType-generated SDF:
    GL_CHECK(sdfEdgeLocation = glGetUniformLocation(shaderProgram, "uEdge"));
    GL_CHECK(sdfSoftnessLocation = glGetUniformLocation(shaderProgram, "uSoftness"));

    if (sdf.outlineColor.has_value())
    {
        GL_CHECK(sdfOutlineColorLocation = glGetUniformLocation(shaderProgram, "uOutlineColor"));
        GL_CHECK(sdfOutlineWidthLocation = glGetUniformLocation(shaderProgram, "uOutlineWidth"));
    }
}

void SdfShaderSupport::BindUniforms()
{
    glUniform1f(sdfEdgeLocation, sdf.edgeValue);
    glUniform1f(sdfSoftnessLocation, sdf.softness);

    if (sdf.outlineColor.has_value())
    {
        glUniform1f(sdfOutlineWidthLocation, sdf.outlineWidth);
        glUniform4f(sdfOutlineColorLocation, sdf.outlineColor->r, sdf.outlineColor->g,
            sdf.outlineColor->b, sdf.outlineColor->a);
    }
}