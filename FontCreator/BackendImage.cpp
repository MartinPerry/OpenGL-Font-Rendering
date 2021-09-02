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
	channelsCount(grayScale ? 1 : 3),
	enableTightCanvas(false)
{
}


BackendImage::~BackendImage()
{

}

const BackendImage::ImageData& BackendImage::GetRawData() const
{
	return this->img;
}

BackendImage::ImageData BackendImage::GetTightClampedRawData() const
{		
	int minY = static_cast<int>(this->quadsAABB.minY);
	int maxY = static_cast<int>(this->quadsAABB.maxY);
	
	int minX = static_cast<int>(this->quadsAABB.minX);
	int maxX = static_cast<int>(this->quadsAABB.maxX);

	ImageData img;
	img.grayScale = (channelsCount == 1);
	img.w = maxX - minX;
	img.h = maxY - minY;
	img.rawData = std::vector<uint8_t>(img.w * img.h * channelsCount);


	for (int y = minY, yy = 0; y < maxY; y++, yy++)
	{
		int yW = y * this->rs.deviceW;

		std::copy(
			img.rawData.data() + (minX + yW) * channelsCount,
			img.rawData.data() + (maxX + yW) * channelsCount,
			img.rawData.data() + (yy * img.w) * channelsCount
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
	LodePNGColorType colorType = (channelsCount == 3) ? LodePNGColorType::LCT_RGB : LodePNGColorType::LCT_GREY;
	
	lodepng::encode(fileName,
		img.rawData.data(), img.w, img.h,
		colorType, 8 * sizeof(uint8_t));
}


void BackendImage::SetTightDynamicCanvasEnabled(bool val)
{
	this->enableTightCanvas = val;
}

void BackendImage::UpdateTightCanvasSize()
{
	int minY = static_cast<int>(this->quadsAABB.minY);
	int maxY = static_cast<int>(this->quadsAABB.maxY);

	int minX = static_cast<int>(this->quadsAABB.minX);
	int maxX = static_cast<int>(this->quadsAABB.maxX);

	this->SetCanvasSize(maxX - minX, maxY - minY);
	
	int nextQuadOffset = (channelsCount == 3) ? 12 : 8;

	for (size_t i = 0; i < mainRenderer->geom.size(); i += nextQuadOffset)
	{		
		mainRenderer->geom[i] -= minX;
		mainRenderer->geom[i + 1] -= minY;

		mainRenderer->geom[i + 4] -= minX;
		mainRenderer->geom[i + 5] -= minY;

	}

}

void BackendImage::OnCanvasSizeChanges()
{
	img.w = this->rs.deviceW;
	img.h = this->rs.deviceH;
	img.grayScale = (channelsCount == 1);
	img.rawData.clear();
	img.rawData.resize(img.w * img.h * channelsCount, 0);
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

	if (this->enableTightCanvas)
	{
		this->UpdateTightCanvasSize();
	}


	auto texData = this->mainRenderer->fb->GetTextureData();
	
	int nextQuadOffset = (channelsCount == 3) ? 12 : 8;

	std::array<float, 3> rgb = { 1.0f, 1.0f, 1.0f };
	
	for (size_t i = 0; i < mainRenderer->geom.size(); i += nextQuadOffset)
	{
		//

		int minX = static_cast<int>(mainRenderer->geom[i]);
		int minY = static_cast<int>(mainRenderer->geom[i + 1]);

		int minU = static_cast<int>(mainRenderer->geom[i + 2]);
		int minV = static_cast<int>(mainRenderer->geom[i + 3]);

		int maxX = static_cast<int>(mainRenderer->geom[i + 4]);
		int maxY = static_cast<int>(mainRenderer->geom[i + 5]);

		int maxU = static_cast<int>(mainRenderer->geom[i + 6]);
		int maxV = static_cast<int>(mainRenderer->geom[i + 7]);

		if (channelsCount == 3)
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

		minX = std::clamp(minX, 0, this->rs.deviceW);
		maxX = std::clamp(maxX, 0, this->rs.deviceW);

		minY = std::clamp(minY, 0, this->rs.deviceH);
		maxY = std::clamp(maxY, 0, this->rs.deviceH);
		//

		for (int y = minY, v = minV; y < maxY; y++, v++)
		{			
			for (int x = minX, u = minU; x < maxX; x++, u++)
			{				
				int index = (x + y * this->rs.deviceW) * channelsCount;
				auto texVal = texData[u + v * this->mainRenderer->fb->GetTextureWidth()];

				for (int c = 0; c < channelsCount; c++)
				{
					img.rawData[index + c] = uint8_t(rgb[c] * texVal);
				}
			}
		}
	}

	//this->SaveToFile("D://test2.png");
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

	if (channelsCount == 3)
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
