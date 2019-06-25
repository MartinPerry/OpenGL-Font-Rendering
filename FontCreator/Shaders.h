#ifndef SHADERS_H
#define SHADERS_H


//=============================================================================
// Shaders
//=============================================================================

#if defined(__APPLE__) || defined(__ANDROID_API__)
static const char* DEFAULT_VERTEX_SHADER_SOURCE = {
	"\n\
	precision highp float;\n\
    attribute vec2 POSITION;\n\
    attribute vec2 TEXCOORD0;\n\
	attribute vec4 COLOR;\n\
    varying vec2 texCoord;\n\
	varying vec4 color;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
        texCoord = TEXCOORD0; \n\
		color = COLOR; \n\
    }\n\
" };

static const char* DEFAULT_PIXEL_SHADER_SOURCE = {
	"\n\
	precision highp float;\n\
    uniform sampler2D fontTex;\n\
    varying vec2 texCoord;\n\
	varying vec4 color;\n\
	\n\
    void main()\n\
    {\n\
        float distance = texture2D( fontTex, texCoord.xy ).x; \n\
        gl_FragColor.rgb = color.xyz; \n\
        gl_FragColor.a = color.w * distance;\n\
    }\n\
" };

//============================================================

static const char* SINGLE_COLOR_VERTEX_SHADER_SOURCE = {
	"\n\
	precision highp float;\n\
    attribute vec2 POSITION;\n\
    attribute vec2 TEXCOORD0;\n\
    varying vec2 texCoord;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
        texCoord = TEXCOORD0; \n\
    }\n\
" };

static const char* SINGLE_COLOR_PIXEL_SHADER_SOURCE = {
	"\n\
	precision highp float;\n\
    uniform sampler2D fontTex;\n\
    varying vec2 texCoord;\n\
	\n\
    void main()\n\
    {\n\
        float distance = texture2D( fontTex, texCoord.xy ).x; \n\
        gl_FragColor.rgba = [SINGLE_COLOR]; \n\
        gl_FragColor.a *= distance;\n\
    }\n\
" };

#else
static const char* DEFAULT_VERTEX_SHADER_SOURCE = {
	"\n\
    attribute vec2 POSITION;\n\
    attribute vec2 TEXCOORD0;\n\
	attribute vec4 COLOR;\n\
    varying vec2 texCoord;\n\
	varying vec4 color;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
        texCoord = TEXCOORD0; \n\
		color = COLOR; \n\
    }\n\
" };

static const char* DEFAULT_PIXEL_SHADER_SOURCE = {
	"\n\
    uniform sampler2D fontTex;\n\
    varying vec2 texCoord;\n\
	varying vec4 color;\n\
    //out vec4 fragColor;\n\
	\n\
    void main()\n\
    {\n\
        float distance = texture2D( fontTex, texCoord.xy ).x; \n\
        gl_FragColor.rgb = color.xyz; \n\
		//gl_FragColor.rgb = vec3(distance); \n\
        gl_FragColor.a = color.w * distance;\n\
		//gl_FragColor.a += 0.5;\n\
    }\n\
" };

//============================================================

static const char* SINGLE_COLOR_VERTEX_SHADER_SOURCE = {
	"\n\
    attribute vec2 POSITION;\n\
    attribute vec2 TEXCOORD0;\n\
    varying vec2 texCoord;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
        texCoord = TEXCOORD0; \n\
    }\n\
" };

static const char* SINGLE_COLOR_PIXEL_SHADER_SOURCE = {
	"\n\
    uniform sampler2D fontTex;\n\
    varying vec2 texCoord;\n\
	\n\
    void main()\n\
    {\n\
        float distance = texture2D( fontTex, texCoord.xy ).x; \n\
        gl_FragColor.rgba = [SINGLE_COLOR]; \n\
        gl_FragColor.a *= distance;\n\
    }\n\
" };

#endif

#endif
