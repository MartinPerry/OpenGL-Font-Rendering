#include "FontShaderManager.h"

//===========================================================================================
// Default font shader manager (each glyph can have different color)
//===========================================================================================

DefaultFontShaderManager::DefaultFontShaderManager() : 
	positionLocation(0), 
	texCoordLocation(0), 
	colorLocation(0)
{
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void DefaultFontShaderManager::GetAttributtesUniforms()
{
    GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
    GL_CHECK(texCoordLocation = glGetAttribLocation(shaderProgram, "TEXCOORD0"));
    GL_CHECK(colorLocation = glGetAttribLocation(shaderProgram, "COLOR"));
    
}

void DefaultFontShaderManager::BindVertexAtribs()
{
    const GLsizei POSITION_SIZE = 2;
    const GLsizei TEXCOORD_SIZE = 2;
    const GLsizei COLOR_SIZE = 4;
    
    const GLsizei VERTEX_SIZE = (POSITION_SIZE + TEXCOORD_SIZE + COLOR_SIZE) * sizeof(float);
    const size_t POSITION_OFFSET = 0;
    const size_t TEX_COORD_OFFSET = POSITION_OFFSET + POSITION_SIZE * sizeof(float);
    const size_t COLOR_OFFSET = TEX_COORD_OFFSET + TEXCOORD_SIZE * sizeof(float);
    
    
    GL_CHECK(glEnableVertexAttribArray(positionLocation));
    GL_CHECK(glVertexAttribPointer(positionLocation, POSITION_SIZE,
                                   GL_FLOAT, GL_FALSE,
                                   VERTEX_SIZE, (void*)(POSITION_OFFSET)));
    
    GL_CHECK(glEnableVertexAttribArray(texCoordLocation));
    GL_CHECK(glVertexAttribPointer(texCoordLocation, TEXCOORD_SIZE,
                                   GL_FLOAT, GL_FALSE,
                                   VERTEX_SIZE, (void*)(TEX_COORD_OFFSET)));
    
    GL_CHECK(glEnableVertexAttribArray(colorLocation));
    GL_CHECK(glVertexAttribPointer(colorLocation, COLOR_SIZE,
                                   GL_FLOAT, GL_FALSE,
                                   VERTEX_SIZE, (void*)(COLOR_OFFSET)));
    
}

void DefaultFontShaderManager::FillVertexData(const AbstractGLRenderer::Vertex & minVertex,
                                              const AbstractGLRenderer::Vertex & maxVertex,
                                              const AbstractGLRenderer::RenderParams & rp,
                                              std::vector<float> & vec)
{
    
    float minX = 2.0f * minVertex.x - 1.0f;
    float minY = -(2.0f * minVertex.y - 1.0f);
    
    float maxX = 2.0f * maxVertex.x - 1.0f;
    float maxY = -(2.0f * maxVertex.y - 1.0f);
    
    vec.push_back(minX); vec.push_back(minY);
    vec.push_back(minVertex.u); vec.push_back(minVertex.v);
    vec.push_back(rp.color.r); vec.push_back(rp.color.g);
    vec.push_back(rp.color.b); vec.push_back(rp.color.a);
    
    vec.push_back(maxX); vec.push_back(minY);
    vec.push_back(maxVertex.u); vec.push_back(minVertex.v);
    vec.push_back(rp.color.r); vec.push_back(rp.color.g);
    vec.push_back(rp.color.b); vec.push_back(rp.color.a);
    
    vec.push_back(minX); vec.push_back(maxY);
    vec.push_back(minVertex.u); vec.push_back(maxVertex.v);
    vec.push_back(rp.color.r); vec.push_back(rp.color.g);
    vec.push_back(rp.color.b); vec.push_back(rp.color.a);
    
    //========================================================
    //========================================================
    
    vec.push_back(maxX); vec.push_back(minY);
    vec.push_back(maxVertex.u); vec.push_back(minVertex.v);
    vec.push_back(rp.color.r); vec.push_back(rp.color.g);
    vec.push_back(rp.color.b); vec.push_back(rp.color.a);
    
    vec.push_back(maxX); vec.push_back(maxY);
    vec.push_back(maxVertex.u); vec.push_back(maxVertex.v);
    vec.push_back(rp.color.r); vec.push_back(rp.color.g);
    vec.push_back(rp.color.b); vec.push_back(rp.color.a);
    
    vec.push_back(minX); vec.push_back(maxY);
    vec.push_back(minVertex.u); vec.push_back(maxVertex.v);
    vec.push_back(rp.color.r); vec.push_back(rp.color.g);
    vec.push_back(rp.color.b); vec.push_back(rp.color.a);
    
    /*
    v[0] = a;
    v[1] = b;
    v[2] = d;
    
    v[3] = b;
    v[4] = c;
    v[5] = d;
    */
}

//===========================================================================================
// Single color font shader manager
//===========================================================================================

SingleColorFontShaderManager::SingleColorFontShaderManager() :
	positionLocation(0),
	texCoordLocation(0),
	colorUniform(0),
	r(1.0f),
	g(1.0f),
	b(1.0f),
	a(1.0f)
{
}

void SingleColorFontShaderManager::SetColor(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

/// <summary>
/// Get shader uniforms and attributes locations
/// </summary>
void SingleColorFontShaderManager::GetAttributtesUniforms()
{
    GL_CHECK(colorUniform = glGetUniformLocation(shaderProgram, "fontColor"));

	GL_CHECK(positionLocation = glGetAttribLocation(shaderProgram, "POSITION"));
	GL_CHECK(texCoordLocation = glGetAttribLocation(shaderProgram, "TEXCOORD0"));		
}

void SingleColorFontShaderManager::BindVertexAtribs()
{
	const GLsizei POSITION_SIZE = 2;
	const GLsizei TEXCOORD_SIZE = 2;
	
	const GLsizei VERTEX_SIZE = (POSITION_SIZE + TEXCOORD_SIZE) * sizeof(float);
	const size_t POSITION_OFFSET = 0;
	const size_t TEX_COORD_OFFSET = POSITION_OFFSET + POSITION_SIZE * sizeof(float);
	

	GL_CHECK(glEnableVertexAttribArray(positionLocation));
	GL_CHECK(glVertexAttribPointer(positionLocation, POSITION_SIZE,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(POSITION_OFFSET)));

	GL_CHECK(glEnableVertexAttribArray(texCoordLocation));
	GL_CHECK(glVertexAttribPointer(texCoordLocation, TEXCOORD_SIZE,
		GL_FLOAT, GL_FALSE,
		VERTEX_SIZE, (void*)(TEX_COORD_OFFSET)));
}

void SingleColorFontShaderManager::PreRender()
{
	GL_CHECK(glUniform4f(colorUniform, r, g, b, a));
}

void SingleColorFontShaderManager::FillVertexData(
    const AbstractGLRenderer::Vertex & minVertex,
	const AbstractGLRenderer::Vertex & maxVertex,
	const AbstractGLRenderer::RenderParams & rp,
	std::vector<float> & vec)
{

	const float minX = 2.0f * minVertex.x - 1.0f;
	const float minY = -(2.0f * minVertex.y - 1.0f);

	const float maxX = 2.0f * maxVertex.x - 1.0f;
	const float maxY = -(2.0f * maxVertex.y - 1.0f);

	vec.push_back(minX); vec.push_back(minY);
	vec.push_back(minVertex.u); vec.push_back(minVertex.v);
	
	vec.push_back(maxX); vec.push_back(minY);
	vec.push_back(maxVertex.u); vec.push_back(minVertex.v);
	
	vec.push_back(minX); vec.push_back(maxY);
	vec.push_back(minVertex.u); vec.push_back(maxVertex.v);
	
	//========================================================
	//========================================================

	vec.push_back(maxX); vec.push_back(minY);
	vec.push_back(maxVertex.u); vec.push_back(minVertex.v);
	
	vec.push_back(maxX); vec.push_back(maxY);
	vec.push_back(maxVertex.u); vec.push_back(maxVertex.v);
	
	vec.push_back(minX); vec.push_back(maxY);
	vec.push_back(minVertex.u); vec.push_back(maxVertex.v);
	
	/*
	v[0] = a;
	v[1] = b;
	v[2] = d;

	v[3] = b;
	v[4] = c;
	v[5] = d;
	*/
}
