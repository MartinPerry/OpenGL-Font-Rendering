#include "./AbstractRenderer.h"

#include <limits>

#include "./FontBuilder.h"


//=============================================================================
// GL helpers
//=============================================================================

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();

	if (err != GL_NO_ERROR)
	{
		std::string error = "";

		switch (err)
		{
		case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		default: error = "Unknown"; break;
		}
		error += " (";
		error += std::to_string(err);
		error += ") ";

		printf("OpenGL error %s, at %s:%i - for %s\n", error.c_str(), fname, line, stmt);
		//abort();
	}
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0);
#else
#define GL_CHECK(stmt) stmt
#endif

#ifdef _DEBUG
#define NV_REPORT_COMPILE_ERRORS
#endif

//=============================================================================
// Shaders
//=============================================================================



const char* AbstractRenderer::Shader::vSource = {
	"\n\
    attribute vec2 POSITION;\n\
    attribute vec2 TEXCOORD0;\n\
	attribute vec4 COLOR;\n\
    varying vec2 texCoord;\n\
	varying vec4 color;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
		gl_Position.xy = 2.0 * gl_Position.xy - 1.0; \n\
		gl_Position.y *= -1.0;\n\
        texCoord = TEXCOORD0; \n\
		color = COLOR; \n\
    }\n\
" };

const char* AbstractRenderer::Shader::pSource = {
	"\n\
    uniform sampler2D fontTex;\n\
    varying vec2 texCoord;\n\
	varying vec4 color;\n\
    //out vec4 fragColor;\n\
	\n\
    void main()\n\
    {\n\
        float distance = texture2D( fontTex, texCoord.xy ).x; \n\
        gl_FragColor.rgb = color.xyz; \n\
		//gl_FragColor.rgb = vec3(distance); \n\
        gl_FragColor.a = color.w * distance;\n\
		//gl_FragColor.a = 1;\n\
    }\n\
" };
//=============================================================================

const AbstractRenderer::Color AbstractRenderer::DEFAULT_COLOR = { 1,1,1,1 };

AbstractRenderer::AbstractRenderer(int deviceW, int deviceH, Font f)
	: deviceW(deviceW), deviceH(deviceH), strChanged(false)
{

	this->fb = new FontBuilder(f);
	this->fb->SetGridPacking(f.size, f.size);

	ci.mark = u8"\u2022";
	ci.offset = this->fb->GetFontInfo().newLineOffset / 2;
	ci.offset = static_cast<int>(ci.offset * 1.2); //add extra 20%
	
	this->InitGL();
}

AbstractRenderer::~AbstractRenderer()
{
	
	SAFE_DELETE(this->fb);

	//this will end in error, if OpenGL is not initialized during
	//destruction
	//however, that should be OK

	(glUseProgram(0));
	(glBindTexture(GL_TEXTURE_2D, 0));
	(glBindBuffer(GL_ARRAY_BUFFER, 0));
	(glBindVertexArray(0));

	(glDeleteProgram(shader.program));
	(glDeleteTextures(1, &this->fontTex));
	(glDeleteBuffers(1, &this->vbo));
	(glDeleteVertexArrays(1, &this->vao));

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

	//get location of data in shader

	//GL_CHECK(shader.colorLocation = glGetUniformLocation(shader.program, "color"));

	GLint fontTexLoc = 0;
	GL_CHECK(fontTexLoc = glGetUniformLocation(shader.program, "fontTex"));
#ifdef glProgramUniform1i
	GL_CHECK(glProgramUniform1i(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.
#elif glProgramUniform1iEXT
	GL_CHECK(glProgramUniform1iEXT(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.
#endif

	GL_CHECK(shader.positionLocation = glGetAttribLocation(shader.program, "POSITION"));
	GL_CHECK(shader.texCoordLocation = glGetAttribLocation(shader.program, "TEXCOORD0"));
	GL_CHECK(shader.colorLocation = glGetAttribLocation(shader.program, "COLOR"));


	//create VBO
	GL_CHECK(glGenBuffers(1, &this->vbo));
		
	//create VAO
	this->CreateVAO();



	//create texture
	GL_CHECK(glGenTextures(1, &this->fontTex));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->fontTex));

	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
		this->fb->GetTextureWidth(), this->fb->GetTextureHeight(), 0,
		GL_RED, GL_UNSIGNED_BYTE, nullptr));
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
	const size_t VERTEX_SIZE = sizeof(Vertex);
	const size_t POSITION_OFFSET = 0 * sizeof(float);
	const size_t TEX_COORD_OFFSET = 2 * sizeof(float);
	const size_t COLOR_OFFSET = 4 * sizeof(float);

	//init
	GL_CHECK(glGenVertexArrays(1, &this->vao));

	//bind data to it
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CHECK(glBindVertexArray(this->vao));

	GL_CHECK(glEnableVertexAttribArray(this->shader.positionLocation));
	GL_CHECK(glVertexAttribPointer(this->shader.positionLocation, 2,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(POSITION_OFFSET)));

	GL_CHECK(glEnableVertexAttribArray(this->shader.texCoordLocation));
	GL_CHECK(glVertexAttribPointer(this->shader.texCoordLocation, 2,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(TEX_COORD_OFFSET)));

	GL_CHECK(glEnableVertexAttribArray(this->shader.colorLocation));
	GL_CHECK(glVertexAttribPointer(this->shader.colorLocation, 4,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(COLOR_OFFSET)));


	GL_CHECK(glBindVertexArray(0));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

	
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
		fprintf(stderr, "Compile failed:\n%s\n", temp);
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
		fprintf(stderr, "Link failed:\n%s\n", infoLog);
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


void AbstractRenderer::SetCanvasSize(int w, int h)
{
	this->deviceW = w;
	this->deviceH = h;

	this->strChanged = true;
}


FontBuilder * AbstractRenderer::GetFontBuilder()
{
	return this->fb;
}


/// <summary>
/// Render all fonts
/// </summary>
void AbstractRenderer::Render()
{
	bool vboChanged = this->GenerateGeometry();

	if (geom.empty())
	{
		return;
	}
		
	//activate texture
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, fontTex));
	
	//activate shader
	GL_CHECK(glUseProgram(shader.program));
	//GL_CHECK(glUniform4f(shader.colorLocation, 0, 0, 1, 1));

	//render
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CHECK(glBindVertexArray(this->vao));

	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, geom.size() * 6));

	GL_CHECK(glBindVertexArray(0));

	//deactivate shader
	GL_CHECK(glUseProgram(0));




}

void AbstractRenderer::FillTexture()
{
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->fontTex));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,
		this->fb->GetTextureWidth(), this->fb->GetTextureHeight(),
		GL_RED, GL_UNSIGNED_BYTE, this->fb->GetTexture()));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void AbstractRenderer::FillVB()
{
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, this->geom.size() * sizeof(LetterGeom), &(this->geom.front()), GL_STREAM_DRAW));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}