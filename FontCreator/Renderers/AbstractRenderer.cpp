#include "./AbstractRenderer.h"

#include <limits>
#include <algorithm>
#include <filesystem>

#include "../Backends/Shaders/Shaders.h"
#include "../FontBuilder.h"

#include "../Backends/BackendBase.h"

//=============================================================================

const Color AbstractRenderer::DEFAULT_COLOR = { 1,1,1,1 };

const AbstractRenderer::RenderParams AbstractRenderer::DEFAULT_PARAMS = { DEFAULT_COLOR, 1.0f };

std::vector<std::string> AbstractRenderer::GetFontsInDirectory(const std::string& fontDir)
{
	std::vector<std::string> t;

	for (const auto& dirEntry : std::filesystem::directory_iterator(fontDir))
	{
		if (dirEntry.is_regular_file() == false)
		{
			continue;
		}

		t.emplace_back(dirEntry.path().string());
	}
	
	return t;
}

AbstractRenderer::AbstractRenderer(const FontBuilderSettings& fs, 
	std::unique_ptr<BackendBase>&& backend) :
	AbstractRenderer(std::make_shared<FontBuilder>(fs), std::move(backend))
{
}

AbstractRenderer::AbstractRenderer(std::shared_ptr<FontBuilder> fb,
	std::unique_ptr<BackendBase>&& backend) :
	fb(fb),
	backend(std::move(backend)),
	ci({ u8"", 0 }),
	axisYOrigin(AxisYOrigin::TOP),
	extraGlyphSpacingSize(0),
	checkVisibility(true),
	strChanged(false)
{

	this->backend->SetMainRenderer(this);
	
	this->SetCaption(u8"\u2022", 10);			
}


AbstractRenderer::~AbstractRenderer()
{
	this->fb = nullptr;
	this->backend = nullptr;
}

/// <summary>
/// Check is string is visible on the screen
/// By default, check and do not add (render) strings outside screen
/// If test is disabled, we "render" all strings
/// Can be disabled, if we check it before or know that all strings are visible
/// (in this case the test is useless)
/// </summary>
/// <param name="val"></param>
void AbstractRenderer::SetVisibilityCheck(bool val) noexcept
{
	this->checkVisibility = val;
}

BackendBase* AbstractRenderer::GetBackend() const
{
	return this->backend.get();
}


void AbstractRenderer::SetCaption(const StringUtf8& mark)
{
	ci.mark = mark;
}

void AbstractRenderer::SetCaption(const StringUtf8& mark, int offsetInPixels)
{
	this->SetCaption(mark);
	this->SetCaptionOffset(offsetInPixels);
}

void AbstractRenderer::SetCaptionOffset(int offsetInPixels)
{	
	ci.offset = offsetInPixels;
}

void AbstractRenderer::SetBackgroundSettings(std::optional<BackgroundSettings> bs)
{
	this->backend->SetBackground(bs);
}

void AbstractRenderer::SetExtraGlyphSpacingSize(int sizeInPixels)
{
	this->extraGlyphSpacingSize = sizeInPixels;
}

void AbstractRenderer::SetCanvasSize(int w, int h)
{
	this->backend->SetCanvasSize(w, h);

	this->strChanged = true;
}

void AbstractRenderer::SwapCanvasWidthHeight()
{
	this->backend->SwapCanvasWidthHeight();
	
	this->strChanged = true;
}

void AbstractRenderer::SetAxisYOrigin(AxisYOrigin axisY)
{
	this->axisYOrigin = axisY;
}


std::shared_ptr<FontBuilder> AbstractRenderer::GetFontBuilder()
{
	return this->fb;
}

const RenderSettings& AbstractRenderer::GetRenderSettings() const
{
	return this->backend->GetSettings();
}

int AbstractRenderer::GetCanvasWidth() const
{
	return this->backend->GetSettings().deviceW;
}

int AbstractRenderer::GetCanvasHeight() const
{
	return this->backend->GetSettings().deviceH;
}

int AbstractRenderer::GetCaptionOffset() const
{
	return this->ci.offset;
}


/// <summary>
/// Remove all added strings
/// </summary>
void AbstractRenderer::Clear()
{
	this->strChanged = true;

	this->backend->Clear();
}

/// <summary>
/// Render all fonts
/// </summary>
void AbstractRenderer::Render()
{
	this->backend->Render();
}



void AbstractRenderer::AddQuad(const GlyphInfo& gi, int x, int y, const RenderParams& rp)
{
	this->AddQuad(gi, static_cast<float>(x), static_cast<float>(y), rp);
}

/// <summary>
/// Add single "letter" quad to geom buffer
/// </summary>
/// <param name="gi"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void AbstractRenderer::AddQuad(const GlyphInfo& gi, float x, float y, const RenderParams& rp)
{
	this->backend->AddQuad(gi, x, y, rp);
}

/// <summary>
/// Call when single, standalone string is processed
/// </summary>
void AbstractRenderer::OnFinishQuadGroup(const RenderParams& rp)
{
	this->backend->OnFinishQuadGroup(rp);
}
