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

#include "../qcommon/qcommon.h"
#include <pspiofilemgr.h>

#define CACHE_COUNT 7
#define CACHE_SIZE 1024

typedef struct cache_s
{
	SceUID file;
	size_t file_pos;
	size_t begin;
	size_t end;
	uint8_t buffer[CACHE_SIZE];
} cache_t;

static cache_t caches[CACHE_COUNT];

static file_t *quake_file(SceUID file)
{
	if (file < 0)
	{
		return NULL;
	}
	else
	{
		return (file_t *)(file + 1);
	}
}

static SceUID native_file(file_t *file)
{
	if (file == NULL)
	{
		return -1;
	}
	else
	{
		return ((SceUID)file) - 1;
	}
}

static cache_t *cache_find(SceUID f)
{
	int i;

	for (i = 0; i < CACHE_COUNT; ++i)
	{
		cache_t *const cache = &caches[i];

		if (cache->file == f)
		{
			return cache;
		}
	}

	return NULL;
}

static size_t read_from_cache(cache_t *cache, uint8_t *dst, size_t bytes)
{
	const size_t cache_used = cache->end - cache->begin;
	if (cache_used > 0)
	{
		const size_t use_from_cache = (bytes < cache_used) ? bytes : cache_used;

		memcpy(dst, &cache->buffer[cache->begin], use_from_cache);
		
		cache->begin += use_from_cache;
		cache->file_pos += use_from_cache;

		return use_from_cache;
	}
	else
	{
		return 0;
	}
}

static void fill_cache(cache_t *cache)
{
	cache->begin = 0;
	cache->end = sceIoRead(cache->file, &cache->buffer[cache->begin], CACHE_SIZE);
}

file_t *Sys_OpenFileRead(const char *path)
{
	static qboolean cache_initialised = false;

	const SceUID f = sceIoOpen(path, PSP_O_RDONLY, 0777);

	if (!cache_initialised)
	{
		int i;

		cache_initialised = true;

		for (i = 0; i < CACHE_COUNT; ++i)
		{
			cache_t *const cache = &caches[i];

			cache->file = -1;
		}
	}

	return quake_file(f);
}

void Sys_CloseFile(file_t *file)
{
	const SceUID f = native_file(file);
	cache_t *const cache = cache_find(f);

	if (cache != NULL)
	{
		cache->file = -1;
	}

	sceIoClose(f);
}

size_t Sys_ReadFile(void *buffer, size_t size, size_t count, file_t *file)
{
	const SceUID f = native_file(file);
	cache_t *cache = cache_find(f);

	uint8_t *dst = buffer;
	size_t bytes_left = size * count;
	size_t bytes_read = 0;

	if (cache != NULL)
	{
		// Read as much as we need from the cache
		bytes_read = read_from_cache(cache, dst, bytes_left);
		bytes_left -= bytes_read;

		// Finished?
		if (bytes_left == 0)
		{
			return bytes_read / size;
		}

		dst += bytes_read;
	}
	else
	{
		cache = cache_find(-1);

		if (cache == NULL)
		{
			return sceIoRead(f, buffer, bytes_left) / size;
		}

		cache->file = f;
		cache->file_pos = 0;
		cache->begin = 0;
		cache->end = 0;
	}

	// The cache is now empty.
	// Can the rest fit in the cache?
	if (bytes_left <= CACHE_SIZE)
	{
		// Use the cache.
		fill_cache(cache);
		bytes_read += read_from_cache(cache, dst, bytes_left);
	}
	else
	{
		// Skip the cache.
		size_t bytes_read_from_file = sceIoRead(f, dst, bytes_left);
		bytes_read += bytes_read_from_file;
		cache->file_pos += bytes_read_from_file;
	}

	return bytes_read / size;
}

long Sys_SeekFile(file_t *file, long offset, int whence)
{
	const SceUID f = native_file(file);
	cache_t *const cache = cache_find(f);

	if (cache != NULL)
	{
		const size_t buffer_file_pos_begin = cache->file_pos - cache->begin;
		const size_t buffer_file_pos_end = buffer_file_pos_begin + cache->end;

		switch (whence)
		{
		case SEEK_SET:
			if ((offset >= buffer_file_pos_begin) && (offset <= buffer_file_pos_end))
			{
				cache->file_pos = offset;
				cache->begin = offset - buffer_file_pos_begin;
			}
			else
			{
				cache->file_pos = sceIoLseek32(f, offset, PSP_SEEK_SET);
				cache->begin = 0;
				cache->end = 0;
			}
			break;
		case SEEK_CUR:
			cache->file_pos = sceIoLseek32(f, cache->file_pos + offset, PSP_SEEK_SET);
			cache->begin = 0;
			cache->end = 0;
			break;
		case SEEK_END:
			cache->file_pos = sceIoLseek32(f, offset, PSP_SEEK_END);
			cache->begin = 0;
			cache->end = 0;
			break;
		}

		return cache->file_pos;
	}
	else
	{
		int wh = 0;
		switch (whence)
		{
		case SEEK_SET:
			wh = PSP_SEEK_SET;
			break;
		case SEEK_CUR:
			wh = PSP_SEEK_CUR;
			break;
		case SEEK_END:
			wh = PSP_SEEK_END;
			break;
		}

		return sceIoLseek32(f, offset, wh);
	}
}

long Sys_TellFile(file_t *file)
{
	const SceUID f = native_file(file);
	cache_t *const cache = cache_find(f);

	if (cache != NULL)
	{
		return cache->file_pos;
	}
	else
	{
		return sceIoLseek32(f, 0, PSP_SEEK_CUR);
	}
}
