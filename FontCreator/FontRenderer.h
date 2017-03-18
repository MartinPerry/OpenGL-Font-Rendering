#ifndef _FONT_RENDERER_H_
#define _FONT_RENDERER_H_

class FontBuilder;

#include <vector>

#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"
#include "./freeglut/include/GL/glut.h"

#include "./tinyutf8.h"

class FontRenderer
{
public:
	typedef struct Font
	{
		std::string name;
		int size;
		int textureWidth;
		int textureHeight;

	} Font;

	FontRenderer(int deviceW, int deviceH, Font f);
	~FontRenderer();

	void Render();

	void AddString(const utf8_string & strUTF8, int x, int y);

private:
	
	typedef struct StringInfo
	{
		utf8_string strUTF8;
		int x;
		int y;
	} StringInfo;

	typedef struct Vertex
	{
		float x, y;
		float u, v;

		void Mul(float pW, float pH, float tW, float tH)
		{
			x *= pW;
			y *= pH;
			u *= tW;
			v *= tH;
		};

	} Vertex;

	typedef struct LetterGeom
	{
		Vertex v[6];

		void AddQuad(Vertex & a, Vertex & b, Vertex & c, Vertex & d) 
		{
			v[0] = a;
			v[1] = b;
			v[2] = d;

			v[3] = b;
			v[4] = c;
			v[5] = d;
		}
	} LetterGeom;

	typedef struct Shader
	{
		GLuint program;

		
		GLuint colorLocation;
		
		GLuint positionLocation;
		GLuint texCoordLocation;

		static const char * vSource;
		static const char * pSource;

	} Shader;

	FontBuilder * fb;
	std::vector<StringInfo> strs;
	std::vector<LetterGeom> geom;

	int deviceW;
	int deviceH;
	bool strChanged;

	GLuint vbo;
	GLuint vao;
	GLuint fontTex;
	Shader shader;
	

	void InitGL();
	bool GenerateStringGeometry();
	float MapRange(float fromMin, float fromMax, float toMin, float toMax, float s);

	void CreateVAO();
	GLuint CompileGLSLShader(GLenum target, const char* shader);
	GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader);
};

#endif