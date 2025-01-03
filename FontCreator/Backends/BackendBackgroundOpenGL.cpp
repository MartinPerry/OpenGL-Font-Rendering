#include "./BackendBackgroundOpenGL.h"

#include "./Shaders/Shaders.h"
#include "./Shaders/SingleColorBackgroundShaderManager.h"
#include "./Shaders/BackgroundShaderManager.h"

BackendBackgroundOpenGL::BackendBackgroundOpenGL(const BackgroundSettings& bs, const RenderSettings& r, int glVersion) :	
	BackendBackgroundOpenGL(bs, r, glVersion,
		bs.color.has_value() ? 
			SINGLE_COLOR_BACKGROUND_VERTEX_SHADER_SOURCE : 
			(bs.shadow ? BACKGROUND_SHADOW_VERTEX_SHADER_SOURCE : BACKGROUND_VERTEX_SHADER_SOURCE), 
		bs.color.has_value() ? 
			SINGLE_COLOR_BACKGROUND_PIXEL_SHADER_SOURCE : 
			(bs.shadow ? BACKGROUND_SHADOW_PIXEL_SHADER_SOURCE : BACKGROUND_PIXEL_SHADER_SOURCE),
		bs.color.has_value() ? 
			std::dynamic_pointer_cast<IShaderManager>(std::make_shared<SingleColorBackgroundShaderManager>()) :
			std::dynamic_pointer_cast<IShaderManager>(std::make_shared<BackgroundShaderManager>())
	)
{
}

BackendBackgroundOpenGL::BackendBackgroundOpenGL(const BackgroundSettings& bs, const RenderSettings& r, int glVersion,
	const char* vSource, const char* pSource, std::shared_ptr<IShaderManager> sm) :
	BackendOpenGL(r, glVersion, vSource, pSource, sm), 
	bs(bs)	
{
	if (auto tmp = std::dynamic_pointer_cast<SingleColorBackgroundShaderManager>(this->sm))
	{
		tmp->SetColor(bs.color->r, bs.color->g, bs.color->b, bs.color->a);
		tmp->SetCornerRadius(bs.cornerRadius * this->psW);
	}
	else if (auto tmp = std::dynamic_pointer_cast<BackgroundShaderManager>(this->sm))
	{		
		tmp->SetCornerRadius(bs.cornerRadius * this->psW);
		tmp->SetShadowEnabled(bs.shadow);
	}
}

BackendBackgroundOpenGL::~BackendBackgroundOpenGL()
{
}

void BackendBackgroundOpenGL::SetBackgroundSettings(const BackgroundSettings& bs)
{
	this->bs = bs;
}

/// <summary>
/// Render all fonts with pre and post render callbacks
/// </summary>
/// <param name="preDrawCallback"></param>
/// <param name="postDrawCallback"></param>
void BackendBackgroundOpenGL::Render(std::function<void(GLuint)> preDrawCallback,
	std::function<void()> postDrawCallback)
{
	if (this->enabled == false)
	{
		return;
	}

#ifdef THREAD_SAFETY
	//std::shared_lock<std::shared_timed_mutex> lk(mainRenderer->m);
#endif

	
	//bool vboChanged = this->mainRenderer->GenerateGeometry();

	if (this->geom.empty())
	{
		return;
	}
	
	//activate shader	
	FONT_BIND_SHADER(shader.program);

	//render
	FONT_BIND_ARRAY_BUFFER(this->vbo);

#ifdef __ANDROID_API__
	if (glVersion == 2)
	{
		this->sm->BindVertexAtribs();
	}
	else
	{
		FONT_BIND_VAO(this->vao);
	}
#else
	FONT_BIND_VAO(this->vao);
#endif		

	this->sm->PreRender();
	this->sm->Render(this->quadsCount);

#ifdef __ANDROID_API__
	if (glVersion != 2)
	{
		FONT_UNBIND_VAO;
	}
#else
	FONT_UNBIND_VAO;
#endif	

	//deactivate shader
	FONT_UNBIND_SHADER;
}


void BackendBackgroundOpenGL::AddQuad(AbstractRenderer::Vertex& vmin, AbstractRenderer::Vertex& vmax, 
	const AbstractRenderer::RenderParams& rp)
{
	curQuadAabb.Update(vmin.x, vmin.y, vmax.x - vmin.x, vmax.y - vmin.y);	
}

void BackendBackgroundOpenGL::OnFinishQuadGroup(const AbstractRenderer::RenderParams& rp)
{
	AbstractRenderer::Vertex min, max;

	min.x = curQuadAabb.minX;
	min.y = curQuadAabb.minY;

	max.x = curQuadAabb.maxX;
	max.y = curQuadAabb.maxY;

	min.x -= (bs.padding * psW);
	min.y -= (bs.padding * psH);

	max.x += (bs.padding * psW);
	max.y += (bs.padding * psH);
		
	this->sm->FillQuadVertexData(min, max, rp, this->geom);

	curQuadAabb = AABB();
	

	this->quadsCount++;
}