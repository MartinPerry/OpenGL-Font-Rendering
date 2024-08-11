#include "./BackgroundOpenGL.h"

#include "./Shaders/Shaders.h"
#include "./Shaders/BackgroundShaderManager.h"

BackgroundOpenGL::BackgroundOpenGL() : 
	BackgroundOpenGL(BACKGROUND_PIXEL_SHADER_SOURCE, BACKGROUND_VERTEX_SHADER_SOURCE,
						std::make_shared<BackgroundShaderManager>())
{
}

BackgroundOpenGL::BackgroundOpenGL(const char* vSource, const char* pSource, std::shared_ptr<IShaderManager> sm) :
	sm(sm),
	vbo(0),
	vao(0)
{
	
	this->shader.program = 0;
	this->shader.pSource = pSource;
	this->shader.vSource = vSource;

	if ((pSource == BACKGROUND_PIXEL_SHADER_SOURCE) && (vSource == BACKGROUND_VERTEX_SHADER_SOURCE))
	{
		this->shader.isDefault = true;
	}
	else
	{
		this->shader.isDefault = false;
	}
	

	this->InitGL();
}

BackgroundOpenGL::~BackgroundOpenGL()
{
	FONT_UNBIND_SHADER;	
	FONT_UNBIND_ARRAY_BUFFER;
	FONT_UNBIND_VAO;

	GL_CHECK(glDeleteProgram(shader.program));	
	GL_CHECK(glDeleteBuffers(1, &this->vbo));
	GL_CHECK(glDeleteVertexArrays(1, &this->vao));
}

/// <summary>
/// Init OpenGL
/// </summary>
void BackgroundOpenGL::InitGL()
{

	//create shader
	shader.program = this->sm->BuildFromSources(shader.vSource, shader.pSource);

	

	this->sm->GetAttributtesUniforms();


	//create VBO
	GL_CHECK(glGenBuffers(1, &this->vbo));

	//create VAO
	this->InitVAO();

}

/// <summary>
/// Create VAO
/// </summary>
void BackgroundOpenGL::InitVAO()
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

	//this->sm->BindVertexAtribs();

	FONT_UNBIND_ARRAY_BUFFER;

	FONT_UNBIND_VAO;
}


void BackgroundOpenGL::AddQuad(const AbstractRenderer::Vertex& vmin, const AbstractRenderer::Vertex& vmax)
{
}