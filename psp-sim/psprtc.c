#include "psprtc.h"

#include <SDL_timer.h>

void sceRtcGetCurrentTick(u64 *t)
{
	*t = SDL_GetPerformanceCounter();
}

u32 sceRtcGetTickResolution(void)
{
	return (u32)SDL_GetPerformanceFrequency();
}
