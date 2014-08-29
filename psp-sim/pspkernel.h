#pragma once

#include "psptypes.h"

#include <SDL_main.h>

#define PSP_MODULE_INFO(a, b, c, d)
#define PSP_MAIN_THREAD_ATTR(x)

int sceKernelCreateCallback(const char *a, void *b, void *c);
void sceKernelRegisterExitCallback(int a);

int sceKernelCreateThread(const char *a, void *b, int c, int d, int e, int f);
void sceKernelStartThread(int id, int a, int b);
void sceKernelSleepThreadCB(void);

void sceKernelExitGame(void);

void sceKernelDcacheWritebackAll();
void sceKernelDcacheWritebackRange(const void *mem, int size);

void pspDebugScreenInit(void);
void pspDebugScreenPuts(const char *s);
void pspDebugScreenSetTextColor(ScePspRGBA8888 c);
