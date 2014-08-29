#pragma once

#include "pspgu.h"

#include <stdlib.h>

char *display_list_base;
static int display_list_used;

void sceGuInit(void)
{
}

void sceGuTerm(void)
{
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
}

void sceGuScissor(int x, int y, int w, int h)
{
}

void sceGuEnable(int feature)
{
}

void sceGuDisable(int feature)
{
}

void sceGuDepthRange(int min, int max)
{
}

void sceGuDepthMask(int disabled)
{
}

void sceGuDepthFunc(int func)
{
}

void sceGuClear(int mask)
{
}

void sceGuClearColor(ScePspRGBA8888 c)
{
}

void sceGuClearDepth(u16 depth)
{
}

void sceGuBlendFunc(int combine, int src_func, int dst_func, int a, int b)
{
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
}

void sceGuShadeModel(int model)
{
}

void sceGuFrontFace(int face)
{
}
