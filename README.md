# OpenGL-Font-Rendering
Rendering UNICODE fonts with OpenGL 

This library is still work-in-progress. This is a working beta version.

Overview
------------------------------------------

There are plenty of FontRendering libraries. 
* https://github.com/rougier/freetype-gl - also with UNICODE, too complex and not easily "bend" to specified task
* https://github.com/tlorach/OpenGLText - only ASCII


However, many of them are only for ASCII characters, they are too complex, do not support multiple fonts at once or are not suitable for OpenGL ES.
This library is using FreeType to generate glyphs. 
Unlike other libraries, the font texture is not generated once. Due to the UNICODE, they can be very large amount of characters and
they cannot be stored in one texture. Generate many textures for different alphabets is also not possible, 
because we can render letters from different alphabets together.
This library generates texture in runtime. There is caching, so texture is updated only if new characters are added. 
Also, the geometry of entire text is generated in runtime and stored in VBO. 
Based on this, final rendering is done with only one draw-call.

This library is created with ICU (http://site.icu-project.org/) for handling bidirectional texts and Arabis shaping. We are aware, that not everyone want to use ICU (building it is not straightdorward).
For this reason, ICU can be turned-off and replaced with any string library, that supports Unicode.
We have included TinyUTF8. In ExternalIncludes.h, undefine `USE_ICU_LIBRARY` to turn-off ICU.

Also, you have to change the following lines:

````c++
typedef icu::UnicodeString UnicodeString;

#define FOREACH_32_CHAR_ITERATION(c, str) icu::StringCharacterIterator iter = icu::StringCharacterIterator(str); \
										  for (UChar32 c = iter.first32(); iter.hasNext(); c = iter.next32())

#define BIDI(x) BidiHelper::ConvertOneLine(x)
#define UTF8_TEXT(x) icu::UnicodeString::fromUTF8(x)
#define UTF8_UNESCAPE(x) icu::UnicodeString::fromUTF8(x).unescape()
````

For example, to use the library with TinyUTF8, change lines to:

````c++
typedef utf8_string UnicodeString;

#define FOREACH_32_CHAR_ITERATION(c, str) for (auto c : str)
#define BIDI(x) x
#define UTF8_TEXT(x) x
#define UTF8_UNESCAPE(x) utf8_string::build_from_escaped(x.c_str())
````

Any text, that is being passed to font rendering, must be wrapped inside `UTF8_TEXT(u8"....")`. `UTF8_UNESCAPE` is used only in `CharacterExtractor class`. If you want to 
use elsewhere, use library specific calls.

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

RenderSettings r;
r.screenDpi = 0; //if 0 => will use size directly in pixels, otherwise use dpi and size is in pt
r.screenScale = 1.0; //usually used on iOS devices - screen scale is taken from UIScreen.main.nativeScale
r.textureW = 512; //cache texture width
r.textureH = 512; //cache texture height
r.deviceW = 1024; //screen width in pixels
r.deviceH = 768; //screen height in pixels

StringRenderer * fr = new StringRenderer({f, f2}, r);
fr->AddString(UTF8_TEXT(u8"Příliš\nžluťoučký\nkůň"), posX, posY, { 1,1,0,1 }, AbstractRenderer::CENTER, AbstractRenderer::ALIGN_CENTER);
fr->AddStringCaption(UTF8_TEXT(u8"\u0633\u0644\u0627\u0645"), posX, posY, { 1,1,0,1 }); //Some Arabic text
fr->Render();


NumberRenderer * nr = new NumberRenderer(f, r);
nr->AddNumber(-45.75, posX, posY, { 1,1,0,1 }, AbstractRenderer::CENTER);		
nr->Render();

````


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
	
````	


References
------------------------------------------
* https://www.freetype.org/
* https://sourceforge.net/projects/tiny-utf8/
* http://utf8everywhere.org/
* https://github.com/JeffBezanson/cutef8
* https://blog.mapbox.com/improving-arabic-and-hebrew-text-in-map-labels-fd184cf5ebd1
* http://site.icu-project.org/


