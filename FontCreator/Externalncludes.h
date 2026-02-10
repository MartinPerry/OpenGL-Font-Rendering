#ifndef EXTERNAL_INCLUDES_H
#define EXTERNAL_INCLUDES_H

//=====================================================================================
//additional preprocessor directives

#ifndef USE_ICU_LIBRARY
#	define USE_ICU_LIBRARY
#endif

#ifndef THREAD_SAFETY
#	define THREAD_SAFETY
#endif

//Path to this can be changes - eg. if you are not using freeglut - include OpenGL here
//if you want to move strings change dir here
//the same goes if you want to move lodepng

//=====================================================================================
//includes

#include <string>

#include "./lodepng.h"


#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"
#include "./freeglut/include/GL/glut.h"

#ifdef USE_ICU_LIBRARY
#	include <unicode/unistr.h>
#	include <unicode/schriter.h>
#	include "./Unicode/BidiHelper.h"
#	include "./Unicode/ICUUtils.h"
#endif

#ifdef _WIN32
#	include "./Utils/win_dirent.h"
#else 
#	include <dirent.h>
#endif

#include "./Utils/utf8/utf8.h"

#include "./Utils/StringIterators.h"

#include "./Utils/ankerl/unordered_dense.h"

template <typename K, typename V>
using HashMap = ankerl::unordered_dense::map<K, V>;

//=====================================================================================
//Global macros

#ifndef SAFE_DELETE
#	define SAFE_DELETE(a) {if (a != nullptr) { delete   a; a = nullptr; }};
#endif

#ifndef SAFE_DELETE_ARRAY
#	define SAFE_DELETE_ARRAY(a) {if (a != nullptr) { delete[] a; a = nullptr; }};
#endif

#ifndef MY_LOG_ERROR
#	define MY_LOG_ERROR(...) printf(__VA_ARGS__); printf("\n");
#endif


#ifndef MY_LOG_INFO
#	define MY_LOG_INFO(...) printf(__VA_ARGS__); printf("\n");
#endif

//=====================================================================================
//OpenGL error checks

static void CheckOpenGLError(const char* stmt, const char* fname, int line)
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

		MY_LOG_ERROR("OpenGL error %s, at %s:%i - for %s", error.c_str(), fname, line, stmt);
		//abort();
	}
}

#ifndef GL_CHECK
#	if defined(_DEBUG) || defined(DEBUG)
#		define GL_CHECK(stmt) do { \
                stmt; \
                CheckOpenGLError(#stmt, __FILE__, __LINE__); \
            } while (0);
#	else
#		define GL_CHECK(stmt) stmt
#	endif
#endif

//=====================================================================================
//OpenGL "overrides"

//You can override here binding / unbinding of OpenGL things and 
//use your own management system
#define FONT_BIND_SHADER(id) GL_CHECK(glUseProgram(id))
#define FONT_UNBIND_SHADER GL_CHECK(glUseProgram(0))
#define FONT_BIND_VAO(id) GL_CHECK(glBindVertexArray(id))
#define FONT_UNBIND_VAO GL_CHECK(glBindVertexArray(0))
#define FONT_BIND_ARRAY_BUFFER(id) GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, id))
#define FONT_UNBIND_ARRAY_BUFFER GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0))
#define FONT_BIND_TEXTURE_2D(id) GL_CHECK(glBindTexture(GL_TEXTURE_2D, id))
#define FONT_UNBIND_TEXTURE_2D GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0))


#ifdef TARGET_COMPUTER
#	define TEXTURE_SINGLE_CHANNEL GL_RED
#else 
#	define TEXTURE_SINGLE_CHANNEL GL_LUMINANCE
#endif

//=====================================================================================
//String manipulation

using StringUtf8 = std::u8string;

#define AS_UTF8(x) StringUtf8((char8_t*)x)


#endif
