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

#include <SDL_rwops.h>

file_t *Sys_OpenFileRead(const char *path)
{
	SDL_RWops *f = SDL_RWFromFile(path, "rb");
	return (file_t *)f;
}

void Sys_CloseFile(file_t *file)
{
	SDL_RWclose((SDL_RWops *)file);
}

size_t Sys_ReadFile(void *buffer, size_t size, size_t count, file_t *file)
{
	SDL_RWops *f = (SDL_RWops *)file;
	return SDL_RWread(f, buffer, size, count);
}

long Sys_SeekFile(file_t *file, long offset, int whence)
{
	SDL_RWops *f = (SDL_RWops *)file;

	int wh = 0;
	switch (whence)
	{
	case SEEK_SET:
		wh = RW_SEEK_SET;
		break;
	case SEEK_CUR:
		wh = RW_SEEK_CUR;
		break;
	case SEEK_END:
		wh = RW_SEEK_END;
		break;
	}

	return SDL_RWseek(f, offset, wh);
}

long Sys_TellFile(file_t *file)
{
	SDL_RWops *f = (SDL_RWops *)file;
	return SDL_RWtell(f);
}
