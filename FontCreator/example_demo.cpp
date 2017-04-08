
#include "./TextureAtlasPack.h"

#include "./FontBuilder.h"
#include "./StringRenderer.h"
#include "./NumberRenderer.h"

#include "./Unicode/utf8.h"
#include "./Unicode/uninorms.h"

#include <chrono>
#include <iostream>


#ifdef _WIN32
#include <windows.h>
#ifdef _DEBUG
//#include <vld.h>
#endif
#endif


#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"

#include "./freeglut/include/GL/freeglut.h"


#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "./libs/freeglut.lib")		
#pragma comment(lib, "./libs/glew32.lib")
#endif


int g_width = 800;
int g_height = 600;

StringRenderer * fr;
NumberRenderer * fn;

//------------------------------------------------------------------------------
void reshape(int width, int height) {

	g_width = width;
	g_height = height;
	
}
//------------------------------------------------------------------------------
void display() {

	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, g_width, g_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//glDisable(GL_LIGHTING);
	
		
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
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glPrimitiveRestartIndex(-1);
	//glEnable(GL_PRIMITIVE_RESTART);
	

	//render here
	/*
	fr->AddString(u8"P¯Ìliö\nûluùouËk˝\nk˘Ú", 400, 300,
		FontRenderer::LEFT_TOP,
		FontRenderer::ALIGN_LEFT);
	*/
	/*
	fr->AddString(u8"P¯Ìliö\nûluùouËk˝\nk˘Ú", 0.5f, 0.5f,
		{1,1,0,1},
		FontRenderer::CENTER,
		FontRenderer::ALIGN_CENTER);
	*/

	
	fr->AddStringCaption(u8"P¯Ìliö\nûluùouËk˝\nk˘Ú", 0.5f, 0.5f, { 1,1,0,1 });

	//fr->AddString(u8"lll", 200, 300);
	fr->Render();

	//fn->AddNumber(-45.27, 100, 100);
	fn->AddNumberCaption(-45.27, 100, 100);
	fn->Render();

	glutSwapBuffers();
	glutPostRedisplay();
	
}

//------------------------------------------------------------------------------
void quit() {
	exit(0);
}


//------------------------------------------------------------------------------
void idle() {
	//glutPostRedisplay();
}

//------------------------------------------------------------------------------
void initGL() {	
			
	fr = new StringRenderer(g_width, g_height, { "arial.ttf", 40, 512, 512 });
	fn = new NumberRenderer(g_width, g_height, { "arial.ttf", 40, 512, 512 });
}


void Normalize()
{
	
	utf8_string u8str = u8"\u4e0a\u6d77 P¯Ìliö ûluùouËk˝ k˘Ú ˙pÏl Ô·belskÈ Ûdy";

	for (auto c : u8str)
	{
		std::u32string uu;
		uu.push_back(c);
		//ufal::unilib::uninorms::nfc(uu);
		ufal::unilib::uninorms::nfd(uu);

		printf("%i ", uu.size());

		//ufal::unilib::uninorms::nfkc(uu);
		//ufal::unilib::uninorms::nfkd(uu);
	}
}

int main(int argc, char ** argv)
{
#ifdef _WIN32 
#ifdef _DEBUG
	//VLDSetReportOptions(VLD_OPT_REPORT_TO_DEBUGGER | VLD_OPT_REPORT_TO_FILE, L"leaks.txt");
#endif
#endif
	

	

	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(g_width, g_height);
	glutCreateWindow("Font Rendering Example");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);		
	glewInit();
	initGL();

	glutIdleFunc(idle);
	glutMainLoop();
	
	delete fr;
	
	quit();


	return 0;

	/*
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
	//build.AddString(utf8_string::build_from_escaped(u8"Plze\\u0148¯"));
	//build.AddString(u8"Plze\u0148");
	build.AddString("abcdefghijklmnopqrstuvwxyz");
	build.AddString("ABCDEFGHIJ");

	//http://www.fileformat.info/info/unicode/char/0148/index.htm
	
	//http://www.fileformat.info/info/charset/UTF-32/list.htm

	//build.AddCharacter('p');
	//build.AddCharacter(u'\u0148');
	//build.AddString(u8"Plze\u0148");
	//build.AddCharacter('h');
	
	
	//build.AddString(u8"ûlùouËk˝"); 

	printf("t2\n");

	//build.AddCharacter(328); //Ú
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
	*/
}