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
because we can render letters from different lphabets together.
This library generates texture in runtime. There is caching, so texture is updated only if new characters are added. 
Also, the geometry of entire text is generated in runtime and stored in VBO. 
Based on this, final rendering is done with only one draw-call.

This library supports multiple fonts to be loaded at once. If multiple fonts are used, the order at which they are added is used as their priority. 
If letter is look for, fon ts are iterated by prioroty. First occurence is returned even if the same letter can be in multiple fonts.

There is a demo program `example_demo.cpp`. It is using [FreeGlut](http://freeglut.sourceforge.net/) library. This library is needed only for this demo.

Simple example
------------------------------------------
````c++

Font f;
f.name = "some_font.ttf";	
f.size = 40;

Font f2;
f2.name = "traditional_chinese.ttf";	
f2.size = 40;

RenderSettings r;
r.screenDpi = 0; //if 0 => will use size directly in pixels, otherwise use dpi and size is in pt
r.textureW = 512; //cache texture width
r.textureH = 512; //chache texture height
r.deviceW = 1024; //screen width in pixels
r.deviceH = 768; //screen height in pixels

StringRenderer * fr = new StringRenderer({f, f2}, r);
fr->AddString(u8"Příliš\nžluťoučký\nkůň", posX, posY, { 1,1,0,1 }, AbstractRenderer::CENTER, AbstractRenderer::ALIGN_CENTER);		
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
CharacterExtractor cr("./google_noto_ttf/", "merged_out.ttf"); //specify font directory and output font filename
cr.SetOutputDir("./output/"); //set output directory (must exist)
cr.AddText(u8"žluťoučký"); //add text
cr.AddDirectory("./data/"); //add all files from directory (load all files as UTF8 texts)
cr.GenerateScript("run.sh"); //generate script and save it to run.sh
	
````	


References
------------------------------------------
* https://www.freetype.org/
* https://sourceforge.net/projects/tiny-utf8/
* http://utf8everywhere.org/
* https://github.com/JeffBezanson/cutef8


