#include "./BackendBackgroundOpenGL.h"

#include "./Shaders/Shaders.h"
#include "./Shaders/SingleColorBackgroundShaderManager.h"
#include "./Shaders/BackgroundShaderManager.h"
#include "./Shaders/BackgroundShadowShaderManager.h"

BackendBackgroundOpenGL::BackendBackgroundOpenGL(const BackgroundSettings& bs, const RenderSettings& r) :	
	BackendBackgroundOpenGL(bs, r,
		nullptr, 		
		nullptr,
		bs.shadow.has_value() ?
			std::dynamic_pointer_cast<IShaderManager>(std::make_shared<BackgroundShadowShaderManager>(*bs.shadow)) : (
			bs.color.has_value() ? 
				std::dynamic_pointer_cast<IShaderManager>(std::make_shared<SingleColorBackgroundShaderManager>()) :
				std::dynamic_pointer_cast<IShaderManager>(std::make_shared<BackgroundShaderManager>())
			)
	)
{
}

BackendBackgroundOpenGL::BackendBackgroundOpenGL(const BackgroundSettings& bs, const RenderSettings& r,
	const char* vSource, const char* pSource, std::shared_ptr<IShaderManager> sm) :
	BackendOpenGL(r, vSource, pSource, sm)	
{
	this->SetBackgroundSettings(bs);
}

BackendBackgroundOpenGL::~BackendBackgroundOpenGL()
{
}

const BackgroundSettings& BackendBackgroundOpenGL::GetBackgroundSettings() const
{
	return this->bs;
}

void BackendBackgroundOpenGL::SetBackgroundSettings(const BackgroundSettings& bs)
{
	this->bs = bs;

	if (auto tmp = std::dynamic_pointer_cast<SingleColorBackgroundShaderManager>(this->sm))
	{
		tmp->SetColor(bs.color->r, bs.color->g, bs.color->b, bs.color->a);
		tmp->SetShape(bs.shape, bs.cornerRadius);
	}
	else if (auto tmp = std::dynamic_pointer_cast<BackgroundShaderManager>(this->sm))
	{
		tmp->SetShape(bs.shape, bs.cornerRadius);
	}
	else if (auto tmp = std::dynamic_pointer_cast<BackgroundShadowShaderManager>(this->sm))
	{		
		if (bs.color.has_value())
		{
			tmp->SetColor(bs.color->r, bs.color->g, bs.color->b, bs.color->a);
		}
		tmp->SetShape(bs.shape, bs.cornerRadius);
	}
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
	if (rs.glVersion == 2)
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
	
	this->sm->BindUniforms();
	this->sm->PreRender();
	this->sm->Render(this->quadsCount);

#ifdef __ANDROID_API__
	if (rs.glVersion != 2)
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

	min.x -= (bs.padding * rp.scale * psW);
	min.y -= (bs.padding * rp.scale * psH);

	max.x += (bs.padding * rp.scale * psW);
	max.y += (bs.padding * rp.scale * psH);
		
	this->sm->FillQuadVertexData(min, max, rp, this->geom);

	curQuadAabb = AABB();
	

	this->quadsCount++;
}