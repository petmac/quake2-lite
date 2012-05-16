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

// snddma_psp.c
// all other sound mixing is portable

#include "../client/client.h"
#include "../client/snd_loc.h"

#include <pspaudiolib.h>

typedef struct sample_s
{
	short left;
	short right;
} sample_t;

#define BUFFER_SIZE 4096

static sample_t input_buffer[BUFFER_SIZE];
static volatile int samples_read;

static inline void CopySamples(const sample_t *first, const sample_t *last, sample_t *destination)
{
	const sample_t *source;

	for (source = first; source != last; ++source)
	{
		const sample_t sample = *source;

		*destination++ = sample;
		*destination++ = sample;
	}
}

static void Callback(void *buf, unsigned int reqn, void *pdata)
{
	sample_t *destination;
	const sample_t *first_sample_to_read;
	int samples_to_read;
	int samples_before_end_of_input;

	destination = (sample_t *)buf;
	first_sample_to_read = &input_buffer[samples_read];
	samples_to_read = reqn / 2;

	// Going to wrap past the end of the input buffer?
	samples_before_end_of_input = BUFFER_SIZE - samples_read;
	if (samples_to_read > samples_before_end_of_input)
	{
		// Yes, so write the first chunk from the end of the input buffer.
		CopySamples(
			first_sample_to_read,
			first_sample_to_read + samples_before_end_of_input,
			&destination[0]);

		// Write the second chunk from the start of the input buffer.
		const unsigned int samplesToReadFromBeginning = samples_to_read - samples_before_end_of_input;
		CopySamples(
			&input_buffer[0],
			&input_buffer[samplesToReadFromBeginning],
			&destination[samples_before_end_of_input * 2]);
	}
	else
	{
		// No wrapping, just copy.
		CopySamples(
			first_sample_to_read,
			first_sample_to_read + samples_to_read,
			&destination[0]);
	}

	// Update the read offset.
	samples_read = (samples_read + samples_to_read) % BUFFER_SIZE;
}

qboolean SNDDMA_Init(void)
{
	if (pspAudioInit() < 0)
	{
		return false;
	}

	// Tell Quake what audio spec we have.
	dma.channels = 2;
	dma.submission_chunk = 1;
	dma.samplepos = 0;
	dma.samples = BUFFER_SIZE * 2;
	dma.samplebits = 16;
	dma.speed = 22050;
	dma.buffer = (byte *)input_buffer;

	// Set the channel callback.
	pspAudioSetChannelCallback(0, &Callback, NULL);

	return true;
}

int	SNDDMA_GetDMAPos(void)
{
	return samples_read * 2;
}

void SNDDMA_Shutdown(void)
{
	pspAudioEnd();
}

void SNDDMA_BeginPainting (void)
{
}

void SNDDMA_Submit(void)
{
}
