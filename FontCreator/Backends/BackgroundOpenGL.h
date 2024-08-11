#ifndef BACKGROUND_OPENGL_H
#define BACKGROUND_OPENGL_H

class IShaderManager;

#include "../AbstractRenderer.h"

class BackgroundOpenGL 
{
public:
	BackgroundOpenGL();
	BackgroundOpenGL(const char* vSource, const char* pSource, std::shared_ptr<IShaderManager> sm);
	~BackgroundOpenGL();

	void AddQuad(const AbstractRenderer::Vertex& vmin, const AbstractRenderer::Vertex& vmax);

protected:
	struct Shader
	{
		GLuint program;

		const char* vSource;
		const char* pSource;
		bool isDefault;
	};

	std::shared_ptr<IShaderManager> sm;

	GLuint vbo;
	GLuint vao;
	Shader shader;

	void InitGL();
	void InitVAO();
	
};

#endif