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
// mem.c -- memory handling

#include "qcommon.h"

struct hunk_s
{
	int cursize;
	int capacity;
	char *membase;
	const char *name;
	int tidemark;
};

#define HUNK_ALIGNMENT 8

#define GAME_HUNK_CAPACITY (1 * 1024 * 1024)
#define LEVEL_HUNK_CAPACITY (32 * 1024)
#define REF_HUNK_CAPACITY (8 * 1024 * 1024)
#ifdef PSP
#define SND_HUNK_CAPACITY (1 * 1024 * 1024)
#else
#define SND_HUNK_CAPACITY (9 * 1024 * 1024)
#endif

hunk_t hunk_game;
hunk_t hunk_level;
hunk_t hunk_ref;
hunk_t hunk_snd;

static void Hunk_Init (hunk_t *hunk, const char *name, int capacity)
{
	void *membase;
	int base_as_int;

	// Allocate memory.
	membase = calloc(capacity, 1);
	if (membase == NULL)
	{
		Sys_Error ("Hunk_Init: Failed to allocate hunk \"%s\".", name);
		return;
	}

	// Check alignment.
	base_as_int = (int)membase;
	if ((base_as_int % HUNK_ALIGNMENT) != 0)
	{
		Sys_Error ("Hunk_Init: Hunk \"%s\" membase is not aligned.", name);
		return;
	}

	// Store info.
	hunk->cursize = 0;
	hunk->capacity = capacity;
	hunk->membase = membase;
	hunk->name = name;
	hunk->tidemark = 0;
}

void Mem_Init (void)
{
	Hunk_Init (&hunk_game, "Game", GAME_HUNK_CAPACITY);
	Hunk_Init (&hunk_level, "Level", LEVEL_HUNK_CAPACITY);
	Hunk_Init (&hunk_ref, "Refresh", REF_HUNK_CAPACITY);
	Hunk_Init (&hunk_snd, "Sound", SND_HUNK_CAPACITY);
}

void *Hunk_Alloc (hunk_t *hunk, int size)
{
	void *const mem = Hunk_AllocAllowFail(hunk, size);
	if (!mem)
	{
		Sys_Error("%s: Hunk \"%s\" overflowed allocating %d bytes.", __FUNCTION__, hunk->name, size);
		return NULL;
	}

	return mem;
}

void *Hunk_AllocAllowFail (hunk_t *hunk, int size)
{
	// round to size of double
	size = (size+HUNK_ALIGNMENT-1)&~(HUNK_ALIGNMENT-1);

	hunk->cursize += size;
	if (hunk->cursize > hunk->capacity)
		return NULL;

	if (hunk->cursize > hunk->tidemark)
	{
		hunk->tidemark = hunk->cursize;
	}

	return (hunk->membase+hunk->cursize-size);
}

void Hunk_Free (hunk_t *hunk)
{
	Com_Printf(
		"Hunk_Free: Freeing hunk \"%s\". Current size = %d, tidemark = %d (%d%% of capacity).\n",
		hunk->name,
		hunk->cursize,
		hunk->tidemark,
		(hunk->tidemark * 100) / hunk->capacity);

	if (hunk->cursize > 0)
	{
		memset(hunk->membase, 0, hunk->cursize);

		hunk->cursize = 0;
	}
}
