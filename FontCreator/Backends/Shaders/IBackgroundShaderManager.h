#ifndef IBACKGROUND_SHADER_MANAGER_H
#define IBACKGROUND_SHADER_MANAGER_H

struct RenderSettings;

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IShaderManager.h"

class IBackgroundShaderManager : public IShaderManager
{
public:
    IBackgroundShaderManager(const RenderSettings& r);
    virtual ~IBackgroundShaderManager() = default;

    virtual void Clear() override;

    virtual int GetQuadVertices() const override;

    virtual void PreRender() override;
    virtual void Render(int quadsCount) override;
    virtual void RenderLayer(int layerId) override;

protected:

    BackgroundSettings::Shape shape;
    
    std::vector<std::vector<GLint>> startingElements;
    std::vector<std::vector<GLint>> counts;

    void AddGeometryInfo(int layerId);
};




#endif
