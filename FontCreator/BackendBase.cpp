#include "./BackendBase.h"


BackendBase::BackendBase(const RenderSettings& r) : 
	mainRenderer(nullptr),
	rs(r),
	enabled(true)
{
	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

}

BackendBase::~BackendBase()
{
}

void BackendBase::SetMainRenderer(AbstractRenderer* mainRenderer)
{
	this->mainRenderer = mainRenderer;
	this->CreateTexture();
}

const RenderSettings& BackendBase::GetSettings() const
{
	return this->rs;
}

void BackendBase::SetCanvasSize(int w, int h)
{
	this->rs.deviceW = w;
	this->rs.deviceH = h;

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

}

void BackendBase::SwapCanvasWidthHeight()
{
	std::swap(this->rs.deviceW, this->rs.deviceH);

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

}

