
#include "./TextureAtlasPack.h"

#include "./FontBuilder.h"
#include "./StringRenderer.h"
#include "./NumberRenderer.h"

#include "./Unicode/utf8.h"
#include "./Unicode/uninorms.h"

#include "./Utils/CharacterExtraxtor.h"
#include "./Utils/StringIterators.h"

#include <chrono>
#include <iostream>


#if __has_include(<vld.h>)
#	include <vld.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#endif


#include "./freeglut/include/GL/wgl/glew.h"
#include "./freeglut/include/GL/wgl/wglew.h"

#include "./freeglut/include/GL/freeglut.h"


#ifdef _MSC_VER
#	pragma comment(lib, "opengl32.lib")
#	pragma comment(lib, "./libs/freeglut.lib")		
#	pragma comment(lib, "./libs/glew32.lib")
#endif


int g_width = 800;
int g_height = 600;

StringRenderer * fr;
NumberRenderer * fn;


std::vector<int32_t> allChars;

static int randInited = 0;

std::string CreateRandomString(int len)
{
	if (randInited == 0)
	{
		srand(static_cast<unsigned int>(time(0)));
		randInited = 1;
	}

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz\n";

	std::string r = "";
	for (int i = 0; i < len; ++i) 
	{
		r += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	//printf("%s\n", r.c_str());

	return r;
}


//------------------------------------------------------------------------------
void reshape(int width, int height) {

	g_width = width;
	g_height = height;
	
}

//------------------------------------------------------------------------------
void display() {

	glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
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

	/*
	fr->Clear();
	for (float x = 0; x <= 1.0; x += 0.05)
	{
		for (float y = 0; y <= 1.0; y += 0.05)
		{
			fr->AddStringCaption(u8"P¯Ìliö\nûluùouËk˝\nk˘Ú", x, y, { 1,1,0,1 });
		}
	}
	*/
	
	fr->Clear();
	//fr->AddStringCaption(UTF8_TEXT(u8"Velmi"), -0.05f, 0.5f, { 1,1,0,1 });
	//fr->AddString(UTF8_TEXT(u8"Velmi \U0001F300"), 0.5f, 0.5f, { 1,1,0,1 });
	//fr->AddStringCaption(UTF8_TEXT(u8"\U0001F300P¯Ìliö mal˝ p\n(ûluùouËk˝)\nk˘yÚy"), 0.5f, 0.5f, { 1,1,0,1 });
	//fr->AddStringCaption(UTF8_TEXT(u8"P¯Ìliö mal˝p\n(ûluùouËk˝)\nk˘yÚy"), 0.5f, 0.5f, { 1,1,0,1 });
	//fr->AddStringCaption(UTF8_TEXT(u8"ahoj \u4e3d xx \u0633\u0644\u0627\u0645"), 0.5f, 0.5f, { 1,1,0,1 });
	//fr->AddStringCaption(UTF8_TEXT(u8"\u4e3d"), 0.8f, 0.5f, { 1,1,0,1 });
	//fr->AddStringCaption(UTF8_TEXT(u8"\u103c"), 0.2f, 0.5f, { 1,1,0,1 });
	//fr->AddStringCaption(UTF8_TEXT(u8"Baf"), 0.8f, 0.8f, { 1,1,0,1 });
	//fr->AddString(UTF8_TEXT(u8"[]"), 0.5f, 0.5f, { 1,1,0,1 });
		
	//fr->AddString(UTF8_TEXT(CreateRandomString(int(1 + rand() % 15)).c_str()), 0.5f, 0.5f, { 1,1,0,1 }, AbstractRenderer::TextAnchor::CENTER);
	

	//fr->AddString(UTF8_TEXT(u8"x \U0001F300 x"), 0.5f, 0.5f);
	//fr->AddString(UTF8_TEXT(u8"MLQp\U0001F300x"), 0.5f, 0.5f, { 1,1,1,1 }, AbstractRenderer::TextAnchor::CENTER);
	//fr->AddString(UTF8_TEXT(u8"\U0001F600"), 0.5f, 0.5f);
	//fr->AddString(UTF8_TEXT(u8"x"), 0.5f, 0.5f);
	fr->SetFontTextureLinearFiler(true);
	
	/*
	FontSize f1(12_pt);
	fr->GetFontBuilder()->SetAllFontSize(f1);
	fr->AddString(//UTF8_TEXT(CreateRandomString(5).c_str()), 
		UTF8_TEXT("Ahoj\nsvete\nsvetg"),
		0.0f, 1.0f, 
		{ 1,1,0,1, 1.0 }, 
		AbstractRenderer::TextAnchor::LEFT_DOWN);
		

	fr->AddString(//UTF8_TEXT(CreateRandomString(5).c_str()), 
		UTF8_TEXT("Ahoj\nsvete\nsvetg"),
		0.0f, 0.0f,
		{ 1,1,0,1, 2.0 },
		AbstractRenderer::TextAnchor::LEFT_TOP);
	*/

	/*
	fr->AddString(//UTF8_TEXT(CreateRandomString(5).c_str()), 
		UTF8_TEXT("Ahoj\nsvete\nsvetg"),
		0.5f, 0.5f,
		{ 1,1,0,1, 1.0 },
		AbstractRenderer::TextAnchor::CENTER,
		AbstractRenderer::TextAlign::ALIGN_CENTER);
	*/
	
	fr->AddStringCaption(UTF8_TEXT(CreateRandomString(10).c_str()), 
		//UTF8_TEXT("Ahoj\nsvete\nsvetg kuk"),
		//UTF8_TEXT("H\n1023hPa"),
		0.5f, 0.85f,
		{ 1,1,0,1, 1.0 });

	//=================================
	//debug
	auto & si = fr->GetLastStringInfo();	
	if (si.lines.size() > 1)
	{
		si.lines[0].renderParams.scale = 2.0;
		if (si.lines.size() > 1) si.lines[1].renderParams.scale = 0.8;
		if (si.lines.size() > 2) si.lines[2].renderParams.scale = 3.0;
	}
	else
	{
		//	si.lines[0].renderParams.scale = 4.0;
	}
	//for (auto & l : si.lines) l.renderParams.scale = 4.0;
	//=================================


	//fr->AddStringCaption(rrr, 0.5, 0.8);

	//fr->AddString(u8"lll", 200, 300);
	fr->Render();

	/*
	fr->Clear();
	//FontSize f2(24_pt);
	//fr->GetFontBuilder()->SetAllFontSize(f2);
	fr->AddString(//UTF8_TEXT(CreateRandomString(5).c_str()), 
		UTF8_TEXT("Ahoj\nsvete"),
		0.7f, 0.3f, 
		{ 1,1,0,1, 2.0 }, 
		AbstractRenderer::TextAnchor::CENTER);
	fr->Render();


	fr->Clear();
	FontSize f2(24_pt);
	fr->GetFontBuilder()->SetAllFontSize(f2);
	fr->AddString(//UTF8_TEXT(CreateRandomString(5).c_str()), 
		UTF8_TEXT("Ahoj\nsvete"),
		0.7f, 0.7f,
		{ 1,1,0,1, 1.0 },
		AbstractRenderer::TextAnchor::CENTER);
	fr->Render();
	*/

	
	fn->Clear();
	//fn->AddNumber(-45.27, 100, 100);
	//fn->AddNumberCaption(-450.013, 100, 100, { 1, 1.0f, 1.0f, 1 });
	//fn->AddNumberCaption(-897456, 100, 300, { 1, 1.0f, 1.0f, 1 });
	int nmbr = rand();
	//printf("%d\n", nmbr);
	//fn->AddNumber((double)(nmbr + nmbr / 100.0), 0.5f, 0.5f, { 1,1,0,1 }, AbstractRenderer::TextAnchor::CENTER);
	//fn->AddNumber(600010, 0.5f, 0.5f, { 1,1,0,1 }, AbstractRenderer::TextAnchor::CENTER);
	//fn->AddNumberCaption(60000, 0.5f, 0.4f, { 1,1,0,1 });
	//fn->Render();
	
	

	glutSwapBuffers();	
	//glutPostRedisplay();
	
}

//------------------------------------------------------------------------------
void quit() {

	delete fr;
	delete fn;

	//exit(0);
}


//------------------------------------------------------------------------------
void idle() {
	glutPostRedisplay();
}

template <typename S>
struct Wrapper 
{
	S str;
};

//------------------------------------------------------------------------------
void initGL() {	
		
	//http://dpi.lv
	
	Wrapper<std::string> strA;
	strA.str = "ahaoj";
	CustromAsciiIterator it2 = CustromIteratorCreator::Create(strA.str);

	uint32_t c2;
	while ((c2 = it2.GetCurrentAndAdvance()) != it2.DONE)
	{
		printf("%d ", c2);
	}

	printf("\n");

	/*
	Wrapper<icu::UnicodeString> str;
	str.str = "ahaoj";
	CustomUnicodeIterator it = CustromIteratorCreator::Create(str.str);

	uint32_t c;
	while ((c = it.GetCurrentAndAdvance()) != it.DONE)
	{
		printf("%d ", c);
	}
	*/

	printf("\n");
	/*
	Font f;
	//f.name = "test.ttf";	
	f.name = "f1.ttf";
	//f.name = "NotoSansCJKtc-Regular.otf";	
	f.size = 16_pt;

	Font f2;
	//f.name = "test.ttf";	
	f2.name = "f2.otf";
	//f.name = "NotoSansCJKtc-Regular.otf";	
	f2.size = 16_pt;
	*/

	auto fontFiles = AbstractRenderer::GetFontsInDirectory("../fonts2/");

	std::vector<Font> fonts;
	for (auto d : fontFiles)
	{
		Font f(d, 12_pt);
		
		fonts.push_back(f);
	}
	
	Font ft("../fonts2/merged_out_2048_94.ttf", 12_pt);	
	//fonts.push_back(ft);
	
	Font fArial("../fonts/arial_unicode.ttf", 12_pt);
	//fonts.clear();
	//fonts.push_back(fArial);

	/*
	Font f4;	
	f4.name = "arial.ttf";	
	f4.size = 16_pt;
	

	
	Font f;	
	f.name = "merged_out_1000.otf";	
	f.size = 16_pt;

	Font f2;	
	f2.name = "merged_out_1000.ttf";	
	f2.size = 16_pt;


	Font f3;
	f3.name = "merged_out_2048.ttf";
	f3.size = 16_pt;
	*/

	Font fNum("../fonts2/merged_out_2048_53.ttf", 12_pt);
	//fNum.name = "../fonts/arial.ttf";
	//fNum.name = "../fonts/NotoSans-Regular.ttf";


	RenderSettings r;	
	r.deviceW = g_width;
	r.deviceH = g_height;
	
	FontBuilderSettings fs;
	fs.screenDpi = 260;
	fs.textureW = 512;
	fs.textureH = 512;
	fs.screenScale = 1.0;
	fs.fonts = fonts;

	//fs.fonts = { f, f2, f3 };
	//fr = new StringRenderer(fs, r);
	//fr = new StringRenderer(fs, r);
	fr = StringRenderer::CreateSingleColor({ 1,0,1,1 }, fs, r);
	//fr = new StringRenderer({ fNum }, r);
	//fr = new StringRenderer({ f4 }, r);

	fs.fonts = { fArial };
	fn = new NumberRenderer(fs, r);

	
	fr->SetCaption(UTF8_TEXT(u8"\U0001F300"), 10);
	fr->SetCaption(UTF8_TEXT(u8"\U00002b55"), 0);
	fr->SetCaption(UTF8_TEXT(u8"*"), 0);
	
	
	//fr->AddStringCaption(u8"P¯Ìliö\nûluùouËk˝\nk˘Ú", 0.5f, 0.5f, { 1,1,0,1 });
	//fr->AddStringCaption(u8"AbBd", 0.5f, 0.5f, { 1,1,0,1 });
	//fr->Render();
	//fr->GetFontBuilder()->Save("D://88.png");
	fr->SetNewLineOffset(0);
}




#include <unicode\ucnv.h>

void Normalize()
{
	//http://unicode.org/reports/tr15/
	//UnicodeString u8str = UTF8_TEXT(u8"\u4e0a\u6d77 P¯Ìliö ûluùouËk˝ k˘Ú ˙pÏl Ô·belskÈ Ûdy");
	UnicodeString u8str = UTF8_TEXT(u8"\u0141\u00f3d\u017a");
	
	//std::string ss;
	//u8str.toUTF8String(ss);

	//char * tt = new char[50];
	//UErrorCode pError;
	//ucnv_convert("US-ASCII", "UTF-8", tt, 50, ss.data(), ss.size(), &pError);

	auto it = CustromIteratorCreator::Create(u8str);
	uint32_t c;

	while ((c = it.GetCurrentAndAdvance()) != it.DONE)
	{
		auto uu = UnicodeNormalizer::nfd(c);
		printf("%c -> %i / %c \n", c, uu.length(), uu[0]);

		if (uu[0] >= 128)
		{
			printf("need ascii");
		}
		/*
		std::u32string uu;
		uu.push_back(c);
		//ufal::unilib::uninorms::nfc(uu);
		ufal::unilib::uninorms::nfd(uu);

		printf("%c -> %i / %c \n", c, uu.size(), uu[0]);
		*/

		//ufal::unilib::uninorms::nfkc(uu);
		//ufal::unilib::uninorms::nfkd(uu);
	}
}

int main(int argc, char ** argv)
{

#if __has_include(<vld.h>)
	VLDSetReportOptions(VLD_OPT_REPORT_TO_DEBUGGER | VLD_OPT_REPORT_TO_FILE, L"leaks.txt");
#endif
		
	//Normalize();
	/*
	//CharacterExtractor cr({ "arial.ttf" }, "arial_out.ttf");
	//CharacterExtractor cr(std::vector<std::string>({ "arial.ttf" }), "arial_out.ttf");
	CharacterExtractor cr({ "../ii_v3/noto_max_priority/", "../ii_v3/noto/", "../ii_v3/noto-otf/", "../ii_v3/emoji/"  }, "merged_out");
	//CharacterExtractor cr({ "../ii/noto_max_priority/", "../ii/noto/" }, "merged_out");
	//CharacterExtractor cr({ "../ii/noto_max_priority/" }, "merged_out");
	
	cr.SetOutputDir("../ii_v7/");
	cr.AddText(UTF8_TEXT(u8"P¯Ìliö\nûluùouËk˝\nk˘Ú")); //CZ accents	
	cr.AddText(UTF8_TEXT(u8"\u2022")); //mark in number renderer
	cr.AddText(UTF8_TEXT(u8"0123456789")); //all numbers
	cr.AddText(UTF8_TEXT(u8" !/*-+,.=")); //basic math operators	
	cr.AddText(UTF8_TEXT(u8"\U0001F300")); //CYCLONE "icon"
	cr.AddText(UTF8_TEXT(u8"\U00002B55")); //Donut "icon" (https://www.compart.com/en/unicode/U+2B55)
	//cr.AddDirectory("D:\\Martin\\Programming\\test\\Ventusky\\VentuskyWin\\_bundle_dir_\\DATA\\cities\\");	
	cr.AddDirectory("d:/Martin/Programming/test/Ventusky/Ventusky_v11/Preprocessing/GeneratedCities/cities/");
	
	cr.AddDirectory("d:/Martin/Programming/test/Ventusky/Ventusky_v11/_bundle_dir_/DATA/localization/");
	cr.AddTextFromJsonFile("", [](const char * str, CharacterExtractor * ce) -> void {
		
	});
	//cr.RemoveChar(utf8_string(u8"P")[0]);
	allChars = cr.GetAllCharacters();
	
	cr.GenerateScript("run.sh");
	cr.Release();
	*/

	/*		
	rrr = generateRandomString();
	rrr += "\n";
	rrr += generateRandomString();
	*/
	//auto dd = generateRandomString();
	//auto dd2 = generateRandomString();

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