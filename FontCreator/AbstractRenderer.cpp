#include "./AbstractRenderer.h"

#include <limits>
#include <algorithm>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"

#include "./GLRenderer.h"

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
	std::unique_ptr<GLRenderer>&& renderer) :
	AbstractRenderer(std::make_shared<FontBuilder>(fs), std::move(renderer))
{
}

AbstractRenderer::AbstractRenderer(std::shared_ptr<FontBuilder> fb,
	std::unique_ptr<GLRenderer>&& renderer) :
	fb(fb),
	renderer(std::move(renderer)),	
	ci({ UTF8_TEXT(u8""), 0 }),
	axisYOrigin(AxisYOrigin::TOP),
	quadsCount(0),
	strChanged(false)
{

	this->renderer->SetMainRenderer(this);

	this->SetCaption(UTF8_TEXT(u8"\u2022"), 10);
		
	this->tW = 1.0f / static_cast<float>(this->fb->GetTextureWidth());  //1.0 / pixel size in width
	this->tH = 1.0f / static_cast<float>(this->fb->GetTextureHeight()); //1.0 / pixel size in height

}


AbstractRenderer::~AbstractRenderer()
{

	this->fb = nullptr;
	
}


GLRenderer* AbstractRenderer::GetRenderer() const
{
	return this->renderer.get();
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
	//take half of new line offset and add extra 20%
	ci.offset = offsetInPixels;// static_cast<int>(this->fb->GetNewLineOffsetBasedOnGlyph(ci.mark[0]) * 0.5 * 1.2);	
}


void AbstractRenderer::SetCanvasSize(int w, int h)
{
	this->renderer->SetCanvasSize(w, h);

	this->strChanged = true;
}

void AbstractRenderer::SwapCanvasWidthHeight()
{
	this->renderer->SwapCanvasWidthHeight();
	
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
	return this->renderer->GetSettings();
}

int AbstractRenderer::GetCanvasWidth() const
{
	return this->renderer->GetSettings().deviceW;
}

int AbstractRenderer::GetCanvasHeight() const
{
	return this->renderer->GetSettings().deviceH;
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
}

/// <summary>
/// Render all fonts
/// </summary>
void AbstractRenderer::Render()
{
	this->renderer->Render();
}





/// <summary>
/// Add single "letter" quad to geom buffer
/// </summary>
/// <param name="gi"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void AbstractRenderer::AddQuad(const GlyphInfo& gi, float x, float y, const RenderParams& rp)
{
	this->renderer->AddQuad(gi, x, y, rp);	
}


