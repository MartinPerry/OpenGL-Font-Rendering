#ifndef SDF_SHADER_SUPPORT_H
#define SDF_SHADER_SUPPORT_H

#include <optional>

#include "../../Externalncludes.h"

#include "../../FontStructures.h"

class SdfShaderSupport 
{
public:
    SdfShaderSupport(const SDF& sdf);
    virtual ~SdfShaderSupport() = default;

    const SDF& GetSettings() const;

    void LoadUniforms(GLuint shaderProgram);
    void BindUniforms();

protected:
    SDF sdf;

    GLint sdfEdgeLocation;
    GLint sdfSoftnessLocation;

    GLint sdfOutlineColorLocation;
    GLint sdfOutlineWidthLocation;
};

#endif
