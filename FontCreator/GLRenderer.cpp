#include "./GLRenderer.h"

#include <limits>
#include <algorithm>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"

//=============================================================================
// GL helpers
//=============================================================================


#if defined(_DEBUG) || defined(DEBUG)
#	define NV_REPORT_COMPILE_ERRORS
#endif

//=============================================================================


GLRenderer::GLRenderer(const RenderSettings& r, int glVersion) :
	GLRenderer(r, glVersion,
                       DEFAULT_VERTEX_SHADER_SOURCE, DEFAULT_PIXEL_SHADER_SOURCE,
                       std::make_shared<DefaultFontShaderManager>())
{
}

GLRenderer::GLRenderer(const RenderSettings& r, int glVersion,
	const char* vSource, const char* pSource, std::shared_ptr<IFontShaderManager> sm) :
	mainRenderer(nullptr),
	rs(r),
	glVersion(glVersion),
	sm(sm),
	enabled(true),
	vbo(0),
	vao(0),
	fontTex(0)
{
	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

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


GLRenderer::~GLRenderer()
{
		
	//this will end in error, if OpenGL is not initialized during
	//destruction
	//however, that should be OK

	FONT_UNBIND_SHADER;
	FONT_UNBIND_TEXTURE_2D;
	FONT_UNBIND_ARRAY_BUFFER;
	FONT_UNBIND_VAO;

	GL_CHECK(glDeleteProgram(shader.program));
	GL_CHECK(glDeleteTextures(1, &this->fontTex));
	GL_CHECK(glDeleteBuffers(1, &this->vbo));
	GL_CHECK(glDeleteVertexArrays(1, &this->vao));
}

/// <summary>
/// Init OpenGL
/// </summary>
void GLRenderer::InitGL()
{

	//create shader
	GLuint vShader = CompileGLSLShader(GL_VERTEX_SHADER, shader.vSource);
	GLuint pShader = CompileGLSLShader(GL_FRAGMENT_SHADER, shader.pSource);

	GL_CHECK(shader.program = LinkGLSLProgram(vShader, pShader));

	GL_CHECK(glDeleteShader(vShader));
	GL_CHECK(glDeleteShader(pShader));

    this->sm->SetShaderProgram(shader.program);
    
	//get location of data in shader
    GLint fontTexLoc = 0;
    GL_CHECK(fontTexLoc = glGetUniformLocation(shader.program, "fontTex"));
#ifdef glProgramUniform1i
    GL_CHECK(glProgramUniform1i(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.
#elif glProgramUniform1iEXT
    GL_CHECK(glProgramUniform1iEXT(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.
#endif
    
    this->sm->GetAttributtesUniforms();
    

	//create VBO
	GL_CHECK(glGenBuffers(1, &this->vbo));

	//create VAO
	this->CreateVAO();
	
}

void GLRenderer::CreateTexture()
{
	if (this->fontTex != 0)
	{
		FONT_UNBIND_TEXTURE_2D;
		GL_CHECK(glDeleteTextures(1, &this->fontTex));
	}

	//create texture
	GL_CHECK(glGenTextures(1, &this->fontTex));
	FONT_BIND_TEXTURE_2D(this->fontTex);

	auto fb = mainRenderer->GetFontBuilder();

	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, TEXTURE_SINGLE_CHANNEL,
		fb->GetTextureWidth(), fb->GetTextureHeight(), 0,
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
void GLRenderer::CreateVAO()
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

/// <summary>
/// Compile input shader
/// </summary>
/// <param name="target"></param>
/// <param name="shader"></param>
/// <returns></returns>
GLuint GLRenderer::CompileGLSLShader(GLenum target, const char* shader)
{
	GLuint object;

	GL_CHECK(object = glCreateShader(target));

	if (!object)
		return object;

	GL_CHECK(glShaderSource(object, 1, &shader, NULL));

	GL_CHECK(glCompileShader(object));

	// check if shader compiled
	GLint compiled = 0;
	GL_CHECK(glGetShaderiv(object, GL_COMPILE_STATUS, &compiled));

	if (!compiled)
	{
#ifdef NV_REPORT_COMPILE_ERRORS
		char temp[256] = "";
		GL_CHECK(glGetShaderInfoLog(object, 256, NULL, temp));
		MY_LOG_ERROR("Font renderer - Shader compile failed:\n%s", temp);
#endif
		GL_CHECK(glDeleteShader(object));
		return 0;
	}

	return object;
}

/// <summary>
/// Create a program composed of vertex and fragment shaders.
/// </summary>
/// <param name="vertexShader"></param>
/// <param name="fragmentShader"></param>
/// <returns></returns>
GLuint GLRenderer::LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program = 0;
	GL_CHECK(program = glCreateProgram());
	GL_CHECK(glAttachShader(program, vertexShader));
	GL_CHECK(glAttachShader(program, fragmentShader));
	GL_CHECK(glLinkProgram(program));

#ifdef NV_REPORT_COMPILE_ERRORS
	// Get error log.
	GLint charsWritten, infoLogLength;
	GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength));

	char * infoLog = new char[infoLogLength];
	GL_CHECK(glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog));
	if ((infoLogLength > 0) && (infoLog[0] != '\0'))
	{
		MY_LOG_ERROR("Link failed:\n%s", infoLog);
	}
	delete[] infoLog;
#endif

	// Test linker result.
	GLint linkSucceed = GL_FALSE;
	GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &linkSucceed));

	if (linkSucceed == GL_FALSE)
	{
		GL_CHECK(glDeleteProgram(program));
		return 0;
	}

	return program;
}


