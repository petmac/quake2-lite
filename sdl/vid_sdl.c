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
// vid_null.c -- null video driver to aid porting efforts
// this assumes that one of the refs is statically linked to the executable

#include "../client/client.h"

#include "SDL_video.h"

extern SDL_Window *window;

// Console variables that we need to access from this module
extern cvar_t *vid_ref;
cvar_t		*vid_gamma;
cvar_t		*vid_fullscreen;

void M_PopMenu (void);

viddef_t	viddef;				// global video state

refexport_t	re;

refexport_t GetRefAPI (refimport_t rimp);

static qboolean	reflib_active;

/*
==========================================================================

DIRECT LINK GLUE

==========================================================================
*/

#define	MAXPRINTMSG	4096
void VID_Printf (int print_level, char *fmt, ...)
{
        va_list		argptr;
        char		msg[MAXPRINTMSG];

        va_start (argptr,fmt);
        vsprintf (msg,fmt,argptr);
        va_end (argptr);

        if (print_level == PRINT_ALL)
                Com_Printf ("%s", msg);
        else
                Com_DPrintf ("%s", msg);
}

void VID_Error (int err_level, char *fmt, ...)
{
        va_list		argptr;
        char		msg[MAXPRINTMSG];

        va_start (argptr,fmt);
        vsprintf (msg,fmt,argptr);
        va_end (argptr);

		Com_Error (err_level, "%s", msg);
}

void VID_NewWindow (int width, int height)
{
	viddef.width  = width;
	viddef.height = height;

	cl.force_refdef = true;		// can't use a paused refdef
}

/*
** VID_GetModeInfo
*/
typedef struct vidmode_s
{
    int width, height;
} vidmode_t;

#define VID_MAX_MODES 256

static vidmode_t vid_modes[VID_MAX_MODES];
static int vid_num_modes;

static int CompareModes(const void *a, const void *b)
{
	const vidmode_t *const ma = (const vidmode_t *)a;
	const vidmode_t *const mb = (const vidmode_t *)b;

	if (ma->width != mb->width)
	{
		return ma->width - mb->width;
	}
	else
	{
		return mb->height - mb->height;
	}
}

qboolean VID_GetModeInfo( int *width, int *height, int mode )
{
    if ( mode < 0 || mode >= vid_num_modes )
        return false;

    *width  = vid_modes[mode].width;
    *height = vid_modes[mode].height;

    return true;
}

/*
============
VID_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
void VID_Restart_f (void)
{
	vid_ref->modified = true;
}

/*
==============
VID_LoadRefresh
==============
*/
qboolean VID_LoadRefresh()
{
	refimport_t	ri;
	
	if ( reflib_active )
	{
		re.Shutdown();
	}

	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ExecuteText = Cbuf_ExecuteText;
	ri.Con_Printf = VID_Printf;
	ri.Sys_Error = VID_Error;
	ri.FS_LoadFile = FS_LoadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_FOpenFile = FS_FOpenFile;
	ri.FS_FCloseFile = FS_FCloseFile;
	ri.FS_Read = FS_Read;
	ri.FS_Gamedir = FS_Gamedir;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Vid_GetModeInfo = VID_GetModeInfo;
	ri.Vid_MenuInit = VID_MenuInit;
	ri.Vid_NewWindow = VID_NewWindow;

	re = GetRefAPI( ri );

	if (re.api_version != API_VERSION)
	{
		Com_Error (ERR_FATAL, "Refresh has incompatible api_version");
	}

	if ( re.Init( NULL, window ) == -1 )
	{
		re.Shutdown();
		return false;
	}

	reflib_active = true;

//======
//PGM
	vidref_val = VIDREF_GL;
//PGM
//======

	return true;
}

void	VID_Init (void)
{
	int i;

	// Enumerate modes.
	vid_num_modes = SDL_min(VID_MAX_MODES, SDL_GetNumDisplayModes(0));
	
	for (i = 0; i < vid_num_modes; ++i)
	{
		SDL_DisplayMode mode = { 0 };
		SDL_GetDisplayMode(0, i, &mode);

		vid_modes[i].width = mode.w;
		vid_modes[i].height = mode.h;
	}

	qsort(&vid_modes[0], vid_num_modes, sizeof(vidmode_t), &CompareModes);

	/* Create the video variables so we know how to start the graphics drivers */
	vid_ref = Cvar_Get ("vid_ref", "gl", CVAR_ARCHIVE);
	vid_fullscreen = Cvar_Get ("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma = Cvar_Get( "vid_gamma", "1", CVAR_ARCHIVE );

	/* Add some console commands that we want to handle */
	Cmd_AddCommand ("vid_restart", VID_Restart_f);
		
	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges();
}

void	VID_Shutdown (void)
{
    if (re.Shutdown)
	    re.Shutdown ();
}

void	VID_CheckChanges (void)
{
	if ( vid_ref->modified )
	{
		S_StopAllSounds();
	}

	while (vid_ref->modified)
	{
		/*
		** refresh has changed
		*/
		vid_ref->modified = false;
		vid_fullscreen->modified = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		if ( !VID_LoadRefresh() )
		{
			Com_Error (ERR_FATAL, "Failed to load refresh.");
		}
		cls.disable_screen = false;
	}
}
