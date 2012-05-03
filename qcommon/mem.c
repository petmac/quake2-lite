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
#define REF_HUNK_CAPACITY (7 * 1024 * 1024)

static char game_mem[GAME_HUNK_CAPACITY];
static char level_mem[LEVEL_HUNK_CAPACITY];
static char ref_mem[REF_HUNK_CAPACITY];

hunk_t hunk_game;
hunk_t hunk_level;
hunk_t hunk_ref;

static void Hunk_Init (hunk_t *hunk, const char *name, int capacity, void *membase)
{
	const int base_as_int = (int)membase;

	if ((base_as_int % HUNK_ALIGNMENT) != 0)
	{
		Sys_Error ("Hunk_Init: Hunk \"%s\" membase is not aligned.", name);
	}

	hunk->cursize = 0;
	hunk->capacity = capacity;
	hunk->membase = membase;
	hunk->name = name;
	hunk->tidemark = 0;
}

void Mem_Init (void)
{
	Hunk_Init (&hunk_game, "Game", GAME_HUNK_CAPACITY, game_mem);
	Hunk_Init (&hunk_level, "Level", LEVEL_HUNK_CAPACITY, level_mem);
	Hunk_Init (&hunk_ref, "Refresh", REF_HUNK_CAPACITY, ref_mem);
}

void *Hunk_Alloc (hunk_t *hunk, int size)
{
	// round to size of double
	size = (size+HUNK_ALIGNMENT-1)&~(HUNK_ALIGNMENT-1);

	hunk->cursize += size;
	if (hunk->cursize > hunk->capacity)
		Sys_Error ("Mem_Alloc overflow");

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
