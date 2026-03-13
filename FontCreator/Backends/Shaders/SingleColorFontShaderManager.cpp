#include "./SingleColorFontShaderManager.h"

#include "./Shaders.h"

SingleColorFontShaderManager::SingleColorFontShaderManager(std::optional<SDF> sdf) :
	sdf(sdf),
	positionLocation(0),
	texCoordLocation(0),
	colorUniform(0),
	sdfEdgeLocation(0),
	sdfSoftnessLocation(0),
	sdfOutlineColorLocation(0),
	sdfOutlineWidthLocation(0),
	r(1.0f),
	g(1.0f),
	b(1.0f),
	a(1.0f)
{
}

const char* SingleColorFontShaderManager::GetVertexShaderSource() const
{
	return SINGLE_COLOR_VERTEX_SHADER_SOURCE;
}

const char* SingleColorFontShaderManager::GetPixelShaderSource() const
{
	return (sdf.has_value()) ? (
		sdf->outlineColor.has_value() ? SINGLE_COLOR_SDF_OUTLINE_PIXEL_SHADER_SOURCE : SINGLE_COLOR_SDF_PIXEL_SHADER_SOURCE
		) : SINGLE_COLOR_PIXEL_SHADER_SOURCE;	
}

void SingleColorFontShaderManager::SetColor(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void SingleColorFontShaderManager::GetAttributtesUniforms()
{
    GL_CHECK(colorUniform = glGetUniformLocation(shaderProgram, "fontColor"));

	GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
	GL_CHECK(texCoordLocation = glGetAttribLocation(shaderProgram, "TEXCOORD0"));	

	if (sdf.has_value())
	{
		// Typical setup values for FreeType-generated SDF:
		GL_CHECK(sdfEdgeLocation = glGetUniformLocation(shaderProgram, "uEdge"));
		GL_CHECK(sdfSoftnessLocation = glGetUniformLocation(shaderProgram, "uSoftness"));

		if (sdf->outlineColor.has_value())
		{
			GL_CHECK(sdfOutlineColorLocation = glGetUniformLocation(shaderProgram, "uOutlineColor"));
			GL_CHECK(sdfOutlineWidthLocation = glGetUniformLocation(shaderProgram, "uOutlineWidth"));
		}
	}
}

void SingleColorFontShaderManager::BindVertexAtribs()
{
	const GLsizei POSITION_SIZE = 2;
	const GLsizei TEXCOORD_SIZE = 2;
	
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

void SingleColorFontShaderManager::BindUniforms()
{
	GL_CHECK(glUniform4f(colorUniform, r, g, b, a));

	if (sdf.has_value())
	{
		glUniform1f(sdfEdgeLocation, sdf->edgeValue);
		glUniform1f(sdfSoftnessLocation, sdf->softness);

		if (sdf->outlineColor.has_value())
		{
			glUniform1f(sdfOutlineWidthLocation, sdf->outlineWidth);
			glUniform4f(sdfOutlineColorLocation, sdf->outlineColor->r, sdf->outlineColor->g,
				sdf->outlineColor->b, sdf->outlineColor->a);
		}
	}
}

void SingleColorFontShaderManager::PreRender()
{	
}

int SingleColorFontShaderManager::GetQuadVertices() const
{
	return 6;
}

void SingleColorFontShaderManager::FillQuadVertexData(
    const AbstractRenderer::Vertex & minVertex,
	const AbstractRenderer::Vertex & maxVertex,
	const AbstractRenderer::RenderParams & rp,
	std::vector<float> & vec)
{

	const float minX = 2.0f * minVertex.x - 1.0f;
	const float minY = -(2.0f * minVertex.y - 1.0f);

	const float maxX = 2.0f * maxVertex.x - 1.0f;
	const float maxY = -(2.0f * maxVertex.y - 1.0f);

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
	
	/*
	v[0] = a;
	v[1] = b;
	v[2] = d;

	v[3] = b;
	v[4] = c;
	v[5] = d;
	*/
}

