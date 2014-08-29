#pragma once

#include "psptypes.h"

#define GU_FALSE 0
#define GU_TRUE 1

// enable
enum
{
	GU_BLEND,
	GU_SCISSOR_TEST,
	GU_CLIP_PLANES,
	GU_DEPTH_TEST,
	GU_CULL_FACE,
	GU_TEXTURE_2D,
	GU_LIGHTING
};

// func
enum
{
	GU_LEQUAL
};

// prim
enum
{
	GU_TRIANGLE_FAN,
	GU_POINTS,
	GU_TRIANGLE_STRIP
};

// vertex type
enum
{
	GU_TEXTURE_32BITF = 1,
	GU_VERTEX_32BITF = 2,
	GU_TRANSFORM_3D = 4,
	GU_COLOR_8888 = 8,
	GU_SPRITES = 16,
	GU_TRANSFORM_2D = 32,
	GU_TEXTURE_16BIT = 64,
	GU_VERTEX_16BIT = 128
};

// clear
enum
{
	GU_COLOR_BUFFER_BIT = 1,
	GU_DEPTH_BUFFER_BIT = 2
};

// blend
enum
{
	GU_ADD
};
enum
{
	GU_SRC_COLOR,
	GU_DST_COLOR,
	GU_SRC_ALPHA,
	GU_ONE_MINUS_SRC_ALPHA,
};

// sync
enum
{
	GU_SYNC_FINISH,
	GU_SYNC_DONE
};
enum
{
	GU_SYNC_WHAT_DONE
};

// shade model
enum
{
	GU_SMOOTH
};

// winding
enum
{
	GU_CW,
	GU_CCW
};

// interp
enum
{
	GU_LINEAR,
};

// wrap
enum
{
	GU_REPEAT
};

// tfx
enum
{
	GU_TFX_REPLACE,
	GU_TFX_MODULATE
};

// tcc
enum
{
	GU_TCC_RGB,
	GU_TCC_RGBA
};

// tex map mode
enum
{
	GU_TEXTURE_COORDS
};

// psm
enum
{
	GU_PSM_T8,
	GU_PSM_8888,
	GU_PSM_5650
};

// matrix mode
enum
{
	GU_PROJECTION,
	GU_VIEW,
	GU_TEXTURE,
	GU_MODEL,
};

// display list mode?
enum
{
	GU_DIRECT
};

#define GU_RGBA(r, g, b, a) ((u8)(r) | ((u8)(g) << 8) | ((u8)(b) << 16) | ((u8)(a) << 24))
#define GU_COLOR(r, g, b, a) GU_RGBA((r) * 255.0f, (g) * 255.0f, (b) * 255.0f, (a) * 255.0f)

void sceGuInit(void);
void sceGuTerm(void);

void sceGuDrawBuffer(int psm, const void *buffer, int width);
void sceGuDispBuffer(int w, int h, const void *buffer, int buffer_width);
void sceGuDepthBuffer(const void *buffer, int width);
void sceGuDisplay(int enabled);
int sceGuSwapBuffers(void);

void sceGuStart(int mode, void *display_list);
int sceGuFinish(void);
void *sceGuGetMemory(int size);
void sceGuSync(int sync, int what);

void sceGuOffset(int x, int y);
void sceGuViewport(int x, int y, int w, int h);
void sceGuScissor(int x, int y, int w, int h);

void sceGuEnable(int feature);
void sceGuDisable(int feature);

void sceGuDepthRange(int min, int max);
void sceGuDepthMask(int disabled);
void sceGuDepthFunc(int func);

void sceGuClear(int mask);
void sceGuClearColor(ScePspRGBA8888 c);
void sceGuClearDepth(u16 depth);

void sceGuBlendFunc(int combine, int src_func, int dst_func, int a, int b);

void sceGuClutMode(int psm, int a, int b, int c);
void sceGuClutLoad(int blocks, const ScePspRGBA8888 *palette);

void sceGuTexFlush(void);
void sceGuTexImage(int a, int w, int h, int buf_w, const void *pixels);
void sceGuTexFilter(int a, int b);
void sceGuTexFunc(int tfx, int tcc);
void sceGuTexMode(int psm, int a, int b, int c);
void sceGuTexWrap(int s, int t);
void sceGuTexMapMode(int mode, int a, int b);
void sceGuTexScale(float s, float t);
void sceGuTexOffset(float s, float t);

void sceGuColor(ScePspRGBA8888 c);
void sceGuShadeModel(int model);

void sceGuFrontFace(int face);
