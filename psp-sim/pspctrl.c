#include "pspctrl.h"

#include <SDL_keyboard.h>

#include <string.h>

typedef struct binding_s
{
	SDL_Scancode sdl_key;
	u32 psp_button;
} binding_t;

static const binding_t bindings[] =
{
	{ SDL_SCANCODE_RSHIFT, PSP_CTRL_SELECT },
	{ SDL_SCANCODE_RETURN, PSP_CTRL_START },
	{ SDL_SCANCODE_UP, PSP_CTRL_UP },
	{ SDL_SCANCODE_RIGHT, PSP_CTRL_RIGHT },
	{ SDL_SCANCODE_DOWN, PSP_CTRL_DOWN },
	{ SDL_SCANCODE_LEFT, PSP_CTRL_LEFT },
	{ SDL_SCANCODE_Q, PSP_CTRL_LTRIGGER },
	{ SDL_SCANCODE_E, PSP_CTRL_RTRIGGER },
	{ SDL_SCANCODE_W, PSP_CTRL_TRIANGLE },
	{ SDL_SCANCODE_D, PSP_CTRL_CIRCLE },
	{ SDL_SCANCODE_S, PSP_CTRL_CROSS },
	{ SDL_SCANCODE_A, PSP_CTRL_SQUARE }
};

static const int binding_count = sizeof(bindings) / sizeof(bindings[0]);

void sceCtrlSetSamplingCycle(int cycle)
{
}

void sceCtrlSetSamplingMode(int mode)
{
}

void sceCtrlPeekBufferPositive(SceCtrlData *pad, int a)
{
	const Uint8 *const keys = SDL_GetKeyboardState(NULL);
	int i;

	memset(pad, 0, sizeof(*pad));

	for (i = 0; i < binding_count; ++i)
	{
		const binding_t *const binding = &bindings[i];

		if (keys[binding->sdl_key])
		{
			pad->Buttons |= binding->psp_button;
		}
	}
}
