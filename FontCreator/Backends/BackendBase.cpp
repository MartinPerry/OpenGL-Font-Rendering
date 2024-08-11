#include "./BackendBase.h"


BackendBase::BackendBase(const RenderSettings& r) : 
	quadsCount(0),
	mainRenderer(nullptr),
	rs(r),
	enabled(true)
{
	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height	
}

BackendBase::~BackendBase()
{
	this->Clear();
}

void BackendBase::Clear()
{
	this->geom.clear();
	this->quadsCount = 0;
}

void BackendBase::SetMainRenderer(AbstractRenderer* mainRenderer)
{
	this->mainRenderer = mainRenderer;	
	this->SetCanvasSize(rs.deviceW, rs.deviceH);	
}

const RenderSettings& BackendBase::GetSettings() const
{
	return this->rs;
}

void BackendBase::SetEnabled(bool val)
{
	this->enabled = val;
}

bool BackendBase::IsEnabled() const
{
	return this->enabled;
}

void BackendBase::SetCanvasSize(int w, int h)
{
	this->rs.deviceW = w;
	this->rs.deviceH = h;

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

	this->OnCanvasSizeChanges();
}

void BackendBase::SwapCanvasWidthHeight()
{
	std::swap(this->rs.deviceW, this->rs.deviceH);

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

	this->OnCanvasSizeChanges();
}

