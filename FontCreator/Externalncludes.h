#ifndef _EXTERNAL_INCLUDES_H_
#define _EXTERNAL_INCLUDES_H_



//Path to this can be changes - eg. if you are not using freeglut - include OpenGL here
//if you want to move strings change dir here
//the same goes if you want to move lodepng

#include "./lodepng.h"


#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"
#include "./freeglut/include/GL/glut.h"

#include "./Unicode/tinyutf8.h"
#include "./Unicode/utf8.h"


#ifndef SAFE_DELETE
#define SAFE_DELETE(a) {if (a != nullptr) { delete   a; a = nullptr; }};
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) {if (a != nullptr) { delete[] a; a = nullptr; }};
#endif

#ifndef MY_LOG_ERROR
#define MY_LOG_ERROR(...) printf(__VA_ARGS__)
#endif

#endif
