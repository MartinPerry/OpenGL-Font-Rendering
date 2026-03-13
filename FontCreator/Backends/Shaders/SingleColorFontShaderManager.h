#ifndef SINGLE_COLOR_FONT_SHADER_MANAGER_H
#define SINGLE_COLOR_FONT_SHADER_MANAGER_H

#include <vector>
#include <optional>

#include "../../Externalncludes.h"
#include "../../Renderers/AbstractRenderer.h"

#include "./IShaderManager.h"

class SingleColorFontShaderManager : public IShaderManager
{
public:
	SingleColorFontShaderManager(std::optional<SDF> sdf);
	virtual ~SingleColorFontShaderManager() = default;

	const char* GetVertexShaderSource() const override;
	const char* GetPixelShaderSource() const override;

	void GetAttributtesUniforms() override;
	void BindVertexAtribs() override;
	void BindUniforms() override;

	void SetColor(float r, float g, float b, float a);

	int GetQuadVertices() const override;

	void FillQuadVertexData(const AbstractRenderer::Vertex & minVertex,
						const AbstractRenderer::Vertex & maxVertex,
						const AbstractRenderer::RenderParams & rp,
						std::vector<float> & vec) override;

	void PreRender() override;

protected:
	std::optional<SDF> sdf;

	GLint positionLocation;
	GLint texCoordLocation;	
	GLint colorUniform;

	GLint sdfEdgeLocation;
	GLint sdfSoftnessLocation;

	GLint sdfOutlineColorLocation;
	GLint sdfOutlineWidthLocation;

	float r, g, b, a;
};



#endif
