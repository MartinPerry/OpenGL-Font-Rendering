#include "./SingleColorBackgroundShaderManager.h"


SingleColorBackgroundShaderManager::SingleColorBackgroundShaderManager() :
	positionLocation(0),	
	colorUniform(0),
	r(1.0f),
	g(1.0f),
	b(1.0f),
	a(1.0f),
	roundCornerRadius(0)
{
}

void SingleColorBackgroundShaderManager::SetColor(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

void SingleColorBackgroundShaderManager::SetCornerRadius(float radius)
{	
	this->roundCornerRadius = radius;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void SingleColorBackgroundShaderManager::GetAttributtesUniforms()
{
	GL_CHECK(colorUniform = glGetUniformLocation(shaderProgram, "bgColor"));

	GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
	//GL_CHECK(texCoordLocation = glGetAttribLocation(shaderProgram, "TEXCOORD0"));
}

void SingleColorBackgroundShaderManager::BindVertexAtribs()
{
	const GLsizei POSITION_SIZE = 2;
	const GLsizei TEXCOORD_SIZE = 2;

	//const GLsizei VERTEX_SIZE = (POSITION_SIZE + TEXCOORD_SIZE) * sizeof(float);
	const GLsizei VERTEX_SIZE = (POSITION_SIZE) * sizeof(float);
	const size_t POSITION_OFFSET = 0;	

	GL_CHECK(glEnableVertexAttribArray(positionLocation));
	GL_CHECK(glVertexAttribPointer(positionLocation, POSITION_SIZE,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(POSITION_OFFSET)));

	/*
	const size_t TEX_COORD_OFFSET = POSITION_OFFSET + POSITION_SIZE * sizeof(float);

	GL_CHECK(glEnableVertexAttribArray(texCoordLocation));
	GL_CHECK(glVertexAttribPointer(texCoordLocation, TEXCOORD_SIZE,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(TEX_COORD_OFFSET)));
	*/
}

void SingleColorBackgroundShaderManager::Clear()
{
	this->startingElements.clear();
	this->counts.clear();
}

void SingleColorBackgroundShaderManager::PreRender()
{
	GL_CHECK(glUniform4f(colorUniform, r, g, b, a));
}

void SingleColorBackgroundShaderManager::Render(int quadsCount)
{
	auto type = (roundCornerRadius == 0) ? GL_TRIANGLES : GL_TRIANGLE_FAN;

	//GL_CHECK(glDrawArrays(type, 0, quadsCount * this->GetQuadVertices()));	
	//return;


#if (defined(__APPLE__) || defined(__ANDROID_API__))
	for (int i = 0; i < counts.size(); ++i)
	{
		GL_CHECK(glDrawArrays(type, startingElements[i], counts[i]));
}
#else
	GL_CHECK(glMultiDrawArrays(type, startingElements.data(), counts.data(), counts.size()));
#endif
}

int SingleColorBackgroundShaderManager::GetQuadVertices() const
{
	return (roundCornerRadius == 0) ? 6 : 38;
}

void SingleColorBackgroundShaderManager::FillQuadVertexData(
	const AbstractRenderer::Vertex& minVertex,
	const AbstractRenderer::Vertex& maxVertex,
	const AbstractRenderer::RenderParams& rp,
	std::vector<float>& vec)
{
	//if (vec.size() > 0) return;
	const float minX = 2.0f * minVertex.x - 1.0f;
	const float minY = -(2.0f * minVertex.y - 1.0f);

	const float maxX = 2.0f * maxVertex.x - 1.0f;
	const float maxY = -(2.0f * maxVertex.y - 1.0f);

	if (roundCornerRadius == 0)
	{
		vec.push_back(minX); vec.push_back(minY);

		vec.push_back(maxX); vec.push_back(minY);

		vec.push_back(minX); vec.push_back(maxY);

		//========================================================
		//========================================================

		vec.push_back(maxX); vec.push_back(minY);

		vec.push_back(maxX); vec.push_back(maxY);

		vec.push_back(minX); vec.push_back(maxY);

		/*
		v[0] = a;
		v[1] = b;
		v[2] = d;

		v[3] = b;
		v[4] = c;
		v[5] = d;
		*/


	}
	else
	{
		//https://stackoverflow.com/questions/74960029/how-to-draw-a-rectangle-in-opengl-with-rounded-corners

		float r = this->roundCornerRadius;

		float cx = minX + 0.5f * (maxX - minX);
		float cy = minY + 0.5f * (maxY - minY);
		float dx = std::abs(maxX - minX) - 2 * r;
		float dy = std::abs(maxY - minY) - 2 * r;
		
		//fix overlap of triangles if rounding was too fast
		//some experimental value
		dx = std::max(-0.05f * r, dx);
		dy = std::max(-0.05f * r, dy);

		this->FillRoundCornersQuad(cx, cy, dx, dy, r, vec);
	}

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

void SingleColorBackgroundShaderManager::FillRoundCornersQuad(float cx, float cy, float dx, float dy, float r, std::vector<float>& vec) const
{

	static const float sina[45] = { 0.0f, 0.1736482f, 0.3420201f, 0.5f, 0.6427876f, 0.7660444f, 0.8660254f,
		0.9396926f, 0.9848077f, 1.0f, 0.9848078f, 0.9396927f, 0.8660255f, 0.7660446f, 0.6427878f, 0.5000002f,
		0.3420205f, 0.1736485f, 3.894144E-07f, -0.1736478f, -0.3420197f, -0.4999996f, -0.6427872f, -0.7660443f,
		-0.8660252f, -0.9396925f, -0.9848077f, -1.0f, -0.9848078f, -0.9396928f, -0.8660257f, -0.7660449f, 
		-0.6427881f, -0.5000006f, -0.3420208f, -0.1736489f, 0.0f, 0.1736482f, 0.3420201f, 0.5f, 0.6427876f,
		0.7660444f, 0.8660254f, 0.9396926f, 0.9848077f };
	static const float* cosa = sina + 9;
	
	
	int i;
	float x, y;
	
	
	vec.push_back(cx); vec.push_back(cy);
	
	float x0 = cx + (0.5f * dx);
	float y0 = cy + (0.5f * dy);
	for (i = 0; i < 9; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		vec.push_back(x); vec.push_back(y);
	}
	x0 -= dx;
	for (; i < 18; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		vec.push_back(x); vec.push_back(y);
	}
	y0 -= dy;
	for (; i < 27; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		vec.push_back(x); vec.push_back(y);
	}
	x0 += dx;
	for (; i < 36; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		vec.push_back(x); vec.push_back(y);
	}
	vec.push_back(x); vec.push_back(cy + (0.5f * dy));
	

}
