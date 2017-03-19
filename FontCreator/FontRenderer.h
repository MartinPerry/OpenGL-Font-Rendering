#ifndef _FONT_RENDERER_H_
#define _FONT_RENDERER_H_

class FontBuilder;

#include <vector>
#include <list>
#include <unordered_set>

#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"
#include "./freeglut/include/GL/glut.h"

#include "./tinyutf8.h"

class FontRenderer
{
public:
	typedef enum TextAlign {ALIGN_LEFT, ALIGN_CENTER} TextAlign;
	typedef enum TextAnchor { LEFT_TOP, CENTER, LEFT_DOWN } TextAnchor;

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

	void ClearStrings();

	void AddString(const utf8_string & strUTF8, int x, int y, 
		TextAnchor anchor = TextAnchor::LEFT_TOP,
		TextAlign align = TextAlign::ALIGN_LEFT);

private:
	
	typedef struct AABB
	{
		int minX;
		int maxX;

		int minY;
		int maxY;
	} AABB;

	typedef struct StringInfo
	{
		utf8_string strUTF8;
		int x;
		int y;
		TextAnchor anchor;
		TextAlign align;

		int linesCount;
		int anchorX;
		int anchorY;
		std::vector<AABB> linesAABB;
		AABB aabb;

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

	std::vector<AABB> CalcStringAABB(const utf8_string & strUTF8, 
		int x, int y, AABB & globalAABB);
	int CalcStringLines(const utf8_string & strUTF8) const;
	void CalcAnchoredPosition();
	void CalcLineAlign(const StringInfo & si, int lineId, int & x, int & y) const;

	void CreateVAO();
	GLuint CompileGLSLShader(GLenum target, const char* shader);
	GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader);
};

#endif