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
#include "../client/keys.h"

#include <SDL.h>
#include <Windows.h>

void *GetGameAPI (void *import);

int	curtime;
unsigned	sys_frame_time;

static byte	*membase;
static int	hunkmaxsize;
static int	cursize;
static qboolean go = true;

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

static int MapKey(SDLKey k)
{
	switch (k)
	{
	case SDLK_BACKSPACE: return K_BACKSPACE;

		//case SDLK_KP0: return K_;
	case SDLK_KP1: return K_KP_END;
	case SDLK_KP2: return K_KP_DOWNARROW;
	case SDLK_KP3: return K_KP_PGDN;
	case SDLK_KP4: return K_KP_LEFTARROW;
	case SDLK_KP5: return K_KP_5;
	case SDLK_KP6: return K_KP_RIGHTARROW;
	case SDLK_KP7: return K_KP_HOME;
	case SDLK_KP8: return K_KP_UPARROW;
	case SDLK_KP9: return K_KP_PGUP;
		//case SDLK_KP_PERIOD: return K_;
	case SDLK_KP_DIVIDE: return K_KP_SLASH;
		//case SDLK_KP_MULTIPLY: return K_;
	case SDLK_KP_MINUS: return K_KP_MINUS;
	case SDLK_KP_PLUS: return K_KP_PLUS;
	case SDLK_KP_ENTER: return K_KP_ENTER;
		//case SDLK_KP_EQUALS: return K_;

	case SDLK_UP: return K_UPARROW;
	case SDLK_DOWN: return K_DOWNARROW;
	case SDLK_RIGHT: return K_RIGHTARROW;
	case SDLK_LEFT: return K_LEFTARROW;
	case SDLK_INSERT: return K_INS;
	case SDLK_HOME: return K_HOME;
	case SDLK_END: return K_END;
	case SDLK_PAGEUP: return K_PGUP;
	case SDLK_PAGEDOWN: return K_PGDN;

	case SDLK_F1: return K_F1;
	case SDLK_F2: return K_F2;
	case SDLK_F3: return K_F3;
	case SDLK_F4: return K_F4;
	case SDLK_F5: return K_F5;
	case SDLK_F6: return K_F6;
	case SDLK_F7: return K_F7;
	case SDLK_F8: return K_F8;
	case SDLK_F9: return K_F9;
	case SDLK_F10: return K_F10;
	case SDLK_F11: return K_F11;
	case SDLK_F12: return K_F12;
		//case SDLK_F13: return K_;
		//case SDLK_F14: return K_;
		//case SDLK_F15: return K_;

		//case SDLK_NUMLOCK: return K_;
		//case SDLK_CAPSLOCK: return K_;
		//case SDLK_SCROLLOCK: return K_;
	case SDLK_RSHIFT: return K_SHIFT;
	case SDLK_LSHIFT: return K_SHIFT;
	case SDLK_RCTRL: return K_CTRL;
	case SDLK_LCTRL: return K_CTRL;
	case SDLK_RALT: return K_ALT;
	case SDLK_LALT: return K_ALT;
		//case SDLK_RMETA: return K_;
		//case SDLK_LMETA: return K_;
		//case SDLK_LSUPER: return K_;
		//case SDLK_RSUPER: return K_;
		//case SDLK_MODE: return K_;
		//case SDLK_COMPOSE: return K_;

		//case SDLK_HELP: return K_;
		//case SDLK_PRINT: return K_;
		//case SDLK_SYSREQ: return K_;
		//case SDLK_BREAK: return K_;
		//case SDLK_MENU: return K_;
		//case SDLK_POWER: return K_;
		//case SDLK_EURO: return K_;
		//case SDLK_UNDO: return K_;

	default:
		if (isascii(k))
		{
			return k;
		}
		else
		{
			return 0;
		}
	}
}

void Sys_SendKeyEvents (void)
{
	const int t = SDL_GetTicks();
	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			Key_Event(MapKey(e.key.keysym.sym), e.type == SDL_KEYDOWN, t);
			break;

		case SDL_QUIT:
			go = false;
			break;
		}
	}
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
	const char *const caption = "Quake 2 Lite";
	int oldtime;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) >= 0)
	{
		SDL_WM_SetCaption(caption, caption);

		Qcommon_Init (argc, argv);

		oldtime = SDL_GetTicks();

		while (go)
		{
			curtime = SDL_GetTicks();

			Qcommon_Frame (curtime - oldtime);

			oldtime = curtime;
		}

		SDL_Quit();
	}

	return 0;
}


