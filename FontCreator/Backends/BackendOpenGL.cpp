#include "./BackendOpenGL.h"

#include <limits>
#include <algorithm>

#include "../FontBuilder.h"

#include "./Shaders/Shaders.h"
#include "./Shaders/DefaultFontShaderManager.h"

#include "./BackendBackgroundOpenGL.h"

//=============================================================================
// GL helpers
//=============================================================================



//=============================================================================


BackendOpenGL::BackendOpenGL(const RenderSettings& r, int glVersion) :
	BackendOpenGL(r, glVersion,
                       DEFAULT_VERTEX_SHADER_SOURCE, DEFAULT_PIXEL_SHADER_SOURCE,
                       std::make_shared<DefaultFontShaderManager>())
{
}

BackendOpenGL::BackendOpenGL(const RenderSettings& r, int glVersion,
	const char* vSource, const char* pSource, std::shared_ptr<IShaderManager> sm) :
	BackendBase(r),
	glVersion(glVersion),
	sm(sm),		
	vbo(0),
	vao(0),
	texture(0),
	background(nullptr)
{	
	this->shader.program = 0;
	this->shader.pSource = pSource;
	this->shader.vSource = vSource;

	if ((pSource == DEFAULT_PIXEL_SHADER_SOURCE) && (vSource == DEFAULT_VERTEX_SHADER_SOURCE))
	{
		this->shader.isDefault = true;
	}
	else
	{
		this->shader.isDefault = false;
	}

		
	this->InitGL();
	
}


BackendOpenGL::~BackendOpenGL()
{
		
	//this will end in error, if OpenGL is not initialized during
	//destruction
	//however, that should be OK

	FONT_UNBIND_SHADER;
	FONT_UNBIND_TEXTURE_2D;
	FONT_UNBIND_ARRAY_BUFFER;
	FONT_UNBIND_VAO;

	GL_CHECK(glDeleteProgram(shader.program));
	GL_CHECK(glDeleteTextures(1, &this->texture));
	GL_CHECK(glDeleteBuffers(1, &this->vbo));
	GL_CHECK(glDeleteVertexArrays(1, &this->vao));
}

/// <summary>
/// Init OpenGL
/// </summary>
void BackendOpenGL::InitGL()
{
	//create shader
	shader.program = this->sm->BuildFromSources(shader.vSource, shader.pSource);
	
	    
    this->sm->GetAttributtesUniforms();
    

	//create VBO
	GL_CHECK(glGenBuffers(1, &this->vbo));

	//create VAO
	this->InitVAO();
	
}

