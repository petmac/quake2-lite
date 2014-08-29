#include "pspctrl.h"

#include <string.h>

void sceCtrlSetSamplingCycle(int cycle)
{
}

void sceCtrlSetSamplingMode(int mode)
{
}

void sceCtrlPeekBufferPositive(SceCtrlData *pad, int a)
{
	memset(pad, 0, sizeof(*pad));
}
