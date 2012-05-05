/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// snddma_sdl.c
// all other sound mixing is portable

#include "../client/client.h"
#include "../client/snd_loc.h"

#include "SDL_audio.h"

#define BUFFER_SIZE (4 * 1024)

static byte buffer[BUFFER_SIZE];

static void SDLCALL Callback(void *userdata, Uint8 *stream, int len)
{
	const int bytes_per_mono_sample = (dma.samplebits / 8);
	const int src_beg = dma.samplepos * bytes_per_mono_sample;
	const int src_end = (src_beg + len) % BUFFER_SIZE;

	if (src_end >= src_beg)
	{
		memcpy(stream, &buffer[src_beg], len);
	}
	else
	{
		const int first_chunk_len = BUFFER_SIZE - src_beg;

		memcpy(&stream[0], &buffer[src_beg], first_chunk_len);
		memcpy(&stream[first_chunk_len], &buffer[0], src_end);
	}

	dma.samplepos = src_end / bytes_per_mono_sample;
}

qboolean SNDDMA_Init(void)
{
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	return false;

	// Set up the desired audio spec.
	memset(&desired, 0, sizeof(desired));
	desired.freq = (s_khz->value * 44100) / 44;
	desired.format = AUDIO_S16;
	desired.channels = 2;
	desired.samples = 512;
	desired.callback = &Callback;

	// Initialise the SDL audio system.
	memset(&obtained, 0, sizeof(obtained));
	if (SDL_OpenAudio(&desired, &obtained) != 0)
	{
		return false;
	}

	// Tell Quake what audio spec we got.
	dma.channels = obtained.channels;
	dma.submission_chunk = obtained.size;
	dma.samplepos = 0;
	if ((obtained.format == AUDIO_U8) || (obtained.format == AUDIO_S8))
	{
		dma.samples = BUFFER_SIZE;
		dma.samplebits = 8;
	}
	else
	{
		dma.samples = BUFFER_SIZE / 2;
		dma.samplebits = 16;
	}
	dma.speed = obtained.freq;
	dma.buffer = buffer;

	return true;
}

int	SNDDMA_GetDMAPos(void)
{
	return dma.samplepos;
}

void SNDDMA_Shutdown(void)
{
	SDL_CloseAudio();
}

void SNDDMA_BeginPainting (void)
{
	SDL_LockAudio();
}

void SNDDMA_Submit(void)
{
	SDL_UnlockAudio();
	SDL_PauseAudio(0);
}
