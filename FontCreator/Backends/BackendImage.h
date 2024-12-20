#ifndef BACKEND_IMAGE_H
#define BACKEND_IMAGE_H


#include <vector>
#include <array>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>

#include "./BackendBase.h"

#include "../Externalncludes.h"

class BackendImage : public BackendBase
{
public:

	enum class Format 
	{
		GRAYSCALE = 1,
		RGB = 3,
		RGBA = 4
	};

	struct TightCanvasSettings 
	{
		int borderLeft = 0;
		int borderRight = 0;
		int borderTop = 0;
		int borderBottom = 0;
	};

	struct ImageData
	{
		Format format;
		int w;
		int h;
		std::vector<uint8_t> rawData;
	};

	BackendImage(const RenderSettings& r, Format format);
	
	virtual ~BackendImage();

	const ImageData& GetRawData() const;
	ImageData GetTightClampedRawData() const;

	void SaveToFile(const char* fileName);
	
	void SetBackground(std::optional<BackgroundSettings> bs) override;
	void SetColorBlend(std::function<void(uint8_t, uint8_t*, const std::array<float, 4>&, int)>& blend);

	void SetTightDynamicCanvasEnabled(bool val);
	void SetTightDynamicCanvasEnabled(bool val, const TightCanvasSettings& ts);

	void Clear() override;
	
	void FillFontTexture() override;
	void FillGeometry() override;

	void Render() override;
	

protected:
	Format format;
	ImageData img;
	
	std::array<uint8_t, 4> bgValue;
	std::function<void(uint8_t, uint8_t*, const std::array<float, 4>&, int)> colorBlend;

	bool enableTightCanvas;
	TightCanvasSettings tightSettings;

	AABB quadsAABB;

		
	void UpdateTightCanvasSize();

	void OnCanvasChanges() override;

	void AddQuad(AbstractRenderer::Vertex& vmin, AbstractRenderer::Vertex& vmax, const AbstractRenderer::RenderParams& rp) override;
};

#endif
