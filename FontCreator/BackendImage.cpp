#include "./BackendImage.h"

#include <limits>
#include <array>
#include <algorithm>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"

//=============================================================================


BackendImage::BackendImage(const RenderSettings& r, bool grayScale) :
	BackendBase(r),
	isColored(grayScale == false)
{
}


BackendImage::~BackendImage()
{

}

const std::vector<uint8_t>& BackendImage::GetRawData() const
{
	return this->rawData;
}

BackendImage::ClampedImage BackendImage::GetTightClampedRawData() const
{		
	int minY = static_cast<int>(this->quadsAABB.minY);
	int maxY = static_cast<int>(this->quadsAABB.maxY);
	
	int minX = static_cast<int>(this->quadsAABB.minX);
	int maxX = static_cast<int>(this->quadsAABB.maxX);

	ClampedImage img;
	img.grayScale = (this->isColored == false);
	img.w = maxX - minX;
	img.h = maxY - minY;
	img.rawData = std::vector<uint8_t>(img.w * img.h * 3 * this->isColored);


	for (int y = minY, yy = 0; y < maxY; y++, yy++)
	{
		int yW = y * this->rs.deviceW;

		std::copy(
			rawData.data() + (minX + yW) * 3 * this->isColored,
			rawData.data() + (maxX + yW) * 3 * this->isColored,
			img.rawData.data() + (yy * img.w) * 3 * this->isColored
		);
		
	}

	/*
	LodePNGColorType colorType = (this->isColored) ? LodePNGColorType::LCT_RGB : LodePNGColorType::LCT_GREY;

	lodepng::encode("D://clamp.png",
		img.rawData.data(), img.w, img.h,
		colorType, 8 * sizeof(uint8_t));
	*/
	return img;
}

void BackendImage::SaveToFile(const char* fileName)
{
	LodePNGColorType colorType = (this->isColored) ? LodePNGColorType::LCT_RGB : LodePNGColorType::LCT_GREY;
	
	lodepng::encode(fileName,
		rawData.data(), this->rs.deviceW, this->rs.deviceH,
		colorType, 8 * sizeof(uint8_t));
}

void BackendImage::CreateTexture()
{
	rawData.clear();
	rawData.resize(this->rs.deviceW * this->rs.deviceH * (3 * this->isColored), 0);
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
	
	int nextQuadOffset = (this->isColored) ? 12 : 8;

	std::array<float, 3> rgb = { 1.0f, 1.0f, 1.0f };
	
	for (size_t i = 0; i < mainRenderer->geom.size(); i += nextQuadOffset)
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

		if (this->isColored)
		{
			rgb[0] = mainRenderer->geom[i + 8];
			rgb[1] = mainRenderer->geom[i + 9];
			rgb[2] = mainRenderer->geom[i + 10];
		}

		//clamp to prevent being outside window

		if (minX < 0)
		{
			//move texture U
			minU = minU - minX;
		}
		if (minY < 0)
		{
			//move texture V
			minV = minV - minY;
		}

		minX = std::clamp<float>(minX, 0, this->rs.deviceW);
		maxX = std::clamp<float>(maxX, 0, this->rs.deviceW);

		minY = std::clamp<float>(minY, 0, this->rs.deviceH);
		maxY = std::clamp<float>(maxY, 0, this->rs.deviceH);
		//

		for (int y = minY, v = minV; y < maxY; y++, v++)
		{			
			for (int x = minX, u = minU; x < maxX; x++, u++)
			{				
				int index = (x + y * this->rs.deviceW) * 3 * this->isColored;
				auto texVal = texData[u + v * this->mainRenderer->fb->GetTextureWidth()];

				for (int c = 0; c < 3 * this->isColored; c++)
				{
					rawData[index + c] = rgb[c] * texVal;
				}
			}
		}
	}

	//this->SaveToFile("D://test.png");
	//this->GetTightClampedRawData();	
}



void BackendImage::FillFontTexture()
{
	//empty, not used
}

/// <summary>
/// Called after strings are cleared
/// </summary>
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

	if (this->isColored)
	{
		mainRenderer->geom.push_back(rp.color.r); mainRenderer->geom.push_back(rp.color.g);
		mainRenderer->geom.push_back(rp.color.b); mainRenderer->geom.push_back(rp.color.a);
	}

	if (min.x < quadsAABB.minX) quadsAABB.minX = min.x;
	if (min.y < quadsAABB.minY) quadsAABB.minY = min.y;

	if (max.x > quadsAABB.maxX) quadsAABB.maxX = max.x;
	if (max.y > quadsAABB.maxY) quadsAABB.maxY = max.y;

	mainRenderer->quadsCount++;
}


void BackendImage::FillGeometry()
{
	//not used
}
