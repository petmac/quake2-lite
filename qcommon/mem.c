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
};

#define REF_HEAP_CAPACITY (16 * 1024 * 1024)

static char ref_mem[REF_HEAP_CAPACITY];

hunk_t hunk_ref;

void Mem_Init (void)
{
	hunk_ref.cursize = 0;
	hunk_ref.capacity = REF_HEAP_CAPACITY;
	hunk_ref.membase = ref_mem;
}

void *Hunk_Alloc (hunk_t *hunk, int size)
{
	// round to size of double
	size = (size+7)&~7;

	hunk->cursize += size;
	if (hunk->cursize > hunk->capacity)
		Sys_Error ("Mem_Alloc overflow");

	return (hunk->membase+hunk->cursize-size);
}

void Hunk_Begin (hunk_t *hunk)
{
	if (hunk->cursize > 0)
	{
		memset(hunk->membase, 0, hunk->cursize);

		hunk->cursize = 0;
	}
}

void Hunk_End (hunk_t *hunk)
{
}
