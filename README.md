# OpenGL-Font-Rendering
Rendering UNICODE fonts

This library is still work-in-progress. This is a working beta version.

Overview
------------------------------------------

The library offers a simple font rendering system. 
The user can change rendering backend to a different systems - OpenGL and offline, image-based, rendering is suuported out of the box.
However, DirectX, Vulkan and other systems can be simply added.

Why to create another font rendering library, if there are plenty of FontRendering libraries. 
To name some of them:
* https://github.com/rougier/freetype-gl - also with UNICODE, too complex and not easily "bend" to specified task
* https://github.com/tlorach/OpenGLText - only ASCII


Some disadvantages of existing solutions are:
* only support for ASCII characters
* too complex solutions
* not supporting multiple font families at once 
* not suitable for OpenGL ES
* not an easy possibility to change rendering backend

This library is using FreeType to generate glyphs. 
Unlike other libraries, the font texture is not generated once. 
Due to the UNICODE, there can be very large amount of characters and these cannot be stored in one texture. 
Generate many textures for different alphabets is also not possible, because we can render letters from different alphabets together.
This library generates texture in runtime. There is a caching, so texture is updated only if new characters are added. 
Also, the geometry of entire text is generated in runtime (for OpenGL, it is stored for example in VBO). 
Based on this, final rendering is done with only one draw-call.

If you want to use Bidirectional strings (eg. Arabic), you can use ICU (http://site.icu-project.org/).
In that case `#define USE_ICU_LIBRARY` must be enabled (in `ExternalIncludes.h`).
By default, ICU is disabled and simle `std::u8string` is used to store UTF-8 coded strings.


This library supports multiple fonts to be loaded at once. If multiple fonts are used, the order at which they are added is used as their priority. 
If letter is look for, fonts are iterated by priority. First occurrence is returned even if the same letter can be in multiple fonts.

There is a demo program `example_demo.cpp`. It uses [FreeGlut](http://freeglut.sourceforge.net/) library. This library is needed only for this demo.

Simple example
------------------------------------------
````c++

Font f;
f.name = "some_font.ttf";	
f.size = 40_pt;

Font f2;
f2.name = "traditional_chinese.ttf";	
f2.size = 40_px;

Font f3;
f3.name = "some_other_font.ttf";	
f3.size = 1.0_em; //em size of font -> pixel size = size_em * defaultFontSizeInPx
f3.defaultFontSizeInPx = 16; //default font size for OS, can usually be obtained via OS API

std::vector<Font> fonts;
fonts.push_back(f);
fonts.push_back(f2);
fonts.push_back(f3);

RenderSettings r;
r.deviceW = 1024; //screen width in pixels
r.deviceH = 768; //screen height in pixels
r.useTextureLinearFilter = true; //use linear filtering for texture in OpenGL

FontBuilderSettings fs;
fs.fonts = fonts;
fs.screenDpi = 0; //if 0 => will use size directly in pixels, otherwise use dpi and size is in pt
fs.screenScale = 1.0; //usually used on iOS devices - screen scale is taken from UIScreen.main.nativeScale
fs.textureW = 512; //cache texture width
fs.textureH = 512; //cache texture height

/*
//Optionally, SDF can be used to render fonts
fs.sdf = SDF();
fs.sdf->outlineColor = { 0, 0, 0, 1 };
fs.sdf->outlineWidth = 0.1f;
fs.sdf->softness = 0.05f;
*/

//====================================================
// Basic string renderer
//====================================================

StringRenderer* fr = StringRenderer::CreateSingleColor({ 1,0,1,1 }, fs, r);	
fr->AddString(UTF8_TEXT(u8"Příliš\nžluťoučký\nkůň"), posX, posY, { 1,1,0,1 }, AbstractRenderer::CENTER, AbstractRenderer::ALIGN_CENTER);
fr->AddStringCaption(UTF8_TEXT(u8"\u0633\u0644\u0627\u0645"), posX, posY, { 1,1,0,1 }); //Some Arabic text
fr->Render();

//====================================================
// Specialized faster renderer for numbers only
//====================================================

NumberRenderer* nr = NumberRenderer::CreateDefault(fs, r);
nr->AddNumber(-45.75, posX, posY, { 1,1,0,1 }, AbstractRenderer::CENTER);		
nr->Render();

//====================================================
// Custom renderer for glyphs loaded from texture
//====================================================

std::vector<CustomGlyph> gd;	
CustomGlyph g;
g.c = 'a';
g.fileName = "some_a_image.png";
g.referenceCharCode = 'x';
g.referenceFont = fArial;
gd.push_back(g);
	
g.c = 'b';
g.fileName = "some_b_image.png";
g.referenceCharCode = 'o';
g.referenceFont = fArial;
gd.push_back(g);

CustomFontBuilderSettings ifs;
ifs.textureW = 256;
ifs.textureH = 256;
ifs.screenDpi = 260;
ifs.screenScale = 1;	
ifs.channelsCount = 4;

auto cfb = std::make_shared<CustomImageFontBuilder>(gd, ifs);
	
auto sm = std::make_shared<ColoredFontShaderManager>();
auto backend = std::make_unique<BackendOpenGL>(r, 3, nullptr, nullptr, sm);

srCustom = new StringRenderer(cfb, std::move(backend));

````

For OpenGL rendering, there is `BackendOpenGL` class. 
For the offline, image-based, output, `BackendImage` class can be used.
If user want to add another output system, it is extended from `BackendBase` class.

Custom `CustomImageFontBuilder` can be used if we want to render glyphs from our own texture.
During setup, we specify which letter (char code) is represented by our texture.
We can also specify font and font letter (char code) that we want to 
use as an original letter that will be replaced. From this,
glyph sizes are obtained and used for our texture.

Texture packing
------------------------------------------

Fonts are packed in texture. There are two algorithms for packing. 
* Fast grid packing - size for all letters is computed and all bins have the same size.
* Slower Tight packing - texture is divided to bins based on letter size. Letters are sort from ones with the biggest size to small ones.
This packing will not use entire texture. They will be holes and sometimes more then 30% of texture can be "empty". However, based on
input characters, even this sparse texture can hold more characters than gridded one. Approximately 2x slower than grid packing.
This algorithm is similar to the one referred as "Guillotine algorithm" in http://clb.demon.fi/files/RectangleBinPack.pdf.


Character extractor utility
------------------------------------------
There is a class `CharacterExtractor` in directory `Utils`. 
This class will generate `.sh` script that can extract and merge fonts with use of `pyftsubset` and `pyftmerge` from https://github.com/fonttools/fonttools.

````c++
CharacterExtractor cr({ "./noto/", "./noto-otf/"  }, "merged_out"); //specify font directory and output font base name
cr.SetOutputDir("./output/"); //set output directory (must exist)
cr.AddText(UTF8_TEXT(u8"žluťoučký")); //add text
cr.AddDirectory("./data/"); //add all files from directory (load all files as UTF8 texts)
cr.GenerateScript("run.sh"); //generate script and save it to run.sh
cr.Release();	
````	

For emojis:

* Take SVG files of emojis you want (for example: https://github.com/googlefonts/noto-emoji - in svg directory).
* Upload them to https://icomoon.io/app/
* Generate ttf font from uploaded SVGs and download
* You can see the content of file online on https://fontdrop.info/


References
------------------------------------------
* https://www.freetype.org/
* https://sourceforge.net/projects/tiny-utf8/
* http://utf8everywhere.org/
* https://github.com/JeffBezanson/cutef8
* https://blog.mapbox.com/improving-arabic-and-hebrew-text-in-map-labels-fd184cf5ebd1
* http://site.icu-project.org/


