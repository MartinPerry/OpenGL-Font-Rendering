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

#if defined(__APPLE__) || defined(__ANDROID_API__)
#   define VS_CODE_3(x) "#version 300 es\nprecision highp float; " #x
#   define PS_CODE_3(x) "#version 300 es\nprecision highp float; " #x
#else
#   define VS_CODE_3(x) #x
#   define PS_CODE_3(x) #x
#endif

//============================================================
// Default shaders
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
    varying vec2 texCoord;
    varying vec4 color;

    uniform sampler2D fontTex;    

    void main()
    {
        gl_FragColor.rgb = color.xyz;
        gl_FragColor.a = color.w * texture2D(fontTex, texCoord.xy).x;
    }
);

//============================================================
// Default SDF shaders
//============================================================

static const char* DEFAULT_SDF_VERTEX_SHADER_SOURCE = VS_CODE_3(
    in vec2 POSITION;
    in vec2 TEXCOORD0;
    in vec4 COLOR;
    
    out vec2 texCoord;
    out vec4 color;

    void main()
    {
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0);
        texCoord = TEXCOORD0;
        color = COLOR;
    }
);

static const char* DEFAULT_SDF_PIXEL_SHADER_SOURCE = PS_CODE_3(
    in vec2 texCoord;
    in vec4 color;

    out vec4 fragColor;
                                                               
    uniform sampler2D fontTex;
    uniform float uSoftness; // 0.0 = sharpest, larger values = softer.
    uniform float uEdge; //default: 0.5
        
    void main()
    {       
        float val = texture(fontTex, texCoord.xy).x;
        
        // Screen-space antialiasing width
        float w = fwidth(val) + uSoftness;

        // Coverage from signed distance
        float alpha = smoothstep(uEdge - w, uEdge + w, val);

        fragColor = vec4(color.rgb, color.a * alpha);
        //fragColor = color * alpha; // premultiplied alpha
        
    }
);

static const char* DEFAULT_SDF_OUTLINE_PIXEL_SHADER_SOURCE = PS_CODE_3(
    in vec2 texCoord;
    in vec4 color;

    out vec4 fragColor;
                                                                       
    uniform sampler2D fontTex;
    uniform float uSoftness;
    uniform float uEdge;
    uniform vec4 uOutlineColor;
    uniform float uOutlineWidth;
       
    void main()
    {        
        float val = texture(fontTex, texCoord.xy).x;

        float w = fwidth(val) + uSoftness;

        float fillAlpha = smoothstep(uEdge - w, uEdge + w, val);
        float outlineAlpha = smoothstep((uEdge - uOutlineWidth) - w,
            (uEdge - uOutlineWidth) + w,
            val);

        vec4 finalColor = mix(uOutlineColor, color, fillAlpha);
        float alpha = max(fillAlpha, outlineAlpha) * color.a;

        fragColor = vec4(finalColor.rgb, finalColor.a * alpha);
        //fragColor = finalColor * alpha; // premultiplied alpha
    }
);

//============================================================
// Single color special
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
// Single color SDF
//============================================================

static const char* SINGLE_COLOR_SDF_VERTEX_SHADER_SOURCE = VS_CODE_3(
    in vec2 POSITION;
    in vec2 TEXCOORD0;
    
    out vec2 texCoord;
    
    void main()
    {
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0);
        texCoord = TEXCOORD0;
    }
);


static const char* SINGLE_COLOR_SDF_PIXEL_SHADER_SOURCE = PS_CODE_3(
    in vec2 texCoord;
    
    out vec4 fragColor;
                                                                    
    uniform sampler2D fontTex;
    uniform float uSoftness; // 0.0 = sharpest, larger values = softer.
    uniform float uEdge; //default: 0.5    
    uniform vec4 fontColor;
    
    void main()
    {
        float val = texture(fontTex, texCoord.xy).x;

        // Screen-space antialiasing width
        float w = fwidth(val) + uSoftness;

        // Coverage from signed distance
        float alpha = smoothstep(uEdge - w, uEdge + w, val);

        fragColor = vec4(fontColor.rgb, fontColor.a * alpha);
        //fragColor = fontColor * alpha; // premultiplied alpha

    }
);

static const char* SINGLE_COLOR_SDF_OUTLINE_PIXEL_SHADER_SOURCE = PS_CODE_3(
    in vec2 texCoord;

    out vec4 fragColor;
                                                                            
    uniform sampler2D fontTex;
    uniform float uSoftness;
    uniform float uEdge;
    uniform vec4 uOutlineColor;
    uniform float uOutlineWidth;    
    uniform vec4 fontColor;

    void main()
    {
        float val = texture(fontTex, texCoord.xy).x;

        float w = fwidth(val) + uSoftness;

        float fillAlpha = smoothstep(uEdge - w, uEdge + w, val);
        float outlineAlpha = smoothstep((uEdge - uOutlineWidth) - w,
            (uEdge - uOutlineWidth) + w,
            val);

        vec4 finalColor = mix(uOutlineColor, fontColor, fillAlpha);
        float alpha = max(fillAlpha, outlineAlpha) * fontColor.a;

        fragColor = vec4(finalColor.rgb, finalColor.a * alpha);
        //fragColor = finalColor * alpha; // premultiplied alpha
    }
);

//============================================================
// Backgrounds
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

