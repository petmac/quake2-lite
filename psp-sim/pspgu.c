#pragma once

#include "pspgu.h"

#include <SDL_events.h>
#include <SDL_video.h>

#include <assert.h>

#include <stdlib.h>

static SDL_Window *window;
static SDL_GLContext context;
static char *display_list_base;
static int display_list_used;

typedef struct texture_state_s
{
	u8 clut[256][4];

	int w;
	int h;
	int buf_w;
	const u8 *pixels;
} texture_state_t;

static texture_state_t tex;

static void upload_texture(void)
{
	const int count = tex.w * tex.h;

	static u8 rgba[512 * 256][4];

	for (int i = 0; i < count; ++i)
	{
		const u8 *const src = tex.clut[tex.pixels[i]];
		u8 *const dst = rgba[i];

		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

void sceGuInit(void)
{
	const int scale = 2;

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	window = SDL_CreateWindow("PSP Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480 * scale, 272 * scale, SDL_WINDOW_OPENGL);
	assert(window != NULL);

	context = SDL_GL_CreateContext(window);
	assert(context != NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glEnableClientState(GL_VERTEX_ARRAY);
}

void sceGuTerm(void)
{
	SDL_GL_DeleteContext(context);
	context = NULL;

	SDL_DestroyWindow(window);
	window = NULL;
}

void sceGuDrawBuffer(int psm, const void *buffer, int width)
{
}

void sceGuDispBuffer(int w, int h, const void *buffer, int buffer_width)
{
}

void sceGuDepthBuffer(const void *buffer, int width)
{
}

void sceGuDisplay(int enabled)
{
}

int sceGuSwapBuffers(void)
{
	SDL_Event e;

	SDL_GL_SwapWindow(window);

	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
		{
			exit(0);
		}
	}

	return 0;
}

void sceGuStart(int mode, void *display_list)
{
	display_list_base = (char *)display_list;
	display_list_used = 0;
}

int sceGuFinish(void)
{
	return display_list_used;
}

void *sceGuGetMemory(int size)
{
	void *p = display_list_base + display_list_used;
	display_list_used += size;

	return p;
}

void sceGuSync(int sync, int what)
{
}

void sceGuOffset(int x, int y)
{
}

void sceGuViewport(int x, int y, int w, int h)
{
	//glViewport(x, y, w, h);
}

void sceGuScissor(int x, int y, int w, int h)
{
	glScissor(x, y, w, h);
}

void sceGuEnable(int feature)
{
	glEnable(feature);
}

void sceGuDisable(int feature)
{
	glDisable(feature);
}

void sceGuDepthRange(int min, int max)
{
	glDepthRange(min / (float)UINT16_MAX, max / (float)UINT16_MAX);
}

void sceGuDepthMask(int disabled)
{
	glDepthMask(disabled ? GL_FALSE : GL_TRUE);
}

void sceGuDepthFunc(int func)
{
	glDepthFunc(func);
}

void sceGuClear(int mask)
{
	glClear(mask);
}

void sceGuClearColor(ScePspRGBA8888 c)
{
	glClearColor((c & 0xff) / 255.0f, ((c >> 8) & 0xff) / 255.0f, ((c >> 16) & 0xff) / 255.0f, ((c >> 24) & 0xff) / 255.0f);
}

void sceGuClearDepth(u16 depth)
{
	glClearDepth(depth / (float)UINT16_MAX);
}

void sceGuBlendFunc(int combine, int src_func, int dst_func, int a, int b)
{
	glBlendFunc(src_func, dst_func);
}

void sceGuClutMode(int psm, int a, int b, int c)
{
}

void sceGuClutLoad(int blocks, const ScePspRGBA8888 *palette)
{
	memcpy(tex.clut, palette, blocks * 8 * sizeof(ScePspRGBA8888));
}

void sceGuTexFlush(void)
{
	upload_texture();
}

void sceGuTexImage(int mipmap, int w, int h, int buf_w, const void *pixels)
{
	tex.w = w;
	tex.h = h;
	tex.buf_w = buf_w;
	tex.pixels = pixels;

	sceGuTexFlush();
}

void sceGuTexFilter(int a, int b)
{
}

void sceGuTexFunc(int tfx, int tcc)
{
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, tfx);
}

void sceGuTexMode(int psm, int a, int b, int c)
{
}

void sceGuTexWrap(int s, int t)
{
}

void sceGuTexMapMode(int mode, int a, int b)
{
}

void sceGuTexScale(float s, float t)
{
}

void sceGuTexOffset(float s, float t)
{
}

void sceGuColor(ScePspRGBA8888 c)
{
	glColor4ubv((const GLubyte *)&c);
}

void sceGuShadeModel(int model)
{
	glShadeModel(model);
}

void sceGuFrontFace(int face)
{
	glFrontFace(face);
}

void sceGuDrawArray(int prim, int type, int numverts, const void *indices, const void *vertices)
{
	const float *floats = (const float *)vertices;

	switch (type)
	{
	case GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D:
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 20, floats);
		glVertexPointer(3, GL_FLOAT, 20, &floats[2]);
		break;

	default:
		return;
	}

	glDrawArrays(prim, 0, numverts);
}
