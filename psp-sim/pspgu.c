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

void sceGuInit(void)
{
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	window = SDL_CreateWindow("PSP Sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 272, SDL_WINDOW_OPENGL);
	assert(window != NULL);

	context = SDL_GL_CreateContext(window);
	assert(context != NULL);

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
	//glScissor(x, y, w, h);
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
	//glDepthRange(min / (float)UINT16_MAX, max / (float)UINT16_MAX);
}

void sceGuDepthMask(int disabled)
{
	//glDepthMask(disabled ? GL_FALSE : GL_TRUE);
}

void sceGuDepthFunc(int func)
{
	//glDepthFunc(func);
}

void sceGuClear(int mask)
{
	glClear(mask);
}

void sceGuClearColor(ScePspRGBA8888 c)
{
	//glClearColor((c & 0xff) / 255.0f, ((c >> 8) & 0xff) / 255.0f, ((c >> 16) & 0xff) / 255.0f, ((c >> 24) & 0xff) / 255.0f);
}

void sceGuClearDepth(u16 depth)
{
	glClearDepth(depth / (float)UINT16_MAX);
}

void sceGuBlendFunc(int combine, int src_func, int dst_func, int a, int b)
{
	//glBlendFunc(src_func, dst_func);
}

void sceGuClutMode(int psm, int a, int b, int c)
{
}

void sceGuClutLoad(int blocks, const ScePspRGBA8888 *palette)
{
}

void sceGuTexFlush(void)
{
}

void sceGuTexImage(int a, int w, int h, int buf_w, const void *pixels)
{
}

void sceGuTexFilter(int a, int b)
{
}

void sceGuTexFunc(int tfx, int tcc)
{
	//glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, tfx);
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
	//glColor4ubv((const GLubyte *)&c);
}

void sceGuShadeModel(int model)
{
	//glShadeModel(model);
}

void sceGuFrontFace(int face)
{
	glFrontFace(face);
}