void GLRenderer::SetMainRenderer(AbstractRenderer* mainRenderer)
{
	this->mainRenderer = mainRenderer;
	this->CreateTexture();
}

void GLRenderer::SetFontTextureLinearFiler(bool val)
{
	this->rs.useTextureLinearFilter = val;	

	FONT_BIND_TEXTURE_2D(this->fontTex);

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


const RenderSettings& GLRenderer::GetSettings() const
{
	return this->rs;
}

std::shared_ptr<IFontShaderManager> GLRenderer::GetShaderManager() const
{
	return this->sm;
}


void GLRenderer::SetCanvasSize(int w, int h)
{
	this->rs.deviceW = w;
	this->rs.deviceH = h;

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

}

void GLRenderer::SwapCanvasWidthHeight()
{
	std::swap(this->rs.deviceW, this->rs.deviceH);

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

}

/// <summary>
/// Render all fonts
/// </summary>
void GLRenderer::Render()
{
	this->Render(nullptr, nullptr);
}

void GLRenderer::Render(std::function<void(GLuint)> preDrawCallback, 
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

	if (mainRenderer->geom.empty())
	{
		return;
	}

    
	//activate texture
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	FONT_BIND_TEXTURE_2D(this->fontTex);


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
    
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, mainRenderer->quadsCount * 6));

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



void GLRenderer::FillTexture()
{
	auto fb = mainRenderer->GetFontBuilder();

	FONT_BIND_TEXTURE_2D(this->fontTex);

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
void GLRenderer::AddQuad(const GlyphInfo & gi, float x, float y, const AbstractRenderer::RenderParams & rp)
{    
    float fx = x + gi.bmpX * rp.scale;
	float fy = y - gi.bmpY * rp.scale;
    	
    //build geometry
	AbstractRenderer::Vertex min, max;
    
    min.x = fx * psW;
    min.y = fy * psH;
    min.u = static_cast<float>(gi.tx) * mainRenderer->tW;
    min.v = static_cast<float>(gi.ty) * mainRenderer->tH;
    
    max.x = (fx + gi.bmpW * rp.scale) * psW;
    max.y = (fy + gi.bmpH * rp.scale) * psH;
    max.u = static_cast<float>(gi.tx + gi.bmpW) * mainRenderer->tW;
    max.v = static_cast<float>(gi.ty + gi.bmpH) * mainRenderer->tH;
    
    this->sm->FillVertexData(min, max, rp, mainRenderer->geom);
    
	mainRenderer->quadsCount++;
}


void GLRenderer::FillVB()
{
    if (mainRenderer->geom.empty())
    {
        return;
    }
    
	FONT_BIND_ARRAY_BUFFER(this->vbo);
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, 
		mainRenderer->geom.size() * sizeof(float), 
		mainRenderer->geom.data(), 
		GL_STREAM_DRAW));
	FONT_UNBIND_ARRAY_BUFFER;
}
