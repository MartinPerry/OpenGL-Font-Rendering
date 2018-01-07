#ifndef _EXTERNAL_INCLUDES_H_
#define _EXTERNAL_INCLUDES_H_



//Path to this can be changes - eg. if you are not using freeglut - include OpenGL here
//if you want to move strings change dir here
//the same goes if you want to move lodepng

#include "./lodepng.h"


#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"
#include "./freeglut/include/GL/glut.h"

#include <unicode/unistr.h>
#include <unicode/schriter.h>

//#include "./Unicode/tinyutf8.h"
//#include "./Unicode/utf8.h"

#ifdef _WIN32
#include "./Utils/win_dirent.h"
#else 
#include <dirent.h>
#endif


#ifndef SAFE_DELETE
#define SAFE_DELETE(a) {if (a != nullptr) { delete   a; a = nullptr; }};
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) {if (a != nullptr) { delete[] a; a = nullptr; }};
#endif

#ifndef MY_LOG_ERROR
#define MY_LOG_ERROR(...) printf(__VA_ARGS__)
#endif

#define USE_ICU_LIBRARY

/*
typedef utf8_string UnicodeString;

#define FOREACH_32_CHAR_ITERATION(c, str) for (auto c : str)
#define BIDI(x) x
#define UTF8_TEXT(x) x
#define UTF8_UNESCAPE(x) utf8_string::build_from_escaped(x.c_str())
*/


#ifdef USE_ICU_LIBRARY

typedef icu::UnicodeString UnicodeString;

#define FOREACH_32_CHAR_ITERATION(c, str) icu::StringCharacterIterator iter = icu::StringCharacterIterator(str); \
										  for (UChar32 c = iter.first32(); iter.hasNext(); c = iter.next32())

#define BIDI(x) BidiHelper::ConvertOneLine(x)

#define UTF8_TEXT(x) icu::UnicodeString::fromUTF8(x)

#define UTF8_UNESCAPE(x) icu::UnicodeString::fromUTF8(x).unescape()

#endif

#endif
