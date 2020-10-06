#ifndef EXTERNAL_INCLUDES_H
#define EXTERNAL_INCLUDES_H

//=====================================================================================
//additional preprocessor directives

#define USE_ICU_LIBRARY

#define THREAD_SAFETY

//Path to this can be changes - eg. if you are not using freeglut - include OpenGL here
//if you want to move strings change dir here
//the same goes if you want to move lodepng

//=====================================================================================
//includes

#include "./lodepng.h"


#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"
#include "./freeglut/include/GL/glut.h"

#include <unicode/unistr.h>
#include <unicode/schriter.h>

//#include "./Unicode/tinyutf8.h"
//#include "./Unicode/utf8.h"

#include "./Unicode/ICUUtils.h"

#ifdef _WIN32
#	include "./Utils/win_dirent.h"
#else 
#	include <dirent.h>
#endif

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

/*
typedef utf8_string UnicodeString;

#define FOREACH_32_CHAR_ITERATION(c, str) for (auto c : str)
#define BIDI(x) x
#define UTF8_TEXT(x) x
#define UTF8_UNESCAPE(x) utf8_string::build_from_escaped(x.c_str())
*/


#ifdef USE_ICU_LIBRARY

typedef icu::UnicodeString UnicodeString;
typedef icu::StringCharacterIterator UnicodeCharacterPtr;

//#define GET_SUBSTRING_VIEW(str, start, len) str.tempSubString(start, len)

#define GET_FIRST_UNICODE_CHAR_PTR(str) icu::StringCharacterIterator(str)

#define FOREACH_32_CHAR_ITERATION_FROM(c, str, start, len) icu::StringCharacterIterator iter = GET_FIRST_UNICODE_CHAR_PTR(str); \
										iter.move32(start, icu::CharacterIterator::EOrigin::kStart);	\
										for (UChar32 c = iter.current32(), l = 0; l < len; c = iter.next32(), l++)


#define FOREACH_32_CHAR_ITERATION(c, str) icu::StringCharacterIterator iter = GET_FIRST_UNICODE_CHAR_PTR(str); \
										  for (UChar32 c = iter.first32(); iter.hasNext(); c = iter.next32())

#define BIDI(x) BidiHelper::ConvertOneLine(x)

#define UTF8_TEXT(x) icu::UnicodeString::fromUTF8(x)

#define UTF8_UNESCAPE(x) icu::UnicodeString::fromUTF8(x).unescape()

#endif

#endif
