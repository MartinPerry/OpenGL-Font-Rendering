#include "./BackgroundShaderManager.h"

#include "./Shaders.h"

BackgroundShaderManager::BackgroundShaderManager() :
	positionLocation(-1),	
	colorLocation(-1),
	aabbLocation(-1),
	shape(BackgroundSettings::Shape::SQUARE),
	roundCornerRadius(0.0f),
	min_x(0),
	min_y(0),
	max_x(0),
	max_y(0)
{
}


const char* BackgroundShaderManager::GetVertexShaderSource() const
{
	return BACKGROUND_VERTEX_SHADER_SOURCE;
}

const char* BackgroundShaderManager::GetPixelShaderSource() const
{
	return BACKGROUND_PIXEL_SHADER_SOURCE;
}

void BackgroundShaderManager::SetShape(BackgroundSettings::Shape shape, float radius)
{
	this->shape = shape;
	this->roundCornerRadius = radius;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void BackgroundShaderManager::GetAttributtesUniforms()
{	
	GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
	GL_CHECK(colorLocation = glGetAttribLocation(shaderProgram, "COLOR"));
	GL_CHECK(aabbLocation = glGetAttribLocation(shaderProgram, "AABB"));
	
}

void BackgroundShaderManager::BindVertexAtribs()
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

void BackgroundShaderManager::BindUniforms()
{
	
}

void BackgroundShaderManager::Clear()
{
	this->startingElements.clear();
	this->counts.clear();
}

void BackgroundShaderManager::PreRender()
{
	
}

void BackgroundShaderManager::Render(int quadsCount)
{
	auto type = (shape == BackgroundSettings::Shape::SQUARE) ? GL_TRIANGLES : GL_TRIANGLE_FAN;
	
#if (defined(__APPLE__) || defined(__ANDROID_API__))
	for (int i = 0; i < counts.size(); ++i)
	{
		GL_CHECK(glDrawArrays(type, startingElements[i], counts[i]));
	}
#else
	GL_CHECK(glMultiDrawArrays(type, startingElements.data(), counts.data(), (GLsizei)counts.size()));
#endif
}

int BackgroundShaderManager::GetQuadVertices() const
{
	return (shape == BackgroundSettings::Shape::SQUARE) ? 6 : 38;
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

	min_x = minX;
	min_y = minY;
	max_x = maxX;
	max_y = maxY;

	if (shape == BackgroundSettings::Shape::SQUARE)
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
	else if (shape == BackgroundSettings::Shape::ROUNDED_CORNER_SQUARE)
	{
		//https://stackoverflow.com/questions/74960029/how-to-draw-a-rectangle-in-opengl-with-rounded-corners

		float rx = this->roundCornerRadius * (1.0f / this->canvasW);
		float ry = this->roundCornerRadius * (1.0f / this->canvasH);

		//2 * - projection space is [-1, 1] and we calculate for 0, 1
		rx *= 2;
		ry *= 2;

		float cx = minX + 0.5f * (maxX - minX);
		float cy = minY + 0.5f * (maxY - minY);
		float dx = std::abs(maxX - minX) - 2 * rx;
		float dy = std::abs(maxY - minY) - 2 * ry;
		
		//fix overlap of triangles if rounding was too fast
		//some experimental value
		//dx = std::max(-0.05f * rx, dx);
		//dy = std::max(-0.05f * ry, dy);

		min_x = cx - rx;
		min_y = cy - ry;
		max_x = cx + rx;
		max_y = cy + ry;

		this->FillRoundCornersQuad(cx, cy, dx, dy, rx, ry, rp, vec);
	}
	else if (shape == BackgroundSettings::Shape::CIRCLE)
	{
		const float cx = minX + 0.5f * (maxX - minX);
		const float cy = minY + 0.5f * (maxY - minY);

		float rx, ry;

		//2 * - projection space is [-1, 1] and we calculate for 0, 1
		if (this->roundCornerRadius > 0)
		{
			rx = this->roundCornerRadius * 2.0f * (1.0f / this->canvasW);
			ry = this->roundCornerRadius * 2.0f * (1.0f / this->canvasH);
		}
		else
		{
			const float w = std::abs(maxX - minX);
			const float h = std::abs(maxY - minY);


			// circle fitting inside given rect in screen pixels
			const float rPx = 0.5f * std::max(
				w * 0.5f * this->canvasW,
				h * 0.5f * this->canvasH
			);

			rx = 2.0f * rPx / this->canvasW;
			ry = 2.0f * rPx / this->canvasH;
		}

		min_x = cx - rx;
		min_y = cy - ry;
		max_x = cx + rx;
		max_y = cy + ry;
		
		this->FillCircle(cx, cy, rx, ry, rp, vec);
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

void BackgroundShaderManager::FillRoundCornersQuad(float cx, float cy, float dx, float dy, float rx, float ry, 
	const AbstractRenderer::RenderParams& rp,
	std::vector<float>& vec) const
{

	static const float sina[45] = { 0.0f, 0.1736482f, 0.3420201f, 0.5f, 0.6427876f, 0.7660444f, 0.8660254f,
		0.9396926f, 0.9848077f, 1.0f, 0.9848078f, 0.9396927f, 0.8660255f, 0.7660446f, 0.6427878f, 0.5000002f,
		0.3420205f, 0.1736485f, 3.894144E-07f, -0.1736478f, -0.3420197f, -0.4999996f, -0.6427872f, -0.7660443f,
		-0.8660252f, -0.9396925f, -0.9848077f, -1.0f, -0.9848078f, -0.9396928f, -0.8660257f, -0.7660449f, 
		-0.6427881f, -0.5000006f, -0.3420208f, -0.1736489f, 
		//this is for cosa values that are moved by +9
		0.0f, 0.1736482f, 0.3420201f, 0.5f, 0.6427876f, 0.7660444f, 0.8660254f, 0.9396926f, 0.9848077f };
	static const float* cosa = sina + 9;
	
	
	int i;
	float x, y;
	
	
	this->AddVertex(cx, cy, rp, vec);

	float x0 = cx + (0.5f * dx);
	float y0 = cy + (0.5f * dy);
	for (i = 0; i < 9; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	x0 -= dx;
	for (; i < 18; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	y0 -= dy;
	for (; i < 27; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	x0 += dx;
	for (; i < 36; i++)
	{
		x = x0 + (rx * cosa[i]);
		y = y0 + (ry * sina[i]);
		this->AddVertex(x, y, rp, vec);
	}
	this->AddVertex(x, cy + (0.5f * dy), rp, vec);	
	

}

void BackgroundShaderManager::FillCircle(float cx, float cy, float rx, float ry,
	const AbstractRenderer::RenderParams& rp, std::vector<float>& vec) const
{
	int trianglesCount = 36;

	
	const float pi2 = 3.14159265359f * 2.0f;
	const float step = pi2 / trianglesCount;

	
	float x, y;

	x = cx + (rx * 1); //cos(0) == 1
	y = cy + (ry * 0); //sin(0) == 0

	this->AddVertex(cx, cy, rp, vec);
	this->AddVertex(x, y, rp, vec);

	for (int i = 1; i < trianglesCount; i++)
	{
		x = cx + (rx * cos(i * step));
		y = cy + (ry * sin(i * step));

		this->AddVertex(x, y, rp, vec);
	}

	x = cx + (rx * 1); //cos(0) == 1
	y = cy + (ry * 0); //sin(0) == 0

	this->AddVertex(x, y, rp, vec);

}

void BackgroundShaderManager::AddVertex(float x, float y, const AbstractRenderer::RenderParams& rp, 
	std::vector<float>& vec) const
{
	vec.push_back(x); vec.push_back(y);	
	vec.push_back(rp.bgColor->r); vec.push_back(rp.bgColor->g);
	vec.push_back(rp.bgColor->b); vec.push_back(rp.bgColor->a);

	if (this->aabbLocation != -1)
	{
		vec.push_back(min_x); vec.push_back(min_y);
		vec.push_back(max_x); vec.push_back(max_y);
	}
}