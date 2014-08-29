#include "pspkernel.h"

#include <Windows.h>

int sceKernelCreateCallback(const char *a, void *b, void *c)
{
	return 0;
}

void sceKernelRegisterExitCallback(int a)
{
}

int sceKernelCreateThread(const char *a, void *b, int c, int d, int e, int f)
{
	return 0;
}

void sceKernelStartThread(int id, int a, int b)
{

}

void sceKernelSleepThreadCB(void)
{
}

void sceKernelExitGame(void)
{
	__debugbreak();
	exit(0);
}

void sceKernelDcacheWritebackAll(void)
{
}

void sceKernelDcacheWritebackRange(const void *mem, int size)
{
}

void pspDebugScreenInit(void)
{
}

void pspDebugScreenPuts(const char *s)
{
	OutputDebugStringA(s);
}

void pspDebugScreenSetTextColor(ScePspRGBA8888 c)
{
}
