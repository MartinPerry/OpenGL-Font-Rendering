#include "./BackgroundShaderManager.h"


BackgroundShaderManager::BackgroundShaderManager() :
	positionLocation(0),	
	colorLocation(0),
	roundCornerRadius(0)
{
}


void BackgroundShaderManager::SetCornerRadius(float radius)
{	
	this->roundCornerRadius = radius;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void BackgroundShaderManager::GetAttributtesUniforms()
{	
	GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
	GL_CHECK(colorLocation = glGetAttribLocation(shaderProgram, "COLOR"));
}

void BackgroundShaderManager::BindVertexAtribs()
{
	const GLsizei POSITION_SIZE = 2;
	const GLsizei COLOR_SIZE = 4;
	
	const GLsizei VERTEX_SIZE = (POSITION_SIZE + COLOR_SIZE) * sizeof(float);
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
}

void BackgroundShaderManager::PreRender()
{
	
}

void BackgroundShaderManager::Render(int quadsCount)
{
	auto type = (roundCornerRadius == 0) ? GL_TRIANGLES : GL_TRIANGLE_FAN;

	//GL_CHECK(glDrawArrays(type, 0, quadsCount * this->GetQuadVertices()));	
	//return;

#if defined(__APPLE__) || defined(__ANDROID_API__)
	for (int i = 0; i < quadsCount; ++i)
	{
		GL_CHECK(glDrawArrays(type, startingElements[i], counts[i]));
	}
#else
	GL_CHECK(glMultiDrawArrays(type, startingElements.data(), counts.data(), quadsCount));	
#endif
}

int BackgroundShaderManager::GetQuadVertices() const
{
	return (roundCornerRadius == 0) ? 6 : 38;
}

void BackgroundShaderManager::FillQuadVertexData(
	const AbstractRenderer::Vertex& minVertex,
	const AbstractRenderer::Vertex& maxVertex,
	const AbstractRenderer::RenderParams& rp,
	std::vector<float>& vec)
{
	if (rp.bgColor.has_value() == false)
	{
		return;
	}

	//if (vec.size() > 0) return;
	const float minX = 2.0f * minVertex.x - 1.0f;
	const float minY = -(2.0f * minVertex.y - 1.0f);

	const float maxX = 2.0f * maxVertex.x - 1.0f;
	const float maxY = -(2.0f * maxVertex.y - 1.0f);

	if (roundCornerRadius == 0)
	{
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

		this->FillRoundCornersQuad(cx, cy, dx, dy, r, rp, vec);
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

void BackgroundShaderManager::FillRoundCornersQuad(float cx, float cy, float dx, float dy, float r, 
	const AbstractRenderer::RenderParams& rp,
	std::vector<float>& vec) const
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
	
	
	this->AddVertex(cx, cy, rp, vec);

	float x0 = cx + (0.5f * dx);
	float y0 = cy + (0.5f * dy);
	for (i = 0; i < 9; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	x0 -= dx;
	for (; i < 18; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	y0 -= dy;
	for (; i < 27; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	x0 += dx;
	for (; i < 36; i++)
	{
		x = x0 + (r * cosa[i]);
		y = y0 + (r * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	this->AddVertex(x, cy + (0.5f * dy), rp, vec);	
	

}

void BackgroundShaderManager::AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp, 
	std::vector<float>& vec) const
{
	vec.push_back(x); vec.push_back(y);	
	vec.push_back(rp.bgColor->r); vec.push_back(rp.bgColor->g);
	vec.push_back(rp.bgColor->b); vec.push_back(rp.bgColor->a);
}