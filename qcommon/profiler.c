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
// profiler.c -- profiler handling

#include "qcommon.h"

#if ENABLE_PROFILER

typedef struct prof_stack_s
{
	const char *name;
	int start;
	int children;
} prof_stack_t;

typedef struct prof_block_s
{
	unsigned int hash;
	const char *name;
	int exclusive;
} prof_block_t;

#define MAX_PROF_STACK 1024
#define MAX_PROF_BLOCK 1024

static prof_stack_t stack[MAX_PROF_STACK];
static int stack_size;
static prof_block_t blocks[MAX_PROF_BLOCK];
static int block_count;

static unsigned int Prof_Hash(const char *name)
{
	unsigned int hash = 2166136261u;

	while (1)
	{
		const unsigned int c = *name;
		if (!c)
		{
			break;
		}

        hash = hash ^ c;
        hash = hash * 16777619u;

		++name;
	}

	return hash;
}

static int Prof_CompareName(const void *a, const void *b)
{
	const unsigned int *const ba = (const unsigned int *)a;
	const unsigned int *const bb = (const unsigned int *)b;

	return *bb - *ba;
}

static prof_block_t *Prof_Find(const char *name)
{
	unsigned int hash;
	prof_block_t *block;

	hash = Prof_Hash(name);

	// Find existing block.
	block = bsearch(&hash, blocks, block_count, sizeof(blocks[0]), &Prof_CompareName);
	if (block != NULL)
	{
		return block;
	}

	// Not found, add a new block.
	if (block_count < MAX_PROF_BLOCK)
	{
		block = &blocks[block_count];

		block->hash = hash;
		block->name = name;
		block->exclusive = 0;

		++block_count;

		qsort(blocks, block_count, sizeof(blocks[0]), &Prof_CompareName);

		return block;
	}

	return NULL;
}

static void Prof_Store(const char *name, int exclusive)
{
	prof_block_t *const block = Prof_Find(name);

	if (block == NULL)
	{
		return;
	}

	block->exclusive += exclusive;
}

static int Prof_CompareExclusive(const void *a, const void *b)
{
	const prof_block_t *const ba = (const prof_block_t *)a;
	const prof_block_t *const bb = (const prof_block_t *)b;

	return bb->exclusive - ba->exclusive;
}

void Prof_Init(void)
{
	stack_size = 0;
	block_count = 0;
}

void Prof_Begin(const char *name)
{
	if (stack_size < MAX_PROF_STACK)
	{
		prof_stack_t *const stack_frame = &stack[stack_size];

		stack_frame->name = name;
		stack_frame->children = 0;
		stack_frame->start = Sys_Milliseconds();
	}

	++stack_size;
}

void Prof_End(void)
{
	const int end = Sys_Milliseconds();

	--stack_size;

	if (stack_size < MAX_PROF_STACK)
	{
		const prof_stack_t *const stack_frame = &stack[stack_size];
		const int delta = end - stack_frame->start;
		
		Prof_Store(stack_frame->name, delta - stack_frame->children);

		// Update parent.
		if (stack_size > 0)
		{
			prof_stack_t *const parent = &stack[stack_size - 1];
			
			parent->children += delta;
		}
	}
}

void Prof_Print(void)
{
	int i;

	// Sanity checks.
	if (stack_size != 0)
	{
		Com_Error(ERR_FATAL, "Profiler stack not empty. (%d stack frames)", stack_size);
	}

	// Sort the blocks.
	qsort(blocks, block_count, sizeof(blocks[0]), &Prof_CompareExclusive);

	Com_Printf("\nPROFILER\n");

	for (i = 0; i < block_count; ++i)
	{
		const prof_block_t *const block = &blocks[i];

		if (block->exclusive > 0)
		{
			Com_Printf("\t%-32s:%10d\n", block->name, block->exclusive);
		}
	}

	block_count = 0;
}

#endif // ENABLE_PROFILER
