#include "./FontRenderer.h"

#include <limits>

#include "./FontBuilder.h"
#include "./Macros.h"

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



const char* FontRenderer::Shader::vSource = {
	"#version 140\n\
    \n\
    in vec2 POSITION;\n\
    in vec2 TEXCOORD0;\n\
	in vec4 COLOR;\n\
    out vec2 texCoord;\n\
	out vec4 color;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
		gl_Position.xy = 2.0 * gl_Position.xy - 1.0; \n\
		gl_Position.y *= -1;\n\
        texCoord = TEXCOORD0; \n\
		color = COLOR; \n\
    }\n\
" };

const char* FontRenderer::Shader::pSource = {
	"#version 140\n\
    uniform sampler2D fontTex;\n\
    in vec2 texCoord;\n\
	in vec4 color;\n\
    out vec4 fragColor;\n\
	\n\
    void main()\n\
    {\n\
        float distance = texture2D( fontTex, texCoord.xy ).x; \n\
        fragColor.rgb = color.xyz; \n\
		//fragColor.rgb = vec3(distance); \n\
        fragColor.a = color.w * distance;\n\
		//fragColor.a = 1;\n\
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
void FontRenderer::InitGL()
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
	GL_CHECK(glProgramUniform1i(shader.program, fontTexLoc, 0)); //Texture unit 0 is for font Tex.


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
void FontRenderer::CreateVAO()
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

/// <summary>
/// Remove all added strings
/// </summary>
void FontRenderer::ClearStrings()
{
	this->strChanged = true;
	this->strs.clear();
}

/// <summary>
/// Add new string to be rendered - string coordinates
/// are in percents
/// If string already exist - do not add
/// Existing string => same x, y, align, anchor, content
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void FontRenderer::AddString(const utf8_string & strUTF8,
	double x, double y, Color color,
	TextAnchor anchor, TextAlign align)
{
	int xx = static_cast<int>(x * this->deviceW);
	int yy = static_cast<int>(y * this->deviceH);

	this->AddString(strUTF8, xx, yy, color, anchor, align);
}

/// <summary>
/// Add new string to be rendered
/// If string already exist - do not add
/// Existing string => same x, y, align, anchor, content
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void FontRenderer::AddString(const utf8_string & strUTF8, 
	int x, int y, Color color,
	TextAnchor anchor, TextAlign align)
{
	for (auto & s : this->strs)
	{
		if ((s.x == x) && (s.y == y) && 			
			(s.align == align) && (s.anchor == anchor))
		{
			if (s.strUTF8 == strUTF8)
			{
				//same string on the same position and with same align
				//already exist - do not add it again
				return;
			}
		}
	}

	//new string - add it

	this->strChanged = true;

	//fill basic structure info
	FontRenderer::StringInfo i;
	i.strUTF8 = strUTF8;
	i.x = x;
	i.y = y;
	i.color = color;
	i.anchor = anchor;
	i.align = align;
	i.anchorX = x;
	i.anchorY = y;
	i.linesCount = this->CalcStringLines(strUTF8);


	this->strs.push_back(i);

	this->fb->AddString(strUTF8);
}

/// <summary>
/// Calculate number of lines in input string
/// </summary>
/// <param name="strUTF8"></param>
/// <returns></returns>
int FontRenderer::CalcStringLines(const utf8_string & strUTF8) const
{
	int count = 1;
	for (auto c : strUTF8)
	{
		if (c == '\n')
		{
			count++;			
		}
	}
	return count;
}

/// <summary>
/// Calculate AABB of entire text and AABB for every line
/// </summary>
/// <param name="strUTF8"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="globalAABB"></param>
/// <returns></returns>
std::vector<FontRenderer::AABB> FontRenderer::CalcStringAABB(const utf8_string & strUTF8, 
	int x, int y, FontRenderer::AABB & globalAABB)
{

	FontRenderer::AABB aabb;
	aabb.minX = std::numeric_limits<int>::max();
	aabb.minY = std::numeric_limits<int>::max();

	aabb.maxX = std::numeric_limits<int>::min();
	aabb.maxY = std::numeric_limits<int>::min();

	FontRenderer::AABB lineAabb = aabb;
	std::vector<AABB> aabbs;

	const FontInfo & fi = this->fb->GetFontInfo();

	int startX = x;
	
	for (auto c : strUTF8)
	{
		if (c == '\n')
		{
			x = startX;
			y += fi.newLineOffset;

			aabbs.push_back(lineAabb);
			lineAabb = aabb;

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


		if (fx > lineAabb.maxX) lineAabb.maxX = fx;
		if (fy > lineAabb.maxY) lineAabb.maxY = fy;

		if (fx < lineAabb.minX) lineAabb.minX = fx;
		if (fy < lineAabb.minY) lineAabb.minY = fy;


		if (fx + gi.bmpW > lineAabb.maxX) lineAabb.maxX = fx + gi.bmpW;
		if (fy + gi.bmpH > lineAabb.maxY) lineAabb.maxY = fy + gi.bmpH;

		if (fx + gi.bmpW < lineAabb.minX) lineAabb.minX = fx + gi.bmpW;
		if (fy + gi.bmpH < lineAabb.minY) lineAabb.minY = fy + gi.bmpH;

		
		x += (gi.adv >> 6);
	}
	

	aabbs.push_back(lineAabb);
	globalAABB = aabb;

	for (auto & a : aabbs)
	{
		if (a.minX < globalAABB.minX) globalAABB.minX = a.minX;
		if (a.minY < globalAABB.minY) globalAABB.minY = a.minY;

		if (a.maxX > globalAABB.maxX) globalAABB.maxX = a.maxX;
		if (a.maxY > globalAABB.maxY) globalAABB.maxY = a.maxY;
	}


	return aabbs;
}

/// <summary>
/// Calculate start position of text using anchors
/// </summary>
void FontRenderer::CalcAnchoredPosition()
{	
	//Calculate anchored position of text
	const FontInfo & fi = this->fb->GetFontInfo();

	for (auto & si : this->strs)
	{		
		if (si.align == TextAlign::ALIGN_CENTER)
		{
			si.linesAABB = this->CalcStringAABB(si.strUTF8, si.x, si.y, si.aabb);
		}

		if (si.anchor == TextAnchor::LEFT_TOP)
		{
			si.anchorX = si.x;
			si.anchorY = si.y;
			si.anchorY += fi.newLineOffset; //y position is "line letter start" - move it to letter height
		}
		else if (si.anchor == TextAnchor::CENTER)
		{
			if (si.linesAABB.size() == 0)
			{
				si.linesAABB = this->CalcStringAABB(si.strUTF8, si.x, si.y, si.aabb);
			}

			si.anchorX = si.x - (si.aabb.maxX - si.aabb.minX) / 2;
			
			si.anchorY = si.y;
			si.anchorY += fi.newLineOffset; //move top position to TOP_LEFT
			si.anchorY -= (si.linesCount * fi.newLineOffset) / 2; //calc center from all lines and move TOP_LEFT down

		}
		else if (si.anchor == TextAnchor::LEFT_DOWN)
		{
			si.anchorX = si.x;			
			si.anchorY = si.y;
			si.anchorY -= (si.linesCount - 1) * fi.newLineOffset; //move down - default Y is at (TOP_LEFT - newLineOffset)
		}
	}
}

/// <summary>
/// Calc align of each line from string
/// </summary>
/// <param name="si"></param>
/// <param name="lineId"></param>
/// <param name="x"></param>
/// <param name="y"></param>
void FontRenderer::CalcLineAlign(const StringInfo & si, int lineId, int & x, int & y) const
{
	if (si.align == TextAlign::ALIGN_LEFT)
	{
		return;
	}

	if (si.align == TextAlign::ALIGN_CENTER)
	{
		int blockCenterX = (si.aabb.maxX - si.aabb.minX) / 2;
		//int blockCenterY = (si.aabb.maxY - si.aabb.minY) / 2;

		int lineCenterX = (si.linesAABB[lineId].maxX - si.linesAABB[lineId].minX) / 2;
		//int lineCenterY = (si.linesAABB[lineId].maxY - si.linesAABB[lineId].minY) / 2;

		x = x + (blockCenterX - lineCenterX);
		
	}
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

	//first we must build font atlas - it will load glyph infos
	this->fb->CreateFontAtlas();

	//calculate anchored position
	this->CalcAnchoredPosition();

	//DEBUG !!!
	//this->fb->Save("gl.png");
	//-------

	//Fill font texture
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->fontTex));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,
		this->fb->GetTextureWidth(), this->fb->GetTextureHeight(),
		GL_RED, GL_UNSIGNED_BYTE, this->fb->GetTexture()));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	
	//Build geometry
	const FontInfo & fi = this->fb->GetFontInfo();

	float psW = 1.0f / static_cast<float>(deviceW);	//pixel size in width
	float psH = 1.0f / static_cast<float>(deviceH); //pixel size in height

	float tW = 1.0f / static_cast<float>(this->fb->GetTextureWidth());	//pixel size in width
	float tH = 1.0f / static_cast<float>(this->fb->GetTextureHeight()); //pixel size in height

	this->geom.clear();
	
	
	for (auto si : this->strs)
	{
		int lineId = 0;
		int x = si.anchorX;
		int y = si.anchorY;
		
		int startX = x;
		int startY = y;

		this->CalcLineAlign(si, lineId, x, y);
		
		
		for (auto cc : si.strUTF8)
		{
			if (cc == '\n')
			{				
				x = startX;
				y += fi.newLineOffset;
				lineId++;

				this->CalcLineAlign(si, lineId, x, y);

				continue;
			}

			auto it = fi.usedGlyphs.find(cc);
			if (it == fi.usedGlyphs.end())
			{
				continue;
			}

			GlyphInfo & gi = *it->second;

			if (cc <= 32)
			{
				x += (gi.adv >> 6);
				continue;
			}

			
			int fx = x + gi.bmpX;
			int fy = y - gi.bmpY;

			//build geometry		
			Vertex a, b, c, d;
			a.x = static_cast<float>(fx);
			a.y = static_cast<float>(fy);
			a.u = static_cast<float>(gi.tx);
			a.v = static_cast<float>(gi.ty);

			b.x = static_cast<float>(fx + gi.bmpW);
			b.y = static_cast<float>(fy);
			b.u = static_cast<float>(gi.tx + gi.bmpW);
			b.v = static_cast<float>(gi.ty);

			c.x = static_cast<float>(fx + gi.bmpW);
			c.y = static_cast<float>(fy + gi.bmpH);
			c.u = static_cast<float>(gi.tx + gi.bmpW);
			c.v = static_cast<float>(gi.ty + gi.bmpH);

			d.x = static_cast<float>(fx);
			d.y = static_cast<float>(fy + gi.bmpH);
			d.u = static_cast<float>(gi.tx);
			d.v = static_cast<float>(gi.ty + gi.bmpH);

			a.Mul(psW, psH, tW, tH);
			b.Mul(psW, psH, tW, tH);
			c.Mul(psW, psH, tW, tH);
			d.Mul(psW, psH, tW, tH);

			LetterGeom l;
			l.AddQuad(a, b, c, d);
			l.SetColor(si.color);
			this->geom.push_back(l);

			x += (gi.adv >> 6);
		}
	}

	this->strChanged = false;

	if (this->geom.size() != 0)
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, geom.size() * sizeof(LetterGeom), &(geom.front()), GL_STREAM_DRAW));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	return true;
}