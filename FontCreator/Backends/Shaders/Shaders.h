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
	uniform vec4 fontColor;\n\
	\n\
    void main()\n\
    {\n\
        float distance = texture2D( fontTex, texCoord.xy ).x; \n\
        gl_FragColor.rgba = fontColor; \n\
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
	\n\
    void main()\n\
    {\n\
        gl_FragColor.rgb = color.xyz; \n\
        gl_FragColor.a = color.w * texture2D( fontTex, texCoord.xy ).x;\n\
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
    varying vec2 texCoord;\n\
    uniform sampler2D fontTex;\n\
	uniform vec4 fontColor;\n\
	\n\
    void main()\n\
    {\n\
        gl_FragColor.rgba = fontColor; \n\
        gl_FragColor.a *= texture2D( fontTex, texCoord.xy ).x;\n\
    }\n\
" };

//============================================================

static const char* BACKGROUND_VERTEX_SHADER_SOURCE = {
    "\n\
    attribute vec2 POSITION;\n\
	\n\
    void main()\n\
    {\n\
        gl_Position = vec4(POSITION.x, POSITION.y, 0.0, 1.0); \n\
    }\n\
" };

static const char* BACKGROUND_PIXEL_SHADER_SOURCE = {
    "\n\
    uniform vec4 bgColor;\n\
	\n\
    void main()\n\
    {\n\
        gl_FragColor.rgba = bgColor; \n\
    }\n\
" };

#endif

#endif
