#include "./BackgroundShadowShaderManager.h"

#include "./Shaders.h"

BackgroundShadowShaderManager::BackgroundShadowShaderManager(Shadow shadow) :
	shadow(shadow),
	positionLocation(-1),
	colorLocation(-1),
	aabbLocation(-1),
	cornerRadiusUniform(-1),
	blurRadiusUniform(-1),
	shadowDirUniform(-1),
	shadowColorUniform(-1),
	shape(BackgroundSettings::Shape::SQUARE),
	roundCornerRadius(0.0f),
	r(1.0f),
	g(1.0f),
	b(1.0f),
	a(1.0f),
	min_x(0),
	min_y(0),
	max_x(0),
	max_y(0)
{
}


const char* BackgroundShadowShaderManager::GetVertexShaderSource() const
{
	return SHADOW_BACKGROUND_VERTEX_SHADER_SOURCE;
}

const char* BackgroundShadowShaderManager::GetPixelShaderSource() const
{
	return SHADOW_BACKGROUND_PIXEL_SHADER_SOURCE;
}

void BackgroundShadowShaderManager::SetShape(BackgroundSettings::Shape shape, float radius)
{
	this->shape = shape;
	this->roundCornerRadius = radius;
}

void BackgroundShadowShaderManager::SetColor(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void BackgroundShadowShaderManager::GetAttributtesUniforms()
{
	GL_CHECK(cornerRadiusUniform = glGetUniformLocation(shaderProgram, "cornerRadius"));
	GL_CHECK(blurRadiusUniform = glGetUniformLocation(shaderProgram, "blurRadius"));
	GL_CHECK(shadowDirUniform = glGetUniformLocation(shaderProgram, "shadowDir"));
	GL_CHECK(shadowColorUniform = glGetUniformLocation(shaderProgram, "shadowColor"));

	GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
	GL_CHECK(colorLocation = glGetAttribLocation(shaderProgram, "COLOR"));
	GL_CHECK(aabbLocation = glGetAttribLocation(shaderProgram, "AABB"));

}

void BackgroundShadowShaderManager::BindVertexAtribs()
{
	const GLsizei POSITION_SIZE = 2;
	const GLsizei COLOR_SIZE = 4;
	const GLsizei AABB_SIZE = 4;

	const GLsizei VERTEX_SIZE = (aabbLocation != -1) ?
		(POSITION_SIZE + COLOR_SIZE + AABB_SIZE) * sizeof(float) :
		(POSITION_SIZE + COLOR_SIZE) * sizeof(float);

	const size_t POSITION_OFFSET = 0;
	const size_t COLOR_OFFSET = POSITION_OFFSET + POSITION_SIZE * sizeof(float);

	GL_CHECK(glEnableVertexAttribArray(positionLocation));
	GL_CHECK(glVertexAttribPointer(positionLocation, POSITION_SIZE,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(POSITION_OFFSET)));

	GL_CHECK(glEnableVertexAttribArray(colorLocation));
	GL_CHECK(glVertexAttribPointer(colorLocation, COLOR_SIZE,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(COLOR_OFFSET)));

	if (aabbLocation != -1)
	{
		const size_t AABB_OFFSET = COLOR_OFFSET + COLOR_SIZE * sizeof(float);

		GL_CHECK(glEnableVertexAttribArray(aabbLocation));
		GL_CHECK(glVertexAttribPointer(aabbLocation, AABB_SIZE,
			GL_FLOAT, GL_FALSE,
			VERTEX_SIZE, (void*)(AABB_OFFSET)));
	}
}

void BackgroundShadowShaderManager::BindUniforms()
{
	float pxBoxWidth = ((max_x - min_x) * this->canvasW);

	if (this->shape == BackgroundSettings::Shape::SQUARE)
	{
		GL_CHECK(glUniform1f(cornerRadiusUniform, 0.0f));
	}
	else if (this->shape == BackgroundSettings::Shape::CIRCLE)
	{
		GL_CHECK(glUniform1f(cornerRadiusUniform, 0.25f));
	}
	else if (this->shape == BackgroundSettings::Shape::ROUNDED_CORNER_SQUARE)
	{
		float cornerRadius = this->roundCornerRadius / pxBoxWidth;
		GL_CHECK(glUniform1f(cornerRadiusUniform, cornerRadius));
	}

	float blurRadius = this->shadow.blurRadius / pxBoxWidth;
	GL_CHECK(glUniform1f(blurRadiusUniform, blurRadius));

	GL_CHECK(glUniform2f(shadowDirUniform, this->shadow.dirX, this->shadow.dirY));

	GL_CHECK(glUniform4f(shadowColorUniform, this->shadow.color.r, this->shadow.color.g,
		this->shadow.color.b, this->shadow.color.a));
}

void BackgroundShadowShaderManager::Clear()
{
	this->startingElements.clear();
	this->counts.clear();
}

void BackgroundShadowShaderManager::PreRender()
{

}

void BackgroundShadowShaderManager::Render(int quadsCount)
{
	auto type = GL_TRIANGLES;

#if (defined(__APPLE__) || defined(__ANDROID_API__))
	for (int i = 0; i < counts.size(); ++i)
	{
		GL_CHECK(glDrawArrays(type, startingElements[i], counts[i]));
	}
#else
	GL_CHECK(glMultiDrawArrays(type, startingElements.data(), counts.data(), (GLsizei)counts.size()));
#endif
}

int BackgroundShadowShaderManager::GetQuadVertices() const
{
	return 6;
}


void BackgroundShadowShaderManager::FillQuadVertexData(
	const AbstractRenderer::Vertex& minVertex,
	const AbstractRenderer::Vertex& maxVertex,
	const AbstractRenderer::RenderParams& rp,
	std::vector<float>& vec)
{	
	float shadowPadW = 40 / this->canvasW;
	float shadowPadH = 40 / this->canvasH;	

	//if (vec.size() > 0) return;
	const float minX = 2.0f * (minVertex.x - shadowPadW) - 1.0f;
	const float minY = -(2.0f * (minVertex.y - shadowPadH) - 1.0f);

	const float maxX = 2.0f * (maxVertex.x + shadowPadW) - 1.0f;
	const float maxY = -(2.0f * (maxVertex.y + shadowPadH) - 1.0f);

	min_x = minX;
	min_y = minY;
	max_x = maxX;
	max_y = maxY;

	this->AddVertex(minX, minY, rp, vec);
	this->AddVertex(maxX, minY, rp, vec);
	this->AddVertex(minX, maxY, rp, vec);

	//========================================================
	//========================================================

	this->AddVertex(maxX, minY, rp, vec);
	this->AddVertex(maxX, maxY, rp, vec);
	this->AddVertex(minX, maxY, rp, vec);
	
	/*
	v[0] = a;
	v[1] = b;
	v[2] = d;
	
	v[3] = b;
	v[4] = c;
	v[5] = d;
	*/


	counts.push_back(this->GetQuadVertices());
	if (startingElements.size() == 0)
	{
		startingElements.push_back(0);
	}
	else
	{
		startingElements.push_back(startingElements.back() + this->GetQuadVertices());
	}
}


void BackgroundShadowShaderManager::AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp,
	std::vector<float>& vec) const
{
	vec.push_back(x); vec.push_back(y);
	if (rp.bgColor)
	{
		vec.push_back(rp.bgColor->r); vec.push_back(rp.bgColor->g);
		vec.push_back(rp.bgColor->b); vec.push_back(rp.bgColor->a);
	}
	else 
	{
		vec.push_back(r); vec.push_back(g);
		vec.push_back(b); vec.push_back(a);
	}

	vec.push_back(min_x); vec.push_back(min_y);
	vec.push_back(max_x); vec.push_back(max_y);	
}