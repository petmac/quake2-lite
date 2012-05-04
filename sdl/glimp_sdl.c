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

void		GLimp_BeginFrame( float camera_separation )
{
}

void		GLimp_EndFrame( void )
{
	SDL_GL_SwapBuffers();
}

qboolean	GLimp_Init( void *hinstance, void *hWnd )
{
	return true;
}

void		GLimp_Shutdown( void )
{
}

int     	GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width;
	int height;
	SDL_Surface *screen;

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	if (fullscreen)
	{
		screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL | SDL_FULLSCREEN);
	}
	else
	{
		screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL);
	}
	if (screen == NULL)
	{
		return rserr_invalid_mode;
	}

	*pwidth = screen->w;
	*pheight = screen->h;
	
	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	return rserr_ok;
}

void		GLimp_AppActivate( qboolean active )
{
}
