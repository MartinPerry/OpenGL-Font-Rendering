#ifndef SHADERS_H
#define SHADERS_H


//=============================================================================
// Shaders
//=============================================================================

#if defined(__APPLE__) || defined(__ANDROID_API__)
#   define VS_CODE(x) "precision highp float; " #x
#   define PS_CODE(x) "precision highp float; " #x
#else
#   define VS_CODE(x) #x
#   define PS_CODE(x) #x
#endif

//============================================================
// Shaders code
//============================================================

static const char* DEFAULT_VERTEX_SHADER_SOURCE = VS_CODE(
    attribute vec2 POSITION;
    attribute vec2 TEXCOORD0;
    attribute vec4 COLOR;
    varying vec2 texCoord;
    varying vec4 color;

    void main()
    {
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0);
        texCoord = TEXCOORD0;
        color = COLOR;
    }
);

static const char* DEFAULT_PIXEL_SHADER_SOURCE = PS_CODE(
    uniform sampler2D fontTex;
    varying vec2 texCoord;
	varying vec4 color;
	
    void main()
    {
        gl_FragColor.rgb = color.xyz;
        gl_FragColor.a = color.w * texture2D( fontTex, texCoord.xy ).x;
    }
);

//============================================================

static const char* SINGLE_COLOR_VERTEX_SHADER_SOURCE = VS_CODE(
    attribute vec2 POSITION;
    attribute vec2 TEXCOORD0;
    varying vec2 texCoord;
	
    void main()
    {
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0);
        texCoord = TEXCOORD0;
    }
);

static const char* SINGLE_COLOR_PIXEL_SHADER_SOURCE = PS_CODE(
    varying vec2 texCoord;
    uniform sampler2D fontTex;
	uniform vec4 fontColor;
	
    void main()
    {
        gl_FragColor.rgba = fontColor;
        gl_FragColor.a *= texture2D( fontTex, texCoord.xy ).x;
    }
);

//============================================================

static const char* BACKGROUND_VERTEX_SHADER_SOURCE = VS_CODE(
    attribute vec2 POSITION;
    attribute vec4 COLOR;        
    varying vec4 color;

    void main()
    {
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0);        
        color = COLOR;
    }
);

static const char* BACKGROUND_PIXEL_SHADER_SOURCE = PS_CODE(    
    varying vec4 color;

    void main()
    {        
        gl_FragColor.rgba = color;     
    }
);

//============================================================

static const char* BACKGROUND_SHADOW_VERTEX_SHADER_SOURCE = VS_CODE(
    attribute vec2 POSITION;
    attribute vec4 COLOR;
    attribute vec4 AABB;
    varying vec2 pos;
    varying vec4 aabb;
    varying vec4 color; 

    void main()
    {
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0);
        pos = POSITION;
        aabb = AABB;
        color = COLOR;
    }
);

static const char* BACKGROUND_SHADOW_PIXEL_SHADER_SOURCE = PS_CODE(
    varying vec2 pos;
    varying vec4 aabb;
    varying vec4 color;
	
    vec2 mapTo01(vec2 s, vec2 from1, vec2 from2){ 
        return (s - from1) * vec2(1.0) / (from2 - from1); 
    }

    void main()
    {
        vec2 xy01 = mapTo01(pos, aabb.xy, aabb.zw);
        float dist = ((0.5 - xy01.x) * (0.5 - xy01.x) + (0.5 - xy01.y) * (0.5 - xy01.y));        
        gl_FragColor.rgba = color;
        gl_FragColor.rgb *= (1.0 - dist);
    }
);

//============================================================

static const char* SINGLE_COLOR_BACKGROUND_VERTEX_SHADER_SOURCE = VS_CODE(
    attribute vec2 POSITION;	

    void main()
    {
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0);
    }
);

static const char* SINGLE_COLOR_BACKGROUND_PIXEL_SHADER_SOURCE = PS_CODE(
    uniform vec4 bgColor;	

    void main()
    {
        gl_FragColor.rgba = bgColor;
    }
);

#endif

