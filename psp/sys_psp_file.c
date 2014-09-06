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

file_t *Sys_OpenFileRead(const char *path)
{
	SceUID f = sceIoOpen(path, PSP_O_RDONLY, 0777);
	return quake_file(f);
}

void Sys_CloseFile(file_t *file)
{
	SceUID f = native_file(file);
	sceIoClose(f);
}

size_t Sys_ReadFile(void *buffer, size_t size, size_t count, file_t *file)
{
	SceUID f = native_file(file);
	return sceIoRead(f, buffer, size * count) / size;
}

int Sys_SeekFile(file_t *file, long offset, int whence)
{
	SceUID f = native_file(file);

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

long Sys_TellFile(file_t *file)
{
	SceUID f = native_file(file);
	return sceIoLseek(f, 0, PSP_SEEK_CUR);
}
