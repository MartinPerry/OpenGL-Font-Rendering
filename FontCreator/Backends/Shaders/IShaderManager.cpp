#include "./IShaderManager.h"

#if defined(_DEBUG) || defined(DEBUG)
#	define NV_REPORT_COMPILE_ERRORS
#endif

GLuint IShaderManager::BuildFromSources(const char* vSource, const char* pSource)
{
	GLuint program = 0;

	//create shader
	GLuint vShader = CompileGLSLShader(GL_VERTEX_SHADER, vSource);
	GLuint pShader = CompileGLSLShader(GL_FRAGMENT_SHADER, pSource);

	GL_CHECK(program = LinkGLSLProgram(vShader, pShader));

	GL_CHECK(glDeleteShader(vShader));
	GL_CHECK(glDeleteShader(pShader));

	this->SetShaderProgram(program);

	return program;
}

/// <summary>
/// Compile input shader
/// </summary>
/// <param name="target"></param>
/// <param name="shader"></param>
/// <returns></returns>
GLuint IShaderManager::CompileGLSLShader(GLenum target, const char* shader) const
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
		MY_LOG_ERROR("Font renderer - Shader compile failed:\n%s", temp);
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
GLuint IShaderManager::LinkGLSLProgram(GLuint vertexShader, GLuint fragmentShader) const
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

	char* infoLog = new char[infoLogLength];
	GL_CHECK(glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog));
	if ((infoLogLength > 0) && (infoLog[0] != '\0'))
	{
		MY_LOG_ERROR("Link failed:\n%s", infoLog);
	}
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

void IShaderManager::Render(int quadsCount)
{
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, quadsCount * this->GetQuadVertices()));
}