void BackendOpenGL::InitTexture(const char* uniformName)
{
	if (uniformName == nullptr)
	{
		return;
	}

	//get location of data in shader
	GLint fontTexLoc = 0;
	GL_CHECK(fontTexLoc = glGetUniformLocation(shader.program, uniformName));
#ifdef glProgramUniform1i
	GL_CHECK(glProgramUniform1i(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.
#elif glProgramUniform1iEXT
	GL_CHECK(glProgramUniform1iEXT(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.
#endif

	auto fb = mainRenderer->GetFontBuilder();

	int w = fb->GetTextureWidth();
	int h = fb->GetTextureHeight();

	this->tW = 1.0f / static_cast<float>(w);  //1.0 / pixel size in width
	this->tH = 1.0f / static_cast<float>(h); //1.0 / pixel size in height

	if (this->texture != 0)
	{
		FONT_UNBIND_TEXTURE_2D;
		GL_CHECK(glDeleteTextures(1, &this->texture));
	}

	//create texture
	GL_CHECK(glGenTextures(1, &this->texture));
	FONT_BIND_TEXTURE_2D(this->texture);
	
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, TEXTURE_SINGLE_CHANNEL,
		w, h, 0,
		TEXTURE_SINGLE_CHANNEL, GL_UNSIGNED_BYTE, nullptr));

	if (this->rs.useTextureLinearFilter)
	{
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}
	GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

/// <summary>
/// Create VAO
/// </summary>
void BackendOpenGL::InitVAO()
{
#ifdef __ANDROID_API__
	if (glVersion == 2)
	{
		return;
	}
#endif

	//init
	GL_CHECK(glGenVertexArrays(1, &this->vao));

	//bind data to it	
	FONT_BIND_ARRAY_BUFFER(this->vbo);

	FONT_BIND_VAO(this->vao);

	this->sm->BindVertexAtribs();

	FONT_UNBIND_ARRAY_BUFFER;

	FONT_UNBIND_VAO;
}



void BackendOpenGL::SetFontTextureLinearFiler(bool val)
{
	this->rs.useTextureLinearFilter = val;	

	FONT_BIND_TEXTURE_2D(this->texture);

	if (this->rs.useTextureLinearFilter)
	{
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	else
	{
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	}

	FONT_UNBIND_TEXTURE_2D;
}

void BackendOpenGL::SetBackground(std::optional<BackgroundSettings> bs)
{
	if (bs)
	{
		this->background = std::make_unique<BackendBackgroundOpenGL>(*bs, rs, glVersion);
	}
	else 	
	{
		this->background = nullptr;
	}
}

void BackendOpenGL::SetMainRenderer(AbstractRenderer* mainRenderer)
{
	BackendBase::SetMainRenderer(mainRenderer);
	this->InitTexture("fontText");
}

void BackendOpenGL::OnCanvasChanges()
{
}

std::shared_ptr<IShaderManager> BackendOpenGL::GetShaderManager() const
{
	return this->sm;
}


/// <summary>
/// Render all fonts
/// </summary>
void BackendOpenGL::Render()
{
	this->Render(nullptr, nullptr);
}

/// <summary>
/// Render all fonts with pre and post render callbacks
/// </summary>
/// <param name="preDrawCallback"></param>
/// <param name="postDrawCallback"></param>
void BackendOpenGL::Render(std::function<void(GLuint)> preDrawCallback,
	std::function<void()> postDrawCallback)
{
	if (this->enabled == false)
	{
		return;
	}

#ifdef THREAD_SAFETY
	std::shared_lock<std::shared_timed_mutex> lk(mainRenderer->m);
#endif

	bool vboChanged = this->mainRenderer->GenerateGeometry();

	if (this->geom.empty())
	{
		return;
	}

	if (this->background)
	{
		this->background->Render();
	}
    
	//activate texture
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	FONT_BIND_TEXTURE_2D(this->texture);


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

    if (preDrawCallback != nullptr)
    {
        preDrawCallback(shader.program);
    }
    
	this->sm->Render(this->quadsCount);
	
	if (postDrawCallback != nullptr)
	{
		postDrawCallback();
	}

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


/// <summary>
/// Fill texture from font builder to OpenGL texture
/// so that it can be used in shader
/// </summary>
void BackendOpenGL::FillFontTexture()
{
	auto fb = mainRenderer->GetFontBuilder();

	FONT_BIND_TEXTURE_2D(this->texture);

	GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,
		fb->GetTextureWidth(), fb->GetTextureHeight(),
		TEXTURE_SINGLE_CHANNEL, GL_UNSIGNED_BYTE, fb->GetTextureData()));

	FONT_UNBIND_TEXTURE_2D;
}

/// <summary>
/// Add single "letter" quad to geom buffer
/// </summary>
/// <param name="gi"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void BackendOpenGL::AddQuad(AbstractRenderer::Vertex& vmin, AbstractRenderer::Vertex& vmax, const AbstractRenderer::RenderParams& rp)
{
	vmin.x *= psW;
	vmin.y *= psH;
	vmin.u *= this->tW;
	vmin.v *= this->tH;

	vmax.x *= psW;
	vmax.y *= psH;
	vmax.u *= this->tW;
	vmax.v *= this->tH;

    this->sm->FillQuadVertexData(vmin, vmax, rp, this->geom);
    
	
	if (this->background)
	{
		this->background->AddQuad(vmin, vmax, rp);
	}
	

	this->quadsCount++;
}

void BackendOpenGL::Clear()
{
	BackendBase::Clear();

	if (this->background)
	{
		this->background->Clear();
	}
}

void BackendOpenGL::OnFinishQuadGroup(const AbstractRenderer::RenderParams& rp)
{
	if (this->background)
	{
		this->background->OnFinishQuadGroup(rp);
	}
}

void BackendOpenGL::FillGeometry()
{
    if (this->geom.empty())
    {
        return;
    }
    
	FONT_BIND_ARRAY_BUFFER(this->vbo);
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, 
		this->geom.size() * sizeof(float),
		this->geom.data(),
		GL_STREAM_DRAW));
	FONT_UNBIND_ARRAY_BUFFER;

	if (this->background)
	{
		this->background->FillGeometry();
	}
}
