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

void BackendBase::SetBackground(std::optional<BackgroundSettings> bs)
{
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

	this->OnCanvasChanges();
}

void BackendBase::SwapCanvasWidthHeight()
{
	std::swap(this->rs.deviceW, this->rs.deviceH);

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

	this->OnCanvasChanges();
}

void BackendBase::AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp)
{
	
	float fx = x + gi.bmpX;// *rp.scale;
	float fy = y - gi.bmpY;// *rp.scale;
	
	//build geometry
	AbstractRenderer::Vertex min, max;

	min.x = fx;
	min.y = fy;
	min.u = static_cast<float>(gi.tx);
	min.v = static_cast<float>(gi.ty);

	max.x = fx + gi.bmpW;// *rp.scale);
	max.y = fy + gi.bmpH;// *rp.scale);
	max.u = static_cast<float>(gi.tx + gi.bmpW);
	max.v = static_cast<float>(gi.ty + gi.bmpH);

	
	if (rp.scale != 1.0)
	{
		//move to original -> scale -> move back
		float cx = min.x + gi.bmpW * 0.5f;
		float cy = min.y + gi.bmpH * 0.5f;

		min.x = (min.x - cx) * rp.scale + cx;
		min.y = (min.y - cy) * rp.scale + cy;

		max.x = (max.x - cx) * rp.scale + cx;
		max.y = (max.y - cy) * rp.scale + cy;
	}
	
	/*
	if ((rp.border > 0) && (rp.borderColor))
	{		
		AbstractRenderer::RenderParams borderRp(rp.borderColor.value(), rp.scale);
		auto borderMin = min;
		auto borderMax = max;

		//borderMin.x -= rp.border;
		//borderMin.y -= rp.border;

		//borderMax.x += rp.border;
		//borderMax.y += rp.border;

		float sx = 1.5;
		float sy = 1.5;

		float cx = borderMin.x + gi.bmpW * 0.5f;
		float cy = borderMin.y + gi.bmpH * 0.5f;

		borderMin.x = (borderMin.x - cx) * sx + cx;
		borderMin.y = (borderMin.y - cy) * sy + cy;

		borderMax.x = (borderMax.x - cx) * sx + cx;
		borderMax.y = (borderMax.y - cy) * sy + cy;

		this->AddQuad(borderMin, borderMax, borderRp);
	
	}
	*/

	this->AddQuad(min, max, rp);
}

void BackendBase::OnFinishQuadGroup(const AbstractRenderer::RenderParams& rp)
{
}
