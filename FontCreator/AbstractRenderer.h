#ifndef ABSTRACT_RENDERER_H
#define ABSTRACT_RENDERER_H

class FontBuilder;
class IFontShaderManager;

#include <vector>
#include <list>
#include <unordered_set>
#include <functional>
#include <shared_mutex>

#include "./FontStructures.h"

#include "./Externalncludes.h"


class AbstractRenderer
{
public:
	typedef enum TextAlign {ALIGN_LEFT, ALIGN_CENTER} TextAlign;
	typedef enum TextAnchor { LEFT_TOP, CENTER, LEFT_DOWN } TextAnchor;
	typedef enum TextType { TEXT, CAPTION } TextType;
	typedef enum AxisYOrigin { TOP, DOWN } AxisYOrigin;

    typedef struct Vertex
    {
        float x, y;
        float u, v;
    } Vertex;
    
    typedef struct Color
    {
        float r, g, b, a;
        bool IsSame(const Color & c) {
            return (c.r == r) && (c.g == g) &&
            (c.b == b) && (c.a == a); }
    } Color;
	

	static const Color DEFAULT_COLOR;

	static std::vector<std::string> GetFontsInDirectory(const std::string & fontDir);

	AbstractRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion);
    AbstractRenderer(const std::vector<Font> & fs, RenderSettings r, int glVersion,
                     const char * vSource, const char * pSource, std::shared_ptr<IFontShaderManager> sm);
    
	virtual ~AbstractRenderer();

	FontBuilder * GetFontBuilder();
	void SetCanvasSize(int w, int h);
	void SetAxisYOrigin(AxisYOrigin axisY);
	void SetCaption(const UnicodeString & mark, int offsetInPixels);
	void SetCaptionOffset(int offsetInPixels);

	int GetCanvasWidth() const;
	int GetCanvasHeight() const;

	std::shared_ptr<IFontShaderManager> GetShaderManager() const;

	void SwapCanvasWidthHeight();

	void Clear();

	void SetEnabled(bool val);
	bool IsEnabled() const;

	void Render();
    void Render(std::function<void(GLuint)> preDrawCallback, std::function<void()> postDrawCallback);
	

	
protected:
	        
	typedef struct CaptionInfo
	{
		UnicodeString mark;
		int offset;

	} CaptionInfo;

	typedef struct AABB
	{
		int minX;
		int maxX;

		int minY;
		int maxY;
		
		AABB() : minX(std::numeric_limits<int>::max()),
			minY(std::numeric_limits<int>::max()),
			maxX(std::numeric_limits<int>::min()),
			maxY(std::numeric_limits<int>::min())
		{}

	} AABB;

	
	

	typedef struct Shader
	{
        GLuint program;

		const char * vSource;
		const char * pSource;
        bool isDefault;
        
	} Shader;

    std::shared_ptr<IFontShaderManager> sm;
    RenderSettings rs;
    
	FontBuilder * fb;
	
	CaptionInfo ci;

	AxisYOrigin axisYOrigin;

    int quadsCount;
	std::vector<float> geom;
	
	float psW; //pixel size in width
	float psH; //pixel size in height

	float tW ;  //pixel size in width
	float tH; //pixel size in height


	bool strChanged;
#ifdef THREAD_SAFETY
	std::shared_timed_mutex m;
#endif

	GLuint vbo;
	GLuint vao;
	GLuint fontTex;
	Shader shader;
	int glVersion;
	
	bool renderEnabled;

	virtual bool GenerateGeometry() = 0;
	 
	void InitGL();
	
	void CreateVAO();
	GLuint CompileGLSLShader(GLenum target, const char* shader);
	GLuint LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader);

    void AddQuad(const GlyphInfo & gi, int x, int y, const Color & col);
	void FillTexture();
	void FillVB();
};

#endif
