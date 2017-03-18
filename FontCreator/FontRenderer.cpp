#include "./FontRenderer.h"

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
		abort();
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



const char* FontRenderer::Shader::vSource = {
	"#version 140\n\
    \n\
    in vec2 POSITION;\n\
    in vec2 TEXCOORD0;\n\
    out vec2 texCoord;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
		gl_Position.xy = 2.0 * gl_Position.xy - 1.0; \n\
gl_Position.y *= -1;\n\
        texCoord = TEXCOORD0.xy; \n\
    }\n\
" };

const char* FontRenderer::Shader::pSource = {
	"#version 140\n\
    uniform vec4 color; \n\
    uniform sampler2D fontTex;\n\
    in vec2 texCoord;\n\
    out vec4 fragColor;\n\
	\n\
    void main()\n\
    {\n\
        float distance = (texture2D( fontTex, texCoord.xy ).x); \n\
        fragColor.rgb = color.rgb; \n\
		fragColor.rgb = vec3(distance); \n\
        fragColor.a = color.a * distance;\n\
		fragColor.a = 1;\n\
    }\n\
" };

//=============================================================================

FontRenderer::FontRenderer(int deviceW, int deviceH, Font f)
	: deviceW(deviceW), deviceH(deviceH), strChanged(false)
{
	this->fb = new FontBuilder(f.name, f.textureWidth, f.textureHeight, f.size);
	this->fb->SetGridPacking(f.size, f.size);

	this->InitGL();
}

FontRenderer::~FontRenderer()
{
}

/// <summary>
/// Init OpenGL
/// </summary>
void FontRenderer::InitGL()
{

	//create shader
	GLuint vShader = CompileGLSLShader(GL_VERTEX_SHADER, shader.vSource);
	GLuint pShader = CompileGLSLShader(GL_FRAGMENT_SHADER, shader.pSource);

	GL_CHECK(shader.program = LinkGLSLProgram(vShader, pShader));

	GL_CHECK(glDeleteShader(vShader));
	GL_CHECK(glDeleteShader(pShader));

	//get location of data in shader

	GL_CHECK(shader.colorLocation = glGetUniformLocation(shader.program, "color"));

	GLint fontTexLoc = 0;
	GL_CHECK(fontTexLoc = glGetUniformLocation(shader.program, "fontTex"));
	GL_CHECK(glProgramUniform1i(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.


	GL_CHECK(shader.positionLocation = glGetAttribLocation(shader.program, "POSITION"));
	GL_CHECK(shader.texCoordLocation = glGetAttribLocation(shader.program, "TEXCOORD0"));


	//create VBO
	GL_CHECK(glGenBuffers(1, &this->vbo));
	
	/*
	Vertex a, b, c, d;
	a.x = 0;
	a.y = 0;
	a.u = 0;
	a.v = 0;

	b.x = 1;
	b.y = 0;
	b.u = 1;
	b.v = 0;

	c.x = 1;
	c.y = 1;
	c.u = 1;
	c.v = 1;

	d.x = 0;
	d.y = 1;
	d.u = 0;
	d.v = 1;

	LetterGeom l;
	l.AddQuad(a, b, c, d);
	this->geom.push_back(l);

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, geom.size() * sizeof(LetterGeom), &(geom.front()), GL_STREAM_DRAW));
	*/


	//create VAO
	this->CreateVAO();

}

/// <summary>
/// Create VAO
/// </summary>
void FontRenderer::CreateVAO()
{
	//init
	GL_CHECK(glGenVertexArrays(1, &this->vao));

	//bind data to it
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CHECK(glBindVertexArray(this->vao));

	GL_CHECK(glEnableVertexAttribArray(this->shader.positionLocation));
	GL_CHECK(glVertexAttribPointer(this->shader.positionLocation, 2,
		GL_FLOAT, GL_FALSE,
		4 * sizeof(float), (void*)(0)));

	GL_CHECK(glEnableVertexAttribArray(this->shader.texCoordLocation));
	GL_CHECK(glVertexAttribPointer(this->shader.texCoordLocation, 2,
		GL_FLOAT, GL_FALSE,
		4 * sizeof(float), (void*)(2 * sizeof(float))));

	GL_CHECK(glBindVertexArray(0));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

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
/// Compile input shader
/// </summary>
/// <param name="target"></param>
/// <param name="shader"></param>
/// <returns></returns>
GLuint FontRenderer::CompileGLSLShader(GLenum target, const char* shader)
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
GLuint FontRenderer::LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader)
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





/// <summary>
/// Render all fonts
/// </summary>
void FontRenderer::Render()
{
	bool vboChanged = this->GenerateStringGeometry();

	if (geom.empty())
	{
		return;
	}

	//bs.setStates();
	
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, fontTex));
	//glTextureBuffer(m_texQuads, GL_RGBA32F, m_vbo);


	//activate shader
	GL_CHECK(glUseProgram(shader.program));
	GL_CHECK(glUniform4f(shader.colorLocation, 1, 1, 0, 0));

	//render
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CHECK(glBindVertexArray(this->vao));

	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, geom.size() * 6));

	GL_CHECK(glBindVertexArray(0));

	//deactivate shader
	GL_CHECK(glUseProgram(0));




}

