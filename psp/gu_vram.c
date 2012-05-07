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

// gu_vram.c

#include "gu_local.h"

static unsigned int hunk_used;

void *GU_AllocateVRAM(int size)
{
	const unsigned int vram_reserved = (unsigned int)&((vram_t *)0)->hunk[0];
	const unsigned int vram_size = sceGeEdramGetSize();
	const unsigned int hunk_size = vram_size - vram_reserved;
	const unsigned int hunk_free = hunk_size - hunk_used;

	vram_t *vram;
	char *addr;

	// Round up the size to 16 byte boundary.
	size = (size + 15) & ~15;

	// Not enough space?
	if (size > hunk_free)
	{
		//Sys_Error("%s: Failed to allocate %d bytes.\n", __FUNCTION__, size);
		return NULL;
	}

	// Calculate the start address.
	vram = (vram_t *)sceGeEdramGetAddr();
	addr = &vram->hunk[hunk_used];

	// Allocate it.
	hunk_used += size;

	return addr;
}

void GU_FreeVRAM(void)
{
	hunk_used = 0;
}
