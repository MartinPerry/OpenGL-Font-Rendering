
#include "./TextureAtlasPack.h"

#include "./FontBuilder.h"
#include "./FontRenderer.h"

#include "./utf8.h"

#include <chrono>
#include <iostream>

#ifdef _MSC_VER		
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")	
#endif

#ifdef _WIN32
#include <windows.h>
#endif


//#include <gl/gl.h>
//#include <gl/glext.h>
//#include <gl/wglext.h>

#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"

#include "./freeglut/include/GL/glut.h"


#ifdef _MSC_VER
#if defined(DEBUG)|defined(_DEBUG)
#pragma comment(lib, "freeglut.lib")		
#pragma comment(lib, "glew32.lib")
#else
#pragma comment(lib, "freeglut.lib")		
#pragma comment(lib, "glew32.lib")
#endif	
#endif

float g_rotate[2] = { 0, 0 },
g_prev_x = 0,
g_prev_y = 0,
g_dolly = 5,
g_pan[2] = { 0, 0 },
g_center[3] = { 0, 0, 0 },
g_size = 0;

int   g_mbutton[3] = { 0, 0, 0 };

int   g_width = 800,
g_height = 600;

FontRenderer * fr;

//------------------------------------------------------------------------------
void reshape(int width, int height) {

	g_width = width;
	g_height = height;
	
}
//------------------------------------------------------------------------------
void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, g_width, g_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	

	double aspect = g_width / (double)g_height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, aspect, 0.01, 500.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-g_pan[0], -g_pan[1], -g_dolly);
	glRotatef(g_rotate[1], 1, 0, 0);
	glRotatef(g_rotate[0], 0, 1, 0);
	glTranslatef(-g_center[0], -g_center[1], -g_center[2]);
	glRotatef(-90, 1, 0, 0); // z-up model

	
	// fill mode always
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	// Stencil / Depth buffer and test disabled
	glDisable(GL_STENCIL_TEST);
	glStencilMask(0);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	// Blend on for alpha
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Color active
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glPrimitiveRestartIndex(-1);
	glEnable(GL_PRIMITIVE_RESTART);
	

	//render here
	fr->AddString(u8"Pøíliš\nžluouèký\nkùò", 200, 300);
	//fr->AddString(u8"lll", 200, 300);
	fr->Render();

	glutSwapBuffers();
	glutPostRedisplay();
}

//------------------------------------------------------------------------------
void motion(int x, int y) {

	if (g_mbutton[0] && !g_mbutton[1] && !g_mbutton[2]) {
		// orbit
		g_rotate[0] += x - g_prev_x;
		g_rotate[1] += y - g_prev_y;
	}
	else if (!g_mbutton[0] && g_mbutton[1] && !g_mbutton[2]) {
		// pan
		g_pan[0] -= g_dolly*(x - g_prev_x) / g_width;
		g_pan[1] += g_dolly*(y - g_prev_y) / g_height;
	}
	else if (g_mbutton[0] && g_mbutton[1] && !g_mbutton[2]) {
		// dolly
		g_dolly -= g_dolly*0.01f*(x - g_prev_x);
		if (g_dolly <= 0.01) g_dolly = 0.01f;
	}

	g_prev_x = float(x);
	g_prev_y = float(y);
}

//------------------------------------------------------------------------------
void mouse(int button, int state, int x, int y) {

	g_prev_x = float(x);
	g_prev_y = float(y);
	g_mbutton[button] = !state;
}

//------------------------------------------------------------------------------
void quit() {
	exit(0);
}

//------------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case 'q': quit();
	}
}

//------------------------------------------------------------------------------
void idle() {
	//glutPostRedisplay();
}

//------------------------------------------------------------------------------
void initGL() {	
	glEnable(GL_LIGHT0);	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	
	fr = new FontRenderer(g_width, g_height, { "arial.ttf", 40, 512, 512 });
}


int main(int argc, char ** argv)
{
	
	glutInit(&argc, argv);

	//glutSetWindowTitle( g_defaultShapes[m].name.c_str() );
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(g_width, g_height);
	glutCreateWindow("Text test");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(motion);
	glewInit();
	initGL();

	glutIdleFunc(idle);
	glutMainLoop();

	quit();


	return 0;


	FILE *f = fopen("test.txt", "rb");
	if (f == NULL)
	{
		return 0;
	}

	fseek(f, 0L, SEEK_END);
	long size = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char * data = new char[size + 1];
	fread(data, sizeof(char), size, f);	
	data[size] = 0;

	fclose(f);

	FontBuilder build = FontBuilder("arial.ttf", 512, 512, 20);
	//build.SetTightPacking();
	build.SetGridPacking(20, 20);

	//build.AddString(data);
	//build.AddString(utf8_string::build_from_escaped(data));
	//build.AddAllAsciiLetters();
	//build.AddString(utf8_string::build_from_escaped(u8"Plze\\u0148ø"));
	//build.AddString(u8"Plze\u0148");
	build.AddString("abcdefghijklmnopqrstuvwxyz");
	build.AddString("ABCDEFGHIJ");

	//http://www.fileformat.info/info/unicode/char/0148/index.htm
	
	//http://www.fileformat.info/info/charset/UTF-32/list.htm

	//build.AddCharacter('p');
	//build.AddCharacter(u'\u0148');
	//build.AddString(u8"Plze\u0148");
	//build.AddCharacter('h');
	
	
	//build.AddString(u8"žlouèký"); 

	printf("t2\n");

	//build.AddCharacter(328); //ò
	std::chrono::steady_clock::time_point b;

	b = std::chrono::steady_clock::now();
	build.CreateFontAtlas();
	std::cout << "T = " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - b).count() << std::endl;

	build.Save("p1.png");

	build.AddString("abcdefghijklmnopqrstuvwxyz");
	build.AddString("KLMNOPQRS");

	b = std::chrono::steady_clock::now();
	build.CreateFontAtlas();
	std::cout << "T = " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - b).count() << std::endl;

	build.Save("p2.png");

	return 0;
}