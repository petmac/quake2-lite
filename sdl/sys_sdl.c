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
#include <SDL.h>
#include <Windows.h>

void *GetGameAPI (void *import);

int	curtime;
unsigned	sys_frame_time;

static byte	*membase;
static int	hunkmaxsize;
static int	cursize;

void Sys_mkdir (char *path)
{
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;

	printf ("Sys_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	exit (1);
}

void Sys_Quit (void)
{
	exit (0);
}

void	Sys_UnloadGame (void)
{
}

void	*Sys_GetGameAPI (void *parms)
{
	return GetGameAPI(parms);
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void	Sys_ConsoleOutput (char *string)
{
	puts(string);
	OutputDebugStringA(string);
}

void Sys_SendKeyEvents (void)
{
}

void Sys_AppActivate (void)
{
}

void Sys_CopyProtect (void)
{
}

char *Sys_GetClipboardData( void )
{
	return NULL;
}

void	*Hunk_Begin (int maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	cursize = 0;
	hunkmaxsize = maxsize;

	membase = malloc (maxsize);
	memset (membase, 0, maxsize);

	if (!membase)
		Sys_Error ("Hunk_Begin reserve failed");

	return (void *)membase;
}

void	*Hunk_Alloc (int size)
{
	// round to cacheline
	size = (size+31)&~31;

	cursize += size;
	if (cursize > hunkmaxsize)
		Sys_Error ("Hunk_Alloc overflow");

	return (void *)(membase+cursize-size);
}

void	Hunk_Free (void *buf)
{
	free (buf);
}

int		Hunk_End (void)
{
	return cursize;
}

int		Sys_Milliseconds (void)
{
	return SDL_GetTicks();
}

void	Sys_Mkdir (char *path)
{
}

char	*Sys_FindFirst (char *path, unsigned musthave, unsigned canthave)
{
	return NULL;
}

char	*Sys_FindNext (unsigned musthave, unsigned canthave)
{
	return NULL;
}

void	Sys_FindClose (void)
{
}

void	Sys_Init (void)
{
}


//=============================================================================

int main (int argc, char **argv)
{
	qboolean go = true;
	SDL_Event e;
	Uint32 t1;
	Uint32 t2;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) >= 0)
	{
		Qcommon_Init (argc, argv);

		t1 = SDL_GetTicks();

		while (go)
		{
			t2 = SDL_GetTicks();

			while (SDL_PollEvent(&e))
			{
				switch (e.type)
				{
				case SDL_QUIT:
					go = false;
					break;
				}
			}

			Qcommon_Frame ((t2 - t1) * 0.001f);

			t1 = t2;
		}

		SDL_Quit();
	}

	return 0;
}


