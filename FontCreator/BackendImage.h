#ifndef BACKEND_IMAGE_H
#define BACKEND_IMAGE_H


#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>
#include <algorithm>

#include "./BackendBase.h"

#include "./Externalncludes.h"


class BackendImage : public BackendBase
{
public:

	BackendImage(const RenderSettings& r, bool grayScale);
	
	virtual ~BackendImage();

	const std::vector<uint8_t>& GetRawData() const;
	std::vector<uint8_t> GetTightClampedRawData() const;

	void SaveToFile(const char* fileName);
	
	void Clear();
	void AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp) override;

	void FillFontTexture() override;
	void FillGeometry() override;

	void Render() override;
	

protected:
	bool isColored;
	std::vector<uint8_t> rawData;
	
	AbstractRenderer::AABB quadsAABB;

	void CreateTexture() override;
	

};

#endif
