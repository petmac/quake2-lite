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

#include <malloc.h>

#define HUNK_STATS 1

struct hunk_s
{
	qboolean locked;
	int cursize;
	int capacity;
	char *membase;
	const char *name;
#if HUNK_STATS
	int tidemark;
#endif
};

#define HUNK_ALIGNMENT 16

#define GAME_HUNK_CAPACITY (1 * 1024 * 1024)
#define LEVEL_HUNK_CAPACITY (32 * 1024)
#define REF_HUNK_CAPACITY (10 * 1024 * 1024)
#define SND_HUNK_CAPACITY (5 * 1024 * 1024)

hunk_t hunk_game;
hunk_t hunk_level;
hunk_t hunk_ref;
hunk_t hunk_snd;

#if HUNK_STATS
#define MAX_HUNK_STATS 256

typedef struct hunk_stat_s
{
	// Key.
	hunk_t *hunk;
	const char *file;
	int line;
	const char *function;

	// Value.
	int allocs;
	int total_size;
} hunk_stat_t;

static hunk_stat_t hunk_stats[MAX_HUNK_STATS];

static hunk_stat_t *Hunk_FindStat(hunk_t *hunk, const char *file, int line, const char *function)
{
	int i;

	// Try to find a matching stat.
	for (i = 0; i < MAX_HUNK_STATS; ++i)
	{
		hunk_stat_t *const stat = &hunk_stats[i];

		// Is this one for the same code?
		if ((stat->hunk == hunk) &&
			(Q_stricmp(stat->file, file) == 0) &&
			(stat->line == line) &&
			(strcmp(stat->function, function) == 0))
		{
			// Done.
			return stat;
		}
	}

	// Find a free stat.
	for (i = 0; i < MAX_HUNK_STATS; ++i)
	{
		hunk_stat_t *const stat = &hunk_stats[i];

		// Is this one free?
		if (stat->hunk == NULL)
		{
			// Set it up.
			stat->hunk = hunk;
			stat->file = file;
			stat->line = line;
			stat->function = function;
			stat->allocs = 0;
			stat->total_size = 0;

			// Done.
			return stat;
		}
	}

	// No free stats.
	return NULL;
}

static void Hunk_AddStats(hunk_t *hunk, int size, const char *file, int line, const char *function)
{
	hunk_stat_t *const stat = Hunk_FindStat(hunk, file, line, function);
	if (stat == NULL)
	{
		return;
	}

	stat->allocs += 1;
	stat->total_size += size;
}

static int Hunk_CompareStats(const void *a, const void *b)
{
	const hunk_stat_t *const sa = a;
	const hunk_stat_t *const sb = b;

	return sb->total_size - sa->total_size;
}

static void Hunk_ReportStats(hunk_t *hunk)
{
	int i;

	// Sort stats by size.
	qsort(&hunk_stats[0], MAX_HUNK_STATS, sizeof(hunk_stats[0]), &Hunk_CompareStats);

	// Header.
	Com_Printf(
		"HUNK STATS: \"%s\".\n"
		"\tCurrent size = %d.\n"
		"\tTidemark = %d (%d%% of capacity).\n",
		hunk->name,
		hunk->cursize,
		hunk->tidemark,
		(hunk->tidemark * 100) / hunk->capacity);

	// Try to find a matching stat.
	for (i = 0; i < MAX_HUNK_STATS; ++i)
	{
		hunk_stat_t *const stat = &hunk_stats[i];

		// Is this one for the same hunk?
		if (stat->hunk == hunk)
		{
			// Print this stat.
			Com_Printf(
				"\t%s(%d) :\n\t\t%-26s allocated %4d K (%2d%% of current hunk size) in %d allocs (%d bytes average).\n",
				stat->file,
				stat->line,
				stat->function,
				stat->total_size / 1024,
				(stat->total_size * 100) / hunk->cursize,
				stat->allocs,
				stat->total_size / stat->allocs);

			// Clear stats after printing.
			memset(stat, 0, sizeof(*stat));
		}
	}
}
#endif

static void Hunk_Init(hunk_t *hunk, const char *name, int capacity)
{
	void *membase;
	int base_as_int;

	// Allocate memory.
#ifdef _MSC_VER
	membase = _aligned_malloc(capacity, HUNK_ALIGNMENT);
#else
	membase = memalign(HUNK_ALIGNMENT, capacity);
#endif
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

	// Zero hunk.
	memset(membase, 0, capacity);

	// Store info.
	hunk->locked = true;
	hunk->cursize = 0;
	hunk->capacity = capacity;
	hunk->membase = membase;
	hunk->name = name;
#if HUNK_STATS
	hunk->tidemark = 0;
#endif
}

void Mem_Init(void)
{
	Prof_Begin(__FUNCTION__);

	Hunk_Init (&hunk_game, "Game", GAME_HUNK_CAPACITY);
	Hunk_Init (&hunk_level, "Level", LEVEL_HUNK_CAPACITY);
	Hunk_Init (&hunk_ref, "Refresh", REF_HUNK_CAPACITY);
	Hunk_Init (&hunk_snd, "Sound", SND_HUNK_CAPACITY);

	// Some hunks are always unlocked.
	Hunk_Begin(&hunk_game);
	Hunk_Begin(&hunk_level);

	Prof_End();
}

void *Hunk_AllocEx(hunk_t *hunk, int size, const char *file, int line, const char *function)
{
	void *mem;

	// Allocate.
	mem = Hunk_AllocAllowFailEx(hunk, size, file, line, function);
	if (!mem)
	{
		Sys_Error("%s: Hunk \"%s\" overflowed allocating %d bytes in %s.", __FUNCTION__, hunk->name, size, function);
		return NULL;
	}

	return mem;
}

void *Hunk_AllocAllowFailEx(hunk_t *hunk, int size, const char *file, int line, const char *function)
{
	if (size > 0)
	{
#if HUNK_STATS
		Hunk_AddStats(hunk, size, file, line, function);
#endif

		// Hunk locked?
		if (hunk->locked)
		{
			Com_Printf("%s: Hunk \"%s\" locked when allocating %d bytes in %s.\n", __FUNCTION__, hunk->name, size, function);
		}
	}

	// Round up to alignment.
	size = (size + HUNK_ALIGNMENT - 1) & ~(HUNK_ALIGNMENT - 1);

	hunk->cursize += size;
	if (hunk->cursize > hunk->capacity)
	{
		return NULL;
	}

#if HUNK_STATS
	if (hunk->cursize > hunk->tidemark)
	{
		hunk->tidemark = hunk->cursize;
	}
#endif

	return (hunk->membase + hunk->cursize - size);
}

void Hunk_Begin(hunk_t *hunk)
{
	// Anything to zero?
	if (hunk->cursize > 0)
	{
		// Zero the hunk.
		memset(hunk->membase, 0, hunk->cursize);

		// Reset the size.
		hunk->cursize = 0;
	}

	// Unlock.
	hunk->locked = false;
}

void Hunk_End(hunk_t *hunk)
{
	// Lock.
	hunk->locked = true;

#if HUNK_STATS
	// Anything to report?
	if (hunk->cursize > 0)
	{
		Hunk_ReportStats(hunk);
	}
#endif
}

