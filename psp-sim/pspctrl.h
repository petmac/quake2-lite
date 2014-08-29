#pragma once

#include "psptypes.h"

enum
{
	PSP_CTRL_MODE_ANALOG
};

enum
{
	PSP_CTRL_CROSS = 1
};

typedef struct SceCtrlData
{
	u32 Buttons;
} SceCtrlData;

void sceCtrlSetSamplingCycle(int cycle);
void sceCtrlSetSamplingMode(int mode);
void sceCtrlPeekBufferPositive(SceCtrlData *pad, int a);
