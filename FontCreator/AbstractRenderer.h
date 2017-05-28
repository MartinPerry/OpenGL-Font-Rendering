#ifndef _ABSTRACT_RENDERER_H_
#define _ABSTRACT_RENDERER_H_

class FontBuilder;

#include <vector>
#include <list>
#include <unordered_set>

#include "./FontStructures.h"

#include "./Externalncludes.h"

class AbstractRenderer
{
public:
	typedef enum TextAlign {ALIGN_LEFT, ALIGN_CENTER} TextAlign;
	typedef enum TextAnchor { LEFT_TOP, CENTER, LEFT_DOWN } TextAnchor;
	typedef enum TextType { TEXT, CAPTION } TextType;
	typedef enum AxisYOrigin { TOP, DOWN } AxisYOrigin;

	typedef struct Color 
	{ 
		float r, g, b, a; 
		bool IsSame(const Color & c) { 
			return (c.r == r) && (c.g == g) && 
			(c.b == b) && (c.a == a); }
	} Color;

	

	static const Color DEFAULT_COLOR;

	AbstractRenderer(const std::vector<Font> & fs, RenderSettings r);
	virtual ~AbstractRenderer();

	FontBuilder * GetFontBuilder();
	void SetCanvasSize(int w, int h);
	void SetAxisYOrigin(AxisYOrigin axisY);

	void SwapCanvasWidthHeight();

	void SetEnabled(bool val);
	bool IsEnabled() const;

	void Render();

	

	
protected:
	
	typedef struct CaptionInfo
	{
		utf8_string mark;
		int offset;

	} CaptionInfo;

	typedef struct AABB
	{
		int minX;
		int maxX;

		int minY;
		int maxY;
	} AABB;

	
	typedef struct Vertex
	{
		float x, y;
		float u, v;
		float r, g, b, a;

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

		void SetColor(const Color & c)
		{
			for (int i = 0; i < 6; i++)
			{
				v[i].r = c.r;
				v[i].g = c.g;
				v[i].b = c.b;
				v[i].a = c.a;
			}
		}

	} LetterGeom;

	typedef struct Shader
	{
		GLuint program;


		GLuint positionLocation;
		GLuint texCoordLocation;
		GLuint colorLocation;

		static const char * vSource;
		static const char * pSource;

	} Shader;

	FontBuilder * fb;
	
	CaptionInfo ci;

	AxisYOrigin axisYOrigin;

	std::vector<LetterGeom> geom;
	
	int deviceW;
	int deviceH;
	bool strChanged;

	GLuint vbo;
	GLuint vao;
	GLuint fontTex;
	Shader shader;
	
	bool renderEnabled;

	virtual bool GenerateGeometry() = 0;
	 
	void InitGL();
	
	void CreateVAO();
	GLuint CompileGLSLShader(GLenum target, const char* shader);
	GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader);

	void FillTexture();
	void FillVB();
};

#endif