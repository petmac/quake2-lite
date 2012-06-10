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
// in_sdl.c

#include "../client/client.h"

#include "SDL_mouse.h"

cvar_t	*in_joystick;

void IN_Init (void)
{
	in_joystick				= Cvar_Get ("in_joystick",				"0",		CVAR_ARCHIVE);
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
}

void IN_Frame (void)
{
}

void IN_Move (usercmd_t *cmd)
{
	int mouse_x = 0;
	int mouse_y = 0;

	SDL_GetRelativeMouseState(&mouse_x, &mouse_y);

	mouse_x *= sensitivity->value;
	mouse_y *= sensitivity->value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || lookstrafe->value)
		cmd->sidemove += m_side->value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw->value * mouse_x;

	if ( freelook->value && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch->value * mouse_y;
	}
	else
	{
		cmd->forwardmove -= m_forward->value * mouse_y;
	}
}
