#include "./BackendImage.h"

#include <limits>
#include <array>
#include <algorithm>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"

//=============================================================================


BackendImage::BackendImage(const RenderSettings& r, Format format) :
	BackendBase(r),
	format(format),
	enableTightCanvas(false),
	bgValue({0,0,0,0})
{

	this->colorBlend = [](uint8_t texVal, uint8_t* px, const std::array<float, 4>& textColor, int channelsCount) {
		float alpha = texVal / 255.0;

		for (int c = 0; c < channelsCount; c++)
		{
			uint8_t v = uint8_t(textColor[c] * texVal);
			px[c] = (v * alpha) + (px[c] * (1.0 - alpha));

		}
	};
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
	img.format = format;
	img.w = maxX - minX;
	img.h = maxY - minY;
	img.rawData = std::vector<uint8_t>(img.w * img.h * static_cast<int>(format));


	for (int y = minY, yy = 0; y < maxY; y++, yy++)
	{
		int yW = y * this->rs.deviceW;

		std::copy(
			img.rawData.data() + (minX + yW) * static_cast<int>(format),
			img.rawData.data() + (maxX + yW) * static_cast<int>(format),
			img.rawData.data() + (yy * img.w) * static_cast<int>(format)
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
	LodePNGColorType colorType = LodePNGColorType::LCT_GREY;
	if (format == Format::RGB) colorType = LodePNGColorType::LCT_RGB;
	else if (format == Format::RGBA) colorType = LodePNGColorType::LCT_RGBA;
	
	lodepng::encode(fileName,
		img.rawData.data(), img.w, img.h,
		colorType, 8 * sizeof(uint8_t));
}

void BackendImage::SetBackgroundValue(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	bgValue[0] = r;
	bgValue[1] = g;
	bgValue[2] = b;
	bgValue[3] = a;
}

void BackendImage::SetColorBlend(std::function<void(uint8_t, uint8_t*, const std::array<float, 4>&, int)>& blend)
{
	this->colorBlend = blend;
}

void BackendImage::SetTightDynamicCanvasEnabled(bool val)
{
	this->SetTightDynamicCanvasEnabled(val, TightCanvasSettings());
}

void BackendImage::SetTightDynamicCanvasEnabled(bool val, const TightCanvasSettings& ts)
{
	this->enableTightCanvas = val;
	this->tightSettings = ts;
}

void BackendImage::UpdateTightCanvasSize()
{
	int minY = static_cast<int>(this->quadsAABB.minY);
	int maxY = static_cast<int>(this->quadsAABB.maxY);

	int minX = static_cast<int>(this->quadsAABB.minX);
	int maxX = static_cast<int>(this->quadsAABB.maxX);

	this->SetCanvasSize(
		tightSettings.borderLeft + (maxX - minX) + tightSettings.borderRight, 
		tightSettings.borderTop + (maxY - minY) + tightSettings.borderBottom
	);
	
	int nextQuadOffset = (format == Format::GRAYSCALE) ? 8 : 12;

	for (size_t i = 0; i < mainRenderer->geom.size(); i += nextQuadOffset)
	{		
		mainRenderer->geom[i] = mainRenderer->geom[i] - minX + tightSettings.borderLeft;
		mainRenderer->geom[i + 1] = mainRenderer->geom[i + 1] - minY + tightSettings.borderTop;

		mainRenderer->geom[i + 4] = mainRenderer->geom[i + 4] - minX + tightSettings.borderLeft;
		mainRenderer->geom[i + 5] = mainRenderer->geom[i + 5] - minY + tightSettings.borderTop;
	}

}

void BackendImage::OnCanvasSizeChanges()
{
	img.w = this->rs.deviceW;
	img.h = this->rs.deviceH;
	img.format = format;
	img.rawData.clear();
	img.rawData.resize(img.w * img.h * static_cast<int>(format));

	for (int y = 0; y < img.h; y++)
	{
		for (int x = 0; x < img.w; x++)
		{
			int index = (x + y * this->rs.deviceW) * static_cast<int>(format);
			
			for (int c = 0; c < static_cast<int>(format); c++)
			{				
				img.rawData[index + c] = bgValue[c];
			}
		}
	}

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
	
	int nextQuadOffset = (format == Format::GRAYSCALE) ? 8 : 12;

	std::array<float, 4> rgba = { 1.0f, 1.0f, 1.0f, 1.0f };
	
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

		if (format != Format::GRAYSCALE)
		{
			rgba[0] = mainRenderer->geom[i + 8];
			rgba[1] = mainRenderer->geom[i + 9];
			rgba[2] = mainRenderer->geom[i + 10];
			rgba[3] = mainRenderer->geom[i + 11];
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
				int index = (x + y * this->rs.deviceW) * static_cast<int>(format);
				auto texVal = texData[u + v * this->mainRenderer->fb->GetTextureWidth()];
				
				this->colorBlend(texVal, img.rawData.data() + index, rgba, static_cast<int>(format));

				/*
				float alpha = texVal / 255.0;
				for (int c = 0; c < static_cast<int>(format); c++)
				{
					uint8_t v = uint8_t(rgba[c] * texVal);
					img.rawData[index + c] = (v * alpha) + (img.rawData[index + c] * (1.0 - alpha));
				}
				*/
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

	if (format != Format::GRAYSCALE)
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
