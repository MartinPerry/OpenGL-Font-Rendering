#include "./AbstractRenderer.h"

#include <limits>
#include <algorithm>

#include "./Shaders.h"
#include "./FontBuilder.h"
#include "./FontShaderManager.h"

//=============================================================================
// GL helpers
//=============================================================================


#if defined(_DEBUG) || defined(DEBUG)
#define NV_REPORT_COMPILE_ERRORS
#endif

//=============================================================================

const AbstractRenderer::Color AbstractRenderer::DEFAULT_COLOR = { 1,1,1,1 };

std::vector<std::string> AbstractRenderer::GetFontsInDirectory(const std::string & fontDir)
{
	std::vector<std::string> t;

	DIR * dir = opendir(fontDir.c_str());

	if (dir == nullptr)
	{
		printf("Failed to open dir %s\n", fontDir.c_str());
		return t;
	}

	struct dirent * ent;
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


			t.push_back(fullPath);
		}
	}


	closedir(dir);

	return t;
}


AbstractRenderer::AbstractRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion)
	: AbstractRenderer(fs, r, glVersion,
                       DEFAULT_VERTEX_SHADER_SOURCE, DEFAULT_PIXEL_SHADER_SOURCE,
                       std::make_shared<DefaultFontShaderManager>())
{
}

AbstractRenderer::AbstractRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion,
                 const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm)
    : rs(r), 
	renderEnabled(true), 
	glVersion(glVersion), 
	sm(sm), 
	ci({ UTF8_TEXT(u8""), 0}),
	axisYOrigin(AxisYOrigin::TOP), 
	quadsCount(0), 
	strChanged(false),
	vbo(0),
	vao(0),
	fontTex(0)
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
    
    this->fb = new FontBuilder(fs, r);
    
	int ps = this->fb->GetMaxEmSize();// this->fb->GetMaxFontPixelHeight();
    this->fb->SetGridPacking(ps, ps);
        
    this->SetCaption(UTF8_TEXT(u8"\u2022"), 10);
         
    this->InitGL();

	this->psW = 1.0f / static_cast<float>(rs.deviceW); //pixel size in width
	this->psH = 1.0f / static_cast<float>(rs.deviceH); //pixel size in height

	this->tW = 1.0f / static_cast<float>(this->fb->GetTextureWidth());  //pixel size in width
	this->tH = 1.0f / static_cast<float>(this->fb->GetTextureHeight()); //pixel size in height

}

AbstractRenderer::~AbstractRenderer()
{

	SAFE_DELETE(this->fb);

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
void AbstractRenderer::InitGL()
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



	//create texture
	GL_CHECK(glGenTextures(1, &this->fontTex));
	FONT_BIND_TEXTURE_2D(this->fontTex);

	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, TEXTURE_SINGLE_CHANNEL,
		this->fb->GetTextureWidth(), this->fb->GetTextureHeight(), 0,
		TEXTURE_SINGLE_CHANNEL, GL_UNSIGNED_BYTE, nullptr));

	GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}


/// <summary>
/// Create VAO
/// </summary>
void AbstractRenderer::CreateVAO()
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
GLuint AbstractRenderer::CompileGLSLShader(GLenum target, const char* shader)
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
GLuint AbstractRenderer::LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader)
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

void AbstractRenderer::SetCaption(const UnicodeString & mark, int offsetInPixels)
{
	ci.mark = mark;	
	this->SetCaptionOffset(offsetInPixels);
}

void AbstractRenderer::SetCaptionOffset(int offsetInPixels)
{
	//take half of new line offset and add extra 20%
	ci.offset = offsetInPixels;// static_cast<int>(this->fb->GetNewLineOffsetBasedOnGlyph(ci.mark[0]) * 0.5 * 1.2);	
}

void AbstractRenderer::SetCanvasSize(int w, int h)
{
	this->rs.deviceW = w;
	this->rs.deviceH = h;

	this->strChanged = true;
}

void AbstractRenderer::SwapCanvasWidthHeight()
{
	std::swap(this->rs.deviceW, this->rs.deviceH);

	this->strChanged = true;
}

void AbstractRenderer::SetAxisYOrigin(AxisYOrigin axisY)
{
	this->axisYOrigin = axisY;
}

void AbstractRenderer::SetEnabled(bool val)
{
	this->renderEnabled = val;
}

bool AbstractRenderer::IsEnabled() const
{
	return this->renderEnabled;
}

FontBuilder * AbstractRenderer::GetFontBuilder()
{
	return this->fb;
}

int AbstractRenderer::GetCanvasWidth() const
{
	return this->rs.deviceW;
}

int AbstractRenderer::GetCanvasHeight() const
{
	return this->rs.deviceH;
}

std::shared_ptr<IFontShaderManager> AbstractRenderer::GetShaderManager() const
{
	return this->sm;
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
    this->Render(nullptr);
}

void AbstractRenderer::Render(std::function<void(GLuint)> preDrawCallback)
{
	if (this->renderEnabled == false)
	{
		return;
	}

#ifdef THREAD_SAFETY
	std::shared_lock<std::shared_timed_mutex> lk(m);
#endif

	bool vboChanged = this->GenerateGeometry();

	if (geom.empty())
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
    
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, quadsCount * 6));

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



void AbstractRenderer::FillTexture()
{
	FONT_BIND_TEXTURE_2D(this->fontTex);

	GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,
		this->fb->GetTextureWidth(), this->fb->GetTextureHeight(),
		TEXTURE_SINGLE_CHANNEL, GL_UNSIGNED_BYTE, this->fb->GetTexture()));

	FONT_UNBIND_TEXTURE_2D;
}

/// <summary>
/// Add single "letter" quad to geom buffer
/// </summary>
/// <param name="gi"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void AbstractRenderer::AddQuad(const GlyphInfo & gi, int x, int y, const Color & col)
{    
    int fx = x + gi.bmpX;
    int fy = y - gi.bmpY;
    	
    //build geometry
    Vertex min, max;
    
    min.x = static_cast<float>(fx) * psW;
    min.y = static_cast<float>(fy) * psH;
    min.u = static_cast<float>(gi.tx) * tW;
    min.v = static_cast<float>(gi.ty) * tH;
    
    max.x = static_cast<float>(fx + gi.bmpW) * psW;
    max.y = static_cast<float>(fy + gi.bmpH) * psH;
    max.u = static_cast<float>(gi.tx + gi.bmpW) * tW;
    max.v = static_cast<float>(gi.ty + gi.bmpH) * tH;
    
    this->sm->FillVertexData(min, max, col, this->geom);
    
    this->quadsCount++;
}


void AbstractRenderer::FillVB()
{
    if (this->geom.empty())
    {
        return;
    }
    
	FONT_BIND_ARRAY_BUFFER(this->vbo);
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, this->geom.size() * sizeof(float), this->geom.data(), GL_STREAM_DRAW));
	FONT_UNBIND_ARRAY_BUFFER;
}
