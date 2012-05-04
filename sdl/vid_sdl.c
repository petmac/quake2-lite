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

extern cvar_t *vid_ref;
extern cvar_t *vid_fullscreen;

void M_PopMenu (void);

viddef_t	viddef;				// global video state

refexport_t	re;

refexport_t GetRefAPI (refimport_t rimp);

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


void	VID_Init (void)
{
	SDL_Rect **modes;
    refimport_t	ri;

	vid_ref = Cvar_Get ("vid_ref", "soft", CVAR_ARCHIVE);

	// Enumerate modes.
	modes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);
	if (modes == NULL)
	{
        Com_Error (ERR_FATAL, "SDL_ListModes returned no modes.");
		return;
	}
	else if (modes == (SDL_Rect **)-1)
	{
		vid_modes[0].width = 480;
		vid_modes[0].height = 272;
		vid_num_modes = 1;
	}
	else
	{
		while ((vid_num_modes < VID_MAX_MODES) && (modes[vid_num_modes] != NULL))
		{
			vid_modes[vid_num_modes].width = modes[vid_num_modes]->w;
			vid_modes[vid_num_modes].height = modes[vid_num_modes]->h;
			++vid_num_modes;
		}
	}

	qsort(&vid_modes[0], vid_num_modes, sizeof(vidmode_t), &CompareModes);

    viddef.width = vid_modes[0].width;
	viddef.height = vid_modes[0].height;

    ri.Cmd_AddCommand = Cmd_AddCommand;
    ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
    ri.Cmd_Argc = Cmd_Argc;
    ri.Cmd_Argv = Cmd_Argv;
    ri.Cmd_ExecuteText = Cbuf_ExecuteText;
    ri.Con_Printf = VID_Printf;
    ri.Sys_Error = VID_Error;
    ri.FS_LoadFile = FS_LoadFile;
    ri.FS_FreeFile = FS_FreeFile;
    ri.FS_Gamedir = FS_Gamedir;
	ri.Vid_NewWindow = VID_NewWindow;
	ri.Vid_MenuInit = VID_MenuInit;
    ri.Cvar_Get = Cvar_Get;
    ri.Cvar_Set = Cvar_Set;
    ri.Cvar_SetValue = Cvar_SetValue;
    ri.Vid_GetModeInfo = VID_GetModeInfo;

    re = GetRefAPI(ri);

    if (re.api_version != API_VERSION)
        Com_Error (ERR_FATAL, "Re has incompatible api_version");
    
        // call the init function
    if (re.Init (NULL, NULL) == -1)
		Com_Error (ERR_FATAL, "Couldn't start refresh");
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

		if ( !VID_LoadRefresh( "opengl32" ) )
		{
		        if ( strcmp (vid_ref->string, "soft") == 0 ) {
			        Com_Printf("Refresh failed\n");
				sw_mode = Cvar_Get( "sw_mode", "0", 0 );
				if (sw_mode->value != 0) {
				        Com_Printf("Trying mode 0\n");
					Cvar_SetValue("sw_mode", 0);
					if ( !VID_LoadRefresh( name ) )
						Com_Error (ERR_FATAL, "Couldn't fall back to software refresh!");
				} else
					Com_Error (ERR_FATAL, "Couldn't fall back to software refresh!");
			}

			Cvar_Set( "vid_ref", "soft" );

			/*
			** drop the console if we fail to load a refresh
			*/
			if ( cls.key_dest != key_console )
			{
				Con_ToggleConsole_f();
			}
		}
		cls.disable_screen = false;
	}
}
