#include "./AbstractRenderer.h"

#include <limits>
#include <algorithm>

#include "../Backends/Shaders/Shaders.h"
#include "../FontBuilder.h"

#include "../Backends/BackendBase.h"

//=============================================================================

const AbstractRenderer::Color AbstractRenderer::DEFAULT_COLOR = { 1,1,1,1 };

const AbstractRenderer::RenderParams AbstractRenderer::DEFAULT_PARAMS = { DEFAULT_COLOR, 1.0f };

std::vector<std::string> AbstractRenderer::GetFontsInDirectory(const std::string& fontDir)
{
	std::vector<std::string> t;

	DIR* dir = opendir(fontDir.c_str());

	if (dir == nullptr)
	{
		printf("Failed to open dir %s\n", fontDir.c_str());
		return t;
	}

	struct dirent* ent;
	std::string fullPath;

	/* print all the files and directories within directory */
	while ((ent = readdir(dir)) != nullptr)
	{
		if (ent->d_name[0] == '.')
		{
			continue;
		}
		if (ent->d_type == DT_REG)
		{
			fullPath = fontDir;
#ifdef _WIN32
			fullPath = dir->patt; //full path using Windows dirent
			fullPath = fullPath.substr(0, fullPath.length() - 1);
#else
			if (fullPath[fullPath.length() - 1] != '/')
			{
				fullPath += '/';
			}
#endif				
			fullPath += ent->d_name;


			t.push_back(std::move(fullPath));
		}
	}


	closedir(dir);

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
	ci({ UTF8_TEXT(u8""), 0 }),
	axisYOrigin(AxisYOrigin::TOP),
	quadsCount(0),
	strChanged(false)
{

	this->backend->SetMainRenderer(this);

	this->SetCaption(UTF8_TEXT(u8"\u2022"), 10);
			
}


AbstractRenderer::~AbstractRenderer()
{
	this->fb = nullptr;
	this->backend = nullptr;
}


BackendBase* AbstractRenderer::GetBackend() const
{
	return this->backend.get();
}


void AbstractRenderer::SetCaption(const UnicodeString& mark)
{
	ci.mark = mark;
}

void AbstractRenderer::SetCaption(const UnicodeString& mark, int offsetInPixels)
{
	this->SetCaption(mark);
	this->SetCaptionOffset(offsetInPixels);
}

void AbstractRenderer::SetCaptionOffset(int offsetInPixels)
{	
	ci.offset = offsetInPixels;
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
	this->geom.clear();
	this->quadsCount = 0;

	this->backend->Clear();
}

/// <summary>
/// Render all fonts
/// </summary>
void AbstractRenderer::Render()
{
	this->backend->Render();
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


