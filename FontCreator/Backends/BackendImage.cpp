#include "./BackendImage.h"

#include <limits>
#include <array>
#include <algorithm>

#include "../FontBuilder.h"


//=============================================================================


BackendImage::BackendImage(const RenderSettings& r, Format format) :
	BackendBase(r),
	format(format),
	enableTightCanvas(false),
	bgValue({0,0,0,0})
{

	this->colorBlend = [](uint8_t texVal, uint8_t* px, const std::array<float, 4>& textColor, int channelsCount) {
		float alpha = texVal / 255.0f;

		for (int c = 0; c < channelsCount; c++)
		{
			uint8_t v = uint8_t(textColor[c] * texVal);
			px[c] = static_cast<uint8_t>((v * alpha) + (px[c] * (1.0f - alpha)));

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

void BackendImage::SetBackground(const BackgroundSettings& bs)
{
	bgValue[0] = static_cast<uint8_t>(bs.color.r * 255);
	bgValue[1] = static_cast<uint8_t>(bs.color.g * 255);
	bgValue[2] = static_cast<uint8_t>(bs.color.b * 255);
	bgValue[3] = static_cast<uint8_t>(bs.color.a * 255);

	this->OnCanvasChanges();
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

	for (size_t i = 0; i < this->geom.size(); i += nextQuadOffset)
	{		
		this->geom[i] = this->geom[i] - minX + tightSettings.borderLeft;
		this->geom[i + 1] = this->geom[i + 1] - minY + tightSettings.borderTop;

		this->geom[i + 4] = this->geom[i + 4] - minX + tightSettings.borderLeft;
		this->geom[i + 5] = this->geom[i + 5] - minY + tightSettings.borderTop;
	}

}

void BackendImage::OnCanvasChanges()
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

	if (this->geom.empty())
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
	
	for (size_t i = 0; i < this->geom.size(); i += nextQuadOffset)
	{
		//

		int minX = static_cast<int>(this->geom[i]);
		int minY = static_cast<int>(this->geom[i + 1]);

		int minU = static_cast<int>(this->geom[i + 2]);
		int minV = static_cast<int>(this->geom[i + 3]);

		int maxX = static_cast<int>(this->geom[i + 4]);
		int maxY = static_cast<int>(this->geom[i + 5]);

		int maxU = static_cast<int>(this->geom[i + 6]);
		int maxV = static_cast<int>(this->geom[i + 7]);

		if (format != Format::GRAYSCALE)
		{
			rgba[0] = this->geom[i + 8];
			rgba[1] = this->geom[i + 9];
			rgba[2] = this->geom[i + 10];
			rgba[3] = this->geom[i + 11];
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
	BackendBase::Clear();
	quadsAABB = AABB();
}

/// <summary>
/// Add single "letter" quad to geom buffer
/// </summary>
/// <param name="gi"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void BackendImage::AddQuad(AbstractRenderer::Vertex& vmin, AbstractRenderer::Vertex& vmax, const AbstractRenderer::RenderParams& rp)
{	
	//=====
	//store it 

	this->geom.push_back(vmin.x); this->geom.push_back(vmin.y);
	this->geom.push_back(vmin.u); this->geom.push_back(vmin.v);

	this->geom.push_back(vmax.x); this->geom.push_back(vmax.y);
	this->geom.push_back(vmax.u); this->geom.push_back(vmax.v);

	if (format != Format::GRAYSCALE)
	{
		this->geom.push_back(rp.color.r); this->geom.push_back(rp.color.g);
		this->geom.push_back(rp.color.b); this->geom.push_back(rp.color.a);
	}

	if (vmin.x < quadsAABB.minX) quadsAABB.minX = vmin.x;
	if (vmin.y < quadsAABB.minY) quadsAABB.minY = vmin.y;

	if (vmax.x > quadsAABB.maxX) quadsAABB.maxX = vmax.x;
	if (vmax.y > quadsAABB.maxY) quadsAABB.maxY = vmax.y;

	this->quadsCount++;
}


void BackendImage::FillGeometry()
{
	//not used
}
