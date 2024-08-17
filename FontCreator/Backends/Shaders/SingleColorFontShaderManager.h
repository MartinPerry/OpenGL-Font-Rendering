#ifndef SINGLE_COLOR_FONT_SHADER_MANAGER_H
#define SINGLE_COLOR_FONT_SHADER_MANAGER_H

#include <vector>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IShaderManager.h"

class SingleColorFontShaderManager : public IShaderManager
{
public:
	SingleColorFontShaderManager();
	virtual ~SingleColorFontShaderManager() = default;

	void GetAttributtesUniforms() override;
	void BindVertexAtribs() override;

	void SetColor(float r, float g, float b, float a);

	int GetQuadVertices() const override;

	void FillQuadVertexData(const AbstractRenderer::Vertex & minVertex,
						const AbstractRenderer::Vertex & maxVertex,
						const AbstractRenderer::RenderParams & rp,
						std::vector<float> & vec) override;

	void PreRender() override;

protected:
	GLint positionLocation;
	GLint texCoordLocation;	
	GLint colorUniform;

	float r, g, b, a;
};



#endif