/// <summary>
/// Add new string to be rendered
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void FontRenderer::AddString(const utf8_string & strUTF8, int x, int y)
{
	this->strChanged = true;
	FontRenderer::StringInfo i;
	i.strUTF8 = strUTF8;
	i.x = x;
	i.y = y;

	this->strs.push_back(i);

	this->fb->AddString(strUTF8);
}

/// <summary>
/// Linear mapping of value from - to
/// </summary>
/// <param name="fromMin"></param>
/// <param name="fromMax"></param>
/// <param name="toMin"></param>
/// <param name="toMax"></param>
/// <param name="s"></param>
/// <returns></returns>
float FontRenderer::MapRange(float fromMin, float fromMax, float toMin, float toMax, float s)
{
	return toMin + (s - fromMin) * (toMax - toMin) / (fromMax - fromMin);
}

/// <summary>
/// Generate geometry for all input strings
/// </summary>
/// <returns></returns>
bool FontRenderer::GenerateStringGeometry()
{
	if (this->strChanged == false)
	{
		return false;
	}

	this->fb->CreateFontAtlas();
	this->fb->Save("gl.png");

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->fontTex));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,
		this->fb->GetTextureWidth(), this->fb->GetTextureHeight(),
		GL_RED, GL_UNSIGNED_BYTE, this->fb->GetTexture()));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	

	const FontInfo & fi = this->fb->GetFontInfo();

	float psW = 1.0f / static_cast<float>(deviceW);	//pixel size in width
	float psH = 1.0f / static_cast<float>(deviceH); //pixel size in height

	float tW = 1.0f / static_cast<float>(this->fb->GetTextureWidth());	//pixel size in width
	float tH = 1.0f / static_cast<float>(this->fb->GetTextureHeight()); //pixel size in height

	this->geom.clear();

	for (auto si : this->strs)
	{
		
		int x = si.x;
		int y = si.y;

		for (auto c : si.strUTF8)
		{
			if (c == '\n')
			{
				//to do
				x = si.x;
				y += fi.newLineOffset; // ?

				continue;
			}

			auto it = fi.usedGlyphs.find(c);
			if (it == fi.usedGlyphs.end())
			{
				continue;
			}

			GlyphInfo & gi = *it->second;

			int fx = x + gi.bmpX;
			int fy = y - gi.bmpY;

			//build geometry		
			Vertex a, b, c, d;
			a.x = fx;
			a.y = fy;
			a.u = gi.tx;
			a.v = gi.ty;

			b.x = fx + gi.bmpW;
			b.y = fy;
			b.u = gi.tx + gi.bmpW;
			b.v = gi.ty;

			c.x = fx + gi.bmpW;
			c.y = fy + gi.bmpH;
			c.u = gi.tx + gi.bmpW;
			c.v = gi.ty + gi.bmpH;

			d.x = fx;
			d.y = fy + gi.bmpH;
			d.u = gi.tx;
			d.v = gi.ty + gi.bmpH;

			a.Mul(psW, psH, tW, tH);
			b.Mul(psW, psH, tW, tH);
			c.Mul(psW, psH, tW, tH);
			d.Mul(psW, psH, tW, tH);

			LetterGeom l;
			l.AddQuad(a, b, c, d);
			this->geom.push_back(l);

			x += (gi.adv >> 6);
		}
	}

	this->strChanged = false;

	/*
	this->geom.clear();

	Vertex a, b, c, d;
	a.x = 0;
	a.y = 0;
	a.u = 0;
	a.v = 0;

	b.x = 1;
	b.y = 0;
	b.u = 1;
	b.v = 0;

	c.x = 1;
	c.y = 1;
	c.u = 1;
	c.v = 1;

	d.x = 0;
	d.y = 1;
	d.u = 0;
	d.v = 1;

	LetterGeom l;
	l.AddQuad(a, b, c, d);
	this->geom.push_back(l);
	*/
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, geom.size() * sizeof(LetterGeom), &(geom.front()), GL_STREAM_DRAW));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	

	return true;
}