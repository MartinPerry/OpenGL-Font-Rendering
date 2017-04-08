# OpenGL-Font-Rendering
Rendering UNICODE fonts with OpenGL 

This library is still work-in-progress. This is a working beta version.

Overview
------------------------------------------

There are plenty of FontRendering libraries. 
* https://github.com/rougier/freetype-gl - also with UNICODE, too complex and not easily "bend" to specified task
* https://github.com/tlorach/OpenGLText - only ASCII


However, many of them are only for ASCII characters, they are too complex, or not suitable for OpenGL ES.
This library is using FreeType to generate glyphs. 
Unlike other libraries, the font texture is not generated once. Due to the UNICODE, they can be very large amount of characters and
they cannot be stored in one texture. Generate many textures for different alphabets is also not possible, 
because we can render letters from different lphabets together.
This library generates texture in runtime. There is caching, so texture is updated only if new characters are added. 
Also, the geometry of entire text is generated in runtime and stored in VBO. 
Based on this, final rendering is done with only one draw-call.

There is a demo program `example_demo.cpp`. It is using [FreeGlut](http://freeglut.sourceforge.net/) library. This library is needed only for this demo.

Simple example
------------------------------------------
````c++
StringRenderer * fr = new StringRenderer(g_width, g_height, { "arial.ttf", fontPixelSize, cacheTextureW, cacheTextureH });

fr->AddString(u8"Příliš\nžluťoučký\nkůň", posX, posY, { 1,1,0,1 }, FontRenderer::CENTER, FontRenderer::ALIGN_CENTER);
		
fr->Render();



NumberRenderer * nr = new NumberRenderer(g_width, g_height, { "arial.ttf", fontPixelSize, cacheTextureW, cacheTextureH });

nr->AddNumber(-45.75, posX, posY, { 1,1,0,1 }, FontRenderer::CENTER);
		
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

References
------------------------------------------
* https://www.freetype.org/
* https://sourceforge.net/projects/tiny-utf8/
* http://utf8everywhere.org/
* https://github.com/JeffBezanson/cutef8


