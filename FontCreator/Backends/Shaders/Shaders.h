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
    gl_FragColor.a = color.w * texture2D(fontTex, texCoord.xy).x;
}
);

//============================================================

//https://www.redblobgames.com/articles/sdf-fonts/

static const char* DEFAULT_SDF_PIXEL_SHADER_SOURCE = PS_CODE(
    uniform sampler2D fontTex;
    uniform float uSoftness; // 0.0 = sharpest, larger values = softer.
    uniform float uEdge; //default: 0.5
    varying vec2 texCoord;
	varying vec4 color;
	
    //value is in 0 - 1
    //return distance - + = inside, 0 = border, - = outside
    float map_sdf_to_float(float value)
    {
        float spread = 1.0;

        float signed_dist = 2.0 * value - 1.0;
        return signed_dist * spread;
    }

    void main()
    {
        /*
        float val = texture2D(fontTex, texCoord.xy).x;
        float sdf = map_sdf_to_float(val);
        
        
        //float distance_em = mix(u_aemrange[1], u_aemrange[0], texel);
        
        float v = sdf < uEdge ? 0.0 : 1.0;
        
        float inverse_width = uSoftness; //u_screen_px_scale* u_antialias_per_em;
        float opacity = 1.0 - clamp((uEdge - sdf) * inverse_width + 0.5, 0.0, 1.0);
        gl_FragColor = color * opacity; // premultiplied alpha
        */
       
        
        float val = texture2D(fontTex, texCoord.xy).x;
        
        // Screen-space antialiasing width
        float w = fwidth(val) + uSoftness;

        // Coverage from signed distance
        float alpha = smoothstep(uEdge - w, uEdge + w, val);

        gl_FragColor = vec4(color.rgb, color.a * alpha);
        gl_FragColor = color * alpha; // premultiplied alpha             
    }
);

static const char* DEFAULT_SDF_OUTLINE_PIXEL_SHADER_SOURCE = PS_CODE(
    uniform sampler2D fontTex;
    uniform float uSoftness;
    uniform float uEdge;
    uniform vec4 uOutlineColor;
    uniform float uOutlineWidth;
    varying vec2 texCoord;
    varying vec4 color;
   
    void main()
    {        
        float val = texture2D(fontTex, texCoord.xy).x;

        float w = fwidth(val) + uSoftness;

        float fillAlpha = smoothstep(uEdge - w, uEdge + w, val);
        float outlineAlpha = smoothstep((uEdge - uOutlineWidth) - w,
            (uEdge - uOutlineWidth) + w,
            val);

        vec4 finalColor = mix(uOutlineColor, color, fillAlpha);
        float alpha = max(fillAlpha, outlineAlpha) * color.a;

        gl_FragColor = finalColor * alpha; // premultiplied alpha   
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

