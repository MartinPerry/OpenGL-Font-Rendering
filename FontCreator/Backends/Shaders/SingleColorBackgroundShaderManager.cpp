#include "./SingleColorBackgroundShaderManager.h"

#include "./Shaders.h"

SingleColorBackgroundShaderManager::SingleColorBackgroundShaderManager() :
	positionLocation(-1),	
	colorUniform(-1),
	r(1.0f),
	g(1.0f),
	b(1.0f),
	a(1.0f),
	shape(BackgroundSettings::Shape::SQUARE),
	roundCornerRadius(0)
{
}

const char* SingleColorBackgroundShaderManager::GetVertexShaderSource() const
{
	return SINGLE_COLOR_BACKGROUND_VERTEX_SHADER_SOURCE;
}

const char* SingleColorBackgroundShaderManager::GetPixelShaderSource() const
{
	return SINGLE_COLOR_BACKGROUND_PIXEL_SHADER_SOURCE;
}

void SingleColorBackgroundShaderManager::SetColor(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

void SingleColorBackgroundShaderManager::SetShape(BackgroundSettings::Shape shape, float radius)
{	
	this->shape = shape;
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

void SingleColorBackgroundShaderManager::BindUniforms()
{
	GL_CHECK(glUniform4f(colorUniform, r, g, b, a));	
}


void SingleColorBackgroundShaderManager::Clear()
{
	this->startingElements.clear();
	this->counts.clear();
}

void SingleColorBackgroundShaderManager::PreRender()
{	
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
	GL_CHECK(glMultiDrawArrays(type, startingElements.data(), counts.data(), (GLsizei)counts.size()));
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

	if (shape == BackgroundSettings::Shape::SQUARE)
	{
		this->AddVertex(minX, minY, vec);
		this->AddVertex(maxX, minY, vec);
		this->AddVertex(minX, maxY, vec);

		//========================================================
		//========================================================

		this->AddVertex(maxX, minY, vec);
		this->AddVertex(maxX, maxY, vec);
		this->AddVertex(minX, maxY, vec);

		/*
		v[0] = a;
		v[1] = b;
		v[2] = d;

		v[3] = b;
		v[4] = c;
		v[5] = d;
		*/
	}
	else if (shape == BackgroundSettings::Shape::ROUNDED_CORNER_SQUARE)
	{
		//https://stackoverflow.com/questions/74960029/how-to-draw-a-rectangle-in-opengl-with-rounded-corners

		float rx = this->roundCornerRadius * (1.0f / this->canvasW);
		float ry = this->roundCornerRadius * (1.0f / this->canvasH);

		float cx = minX + 0.5f * (maxX - minX);
		float cy = minY + 0.5f * (maxY - minY);
		float dx = std::abs(maxX - minX) - 2 * rx;
		float dy = std::abs(maxY - minY) - 2 * ry;

		//fix overlap of triangles if rounding was too fast
		//some experimental value
		//dx = std::max(-0.05f * rx, dx);
		//dy = std::max(-0.05f * ry, dy);

		this->FillRoundCornersQuad(cx, cy, dx, dy, rx, ry, vec);
	}
	else if (shape == BackgroundSettings::Shape::CIRCLE)
	{
		//2 * - projection space is [-1, 1] and we calculate for 0, 1
		float rx = this->roundCornerRadius * 2.0f * (1.0f / this->canvasW);
		float ry = this->roundCornerRadius * 2.0f * (1.0f / this->canvasH);

		float cx = minX + 0.5f * (maxX - minX);
		float cy = minY + 0.5f * (maxY - minY);

		this->FillCircle(cx, cy, rx, ry, vec);
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

void SingleColorBackgroundShaderManager::FillRoundCornersQuad(float cx, float cy, float dx, float dy, float rx, float ry, std::vector<float>& vec) const
{
	/*
	r = std::min(r, 0.5f * std::min(dx, dy));

	constexpr int SEG = 8;
	constexpr float PI = 3.14159265358979323846f;

	const float ar = (this->canvasW / this->canvasH);

	AddVertex(cx, ar * cy, vec); // triangle fan center

	auto addArc = [&](float ox, float oy, float a0, float a1)
		{
			for (int i = 0; i <= SEG; ++i)
			{
				float t = float(i) / float(SEG);
				float a = a0 + (a1 - a0) * t;

				AddVertex(
					ox + std::cos(a) * r,
					ar * (oy + std::sin(a) * r),
					vec
				);
			}
		};

	float left = cx - dx * 0.5f;
	float right = cx + dx * 0.5f;
	float bottom = cy - dy * 0.5f;
	float top = cy + dy * 0.5f;

	addArc(right - r, top - r, 0.0f, PI * 0.5f);
	addArc(left + r, top - r, PI * 0.5f, PI);
	addArc(left + r, bottom + r, PI, PI * 1.5f);
	addArc(right - r, bottom + r, PI * 1.5f, PI * 2.0f);

	// close triangle fan
	AddVertex(right, (top - r), vec);
	return;
	*/
	static const float sina[45] = { 0.0f, 0.1736482f, 0.3420201f, 0.5f, 0.6427876f, 0.7660444f, 0.8660254f,
		0.9396926f, 0.9848077f, 1.0f, 0.9848078f, 0.9396927f, 0.8660255f, 0.7660446f, 0.6427878f, 0.5000002f,
		0.3420205f, 0.1736485f, 3.894144E-07f, -0.1736478f, -0.3420197f, -0.4999996f, -0.6427872f, -0.7660443f,
		-0.8660252f, -0.9396925f, -0.9848077f, -1.0f, -0.9848078f, -0.9396928f, -0.8660257f, -0.7660449f,
		-0.6427881f, -0.5000006f, -0.3420208f, -0.1736489f, 0.0f, 0.1736482f, 0.3420201f, 0.5f, 0.6427876f,
		0.7660444f, 0.8660254f, 0.9396926f, 0.9848077f };
	static const float* cosa = sina + 9;


	int i;
	float x, y;


	this->AddVertex(cx, cy, vec);

	float x0 = cx + (0.5f * dx);
	float y0 = cy + (0.5f * dy);
	for (i = 0; i < 9; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, vec);
	}
	x0 -= dx;
	for (; i < 18; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, vec);
	}
	y0 -= dy;
	for (; i < 27; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, vec);
	}
	x0 += dx;
	for (; i < 36; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, vec);
	}

	this->AddVertex(x, cy + (0.5f * dy), vec);


}

void SingleColorBackgroundShaderManager::FillCircle(float cx, float cy, float rx, float ry, std::vector<float>& vec) const
{
	int trianglesCount = 36;


	const float pi2 = 3.14159265359f * 2.0f;
	const float step = pi2 / trianglesCount;

	
	float x, y;

	x = cx + (rx * 1); //cos(0) == 1
	y = cy + (ry * 0); //sin(0) == 0

	this->AddVertex(cx, cy, vec);
	this->AddVertex(x, y, vec);

	for (int i = 1; i < trianglesCount; i++)
	{
		x = cx + (rx * cos(i * step));
		y = cy + (ry * sin(i * step));

		this->AddVertex(x, y, vec);
	}

	x = cx + (rx * 1); //cos(0) == 1
	y = cy + (ry * 0); //sin(0) == 0

	this->AddVertex(x, y, vec);
}

void SingleColorBackgroundShaderManager::AddVertex(float x, float y, std::vector<float>& vec) const
{	
	vec.push_back(x);
	vec.push_back(y); //projection to compensate AR
}