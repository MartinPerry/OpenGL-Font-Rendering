#include "./BackendBase.h"

#include "../TextureBuilders/IFontBuilder.h"

BackendBase::BackendBase(const RenderSettings& r) : 
	quadsCount(0),
	mainRenderer(nullptr),
	rs(r),
	enabled(true),
	heightPx(std::numeric_limits<float>::lowest()),
	heightThresholdKeepBackground(false),
	uvOffset(0.0f)
{
	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height	

	this->geom.resize(r.layersCount, {});
	this->quadsCount.resize(r.layersCount, 0);
}

BackendBase::~BackendBase()
{
	this->Clear();

	this->geom.clear();
}

void BackendBase::Clear()
{
	for (size_t i = 0; i < this->geom.size(); i++)
	{
		this->geom[i].clear();
		this->quadsCount[i] = 0;
	}	
}

void BackendBase::SetBackground(std::optional<BackgroundSettings> bs)
{
}

void BackendBase::SetMainRenderer(AbstractRenderer* mainRenderer)
{
	this->mainRenderer = mainRenderer;	
	this->SetCanvasSize(rs.deviceW, rs.deviceH);	

	if (auto fb = mainRenderer->GetFontBuilder())
	{
		if (fb->GetFontBorder() > 0)
		{
			this->uvOffset = 0.5f;
		}
	}
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

void BackendBase::SetRenderSizeThreshold(float heightPx, bool keepBackground)
{
	this->heightPx = heightPx;
	this->heightThresholdKeepBackground = keepBackground;
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

bool BackendBase::IsGeomEmpty() const
{
	bool geomEmpty = true;
	for (size_t i = 0; i < this->geom.size(); i++)
	{
		if (this->geom.empty() == false)
		{
			geomEmpty = false;
			break;
		}
	}

	return geomEmpty;
}

void BackendBase::AddEmptyQuad(float x, float y, float w, float h, const AbstractRenderer::RenderParams& rp)
{
}

void BackendBase::AddQuad(const GlyphInfo& gi, float x, float y, const AbstractRenderer::RenderParams& rp)
{
	
	float fx = x + gi.bmpX * rp.scale;
	float fy = y - gi.bmpY * rp.scale;
	
	//build geometry
	AbstractRenderer::Vertex min, max;

	min.x = fx;
	min.y = fy;
	min.u = static_cast<float>(gi.tx);
	min.v = static_cast<float>(gi.ty);

	max.x = fx + gi.bmpW * rp.scale;
	max.y = fy + gi.bmpH * rp.scale;
	max.u = static_cast<float>(gi.tx + gi.bmpW);
	max.v = static_cast<float>(gi.ty + gi.bmpH);

	/*
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
	*/
	
	

	this->AddQuad(min, max, rp);
}

void BackendBase::OnFinishQuadGroup(const AbstractRenderer::RenderParams& rp)
{
}
