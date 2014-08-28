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
#include "../ref_gl/gl_local.h"

#include <SDL_video.h>

extern SDL_Window *window;

static SDL_GLContext context;

void		GLimp_BeginFrame( float camera_separation )
{
}

void		GLimp_EndFrame( void )
{
	Prof_Begin(__FUNCTION__);

	SDL_GL_SwapWindow(window);

	Prof_End();
}

qboolean	GLimp_Init( void *hinstance, void *hWnd )
{
	context = SDL_GL_CreateContext(window);

	return context != NULL;
}

void		GLimp_Shutdown( void )
{
	SDL_GL_DeleteContext(context);
}

int     	GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width;
	int height;

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	if (fullscreen)
	{
		SDL_DisplayMode mode = { 0 };
		mode.w = width;
		mode.h = height;

		SDL_SetWindowDisplayMode(window, &mode);
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	}
	else
	{
		SDL_SetWindowFullscreen(window, 0);
		SDL_SetWindowSize(window, width, height);
	}

	*pwidth = width;
	*pheight = height;
	
	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	return rserr_ok;
}

void		GLimp_AppActivate( qboolean active )
{
}
