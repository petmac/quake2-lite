#include "pspaudiolib.h"

#include <SDL_audio.h>

#include <string.h>

static PSP_AUDIO_CALLBACK *psp_callback;
static void *psp_param;

static void SDLCALL sdl_callback(void *userdata, Uint8 *stream, int len)
{
	if (psp_callback != NULL)
	{
		(*psp_callback)(stream, len / 4, psp_param);
	}
	else
	{
		memset(stream, 0, len);
	}
}

int pspAudioInit(void)
{
	SDL_AudioSpec spec = { 0 };
	spec.freq = 44100;
	spec.format = AUDIO_S16SYS;
	spec.channels = 2;
	spec.samples = 512;
	spec.callback = &sdl_callback;

	if (SDL_OpenAudio(&spec, NULL) != 0)
	{
		return -1;
	}

	SDL_PauseAudio(0);

	return 0;
}

void pspAudioEnd(void)
{
	SDL_CloseAudio();
}

void pspAudioSetChannelCallback(int a, PSP_AUDIO_CALLBACK *callback, void *param)
{
	psp_callback = callback;
	psp_param = param;
}
