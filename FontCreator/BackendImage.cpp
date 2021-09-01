#include "./BackendImage.h"

#include <limits>
#include <algorithm>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"

//=============================================================================


BackendImage::BackendImage(const RenderSettings& r) :
	BackendBase(r)
{
}


BackendImage::~BackendImage()
{

}

void BackendImage::SaveToFile(const char* fileName)
{
	lodepng::encode(fileName,
		rawData.data(), this->rs.deviceW, this->rs.deviceH,
		LodePNGColorType::LCT_GREY, 8 * sizeof(uint8_t));
}

void BackendImage::CreateTexture()
{
	rawData.clear();
	rawData.resize(this->rs.deviceW * this->rs.deviceH, 0);
}


/// <summary>
/// Render all fonts
/// </summary>
void BackendImage::Render()
{
	bool vboChanged = this->mainRenderer->GenerateGeometry();

	if (mainRenderer->geom.empty())
	{
		return;
	}

	auto texData = this->mainRenderer->fb->GetTextureData();
	
	for (size_t i = 0; i < mainRenderer->geom.size(); i += 8)
	{
		//

		float minX = mainRenderer->geom[i];
		float minY = mainRenderer->geom[i + 1];

		float minU = mainRenderer->geom[i + 2];
		float minV = mainRenderer->geom[i + 3];

		float maxX = mainRenderer->geom[i + 4];
		float maxY = mainRenderer->geom[i + 5];

		float maxU = mainRenderer->geom[i + 6];
		float maxV = mainRenderer->geom[i + 7];

		//
		/*
		float minX = ((q.a.x + 1.0) / 2.0) / this->psW;
		float minY = ((-q.a.y + 1.0) / 2.0) / this->psH;

		float maxX = ((q.c.x + 1.0) / 2.0) / this->psW;
		float maxY = ((-q.c.y + 1.0) / 2.0) / this->psH;

		float minU = q.a.u / this->tW;
		float minV = q.a.v / this->tH;

		float maxU = q.c.u / this->tW;
		float maxV = q.c.v / this->tH;
		*/

		for (int y = minY, v = minV; y < maxY; y++, v++)
		{
			for (int x = minX, u = minU; x < maxX; x++, u++)
			{
				rawData[x + y * this->rs.deviceW] = texData[u + v * this->mainRenderer->fb->GetTextureWidth()];
			}
		}
	}

	this->SaveToFile("D://test.png");
}



void BackendImage::FillTexture()
{
	
}

void BackendImage::Clear()
{
	quadsAABB = AbstractRenderer::AABB();
}

/// <summary>
/// Add single "letter" quad to geom buffer
/// </summary>
/// <param name="gi"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void BackendImage::AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp)
{
	float fx = x + gi.bmpX * rp.scale;
	float fy = y - gi.bmpY * rp.scale;

	//build geometry
	AbstractRenderer::Vertex min, max;

	min.x = fx;
	min.y = fy;
	min.u = static_cast<float>(gi.tx);
	min.v = static_cast<float>(gi.ty);

	max.x = (fx + gi.bmpW * rp.scale);
	max.y = (fy + gi.bmpH * rp.scale);
	max.u = static_cast<float>(gi.tx + gi.bmpW);
	max.v = static_cast<float>(gi.ty + gi.bmpH);

	//=====
	//store it 

	mainRenderer->geom.push_back(min.x); mainRenderer->geom.push_back(min.y);
	mainRenderer->geom.push_back(min.u); mainRenderer->geom.push_back(min.v);

	mainRenderer->geom.push_back(max.x); mainRenderer->geom.push_back(max.y);
	mainRenderer->geom.push_back(max.u); mainRenderer->geom.push_back(max.v);


	if (min.x < quadsAABB.minX) quadsAABB.minX = min.x;
	if (min.y < quadsAABB.minY) quadsAABB.minY = min.y;

	if (max.x > quadsAABB.maxX) quadsAABB.maxX = max.x;
	if (max.y > quadsAABB.maxY) quadsAABB.maxY = max.y;

	mainRenderer->quadsCount++;
}


void BackendImage::FillGeometry()
{
	
}
