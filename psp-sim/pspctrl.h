#pragma once

#include "psptypes.h"

enum
{
	PSP_CTRL_MODE_ANALOG
};

enum PspCtrlButtons
{
	PSP_CTRL_SELECT = 1,
	PSP_CTRL_START = 2,
	PSP_CTRL_UP = 4,
	PSP_CTRL_RIGHT = 8,
	PSP_CTRL_DOWN = 16,
	PSP_CTRL_LEFT = 32,
	PSP_CTRL_LTRIGGER = 64,
	PSP_CTRL_RTRIGGER = 128,
	PSP_CTRL_TRIANGLE = 256,
	PSP_CTRL_CIRCLE = 512,
	PSP_CTRL_CROSS = 1024,
	PSP_CTRL_SQUARE = 2048
};

typedef struct SceCtrlData
{
	u32 Buttons;
} SceCtrlData;

void sceCtrlSetSamplingCycle(int cycle);
void sceCtrlSetSamplingMode(int mode);
void sceCtrlPeekBufferPositive(SceCtrlData *pad, int a);
