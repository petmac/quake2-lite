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
// in_psp.c

#include "../client/client.h"

#include <pspctrl.h>

extern unsigned sys_frame_time;

cvar_t	*in_joystick;

typedef struct button_mapping_s
{
	enum PspCtrlButtons button;
	qkey_t key;
} button_mapping_t;

static SceCtrlData previous_pad;

static button_mapping_t button_mappings[] =
{
	{ PSP_CTRL_SELECT, K_SELECT },
	{ PSP_CTRL_START, K_START },
	{ PSP_CTRL_UP, K_UPARROW },
	{ PSP_CTRL_RIGHT, K_RIGHTARROW },
	{ PSP_CTRL_DOWN, K_DOWNARROW },
	{ PSP_CTRL_LEFT, K_LEFTARROW },
	{ PSP_CTRL_LTRIGGER, K_LTRIGGER },
	{ PSP_CTRL_RTRIGGER, K_RTRIGGER },
	{ PSP_CTRL_TRIANGLE, K_TRIANGLE },
	{ PSP_CTRL_CIRCLE, K_CIRCLE },
	{ PSP_CTRL_CROSS, K_CROSS },
	{ PSP_CTRL_SQUARE, K_SQUARE }
};

void IN_Init(void)
{
	in_joystick = Cvar_Get("in_joystick", "1", 0);

	Key_SetBinding(K_TRIANGLE, "screenshot");
}

void IN_Shutdown(void)
{
}

void IN_Commands(void)
{
	const int button_mapping_count = sizeof(button_mappings) / sizeof(button_mappings[0]);
	const int time = sys_frame_time;

	SceCtrlData pad;
	SceCtrlData pad_changed;
	int i;

	sceCtrlPeekBufferPositive(&pad, 1);
	pad_changed.Buttons = previous_pad.Buttons ^ pad.Buttons;

	for (i = 0; i < button_mapping_count; ++i)
	{
		const button_mapping_t *const mapping = &button_mappings[i];
		if (pad_changed.Buttons & mapping->button)
		{
			Key_Event(mapping->key, (pad.Buttons & mapping->button) != 0, time);
		}
	}

	previous_pad = pad;
}

void IN_Frame(void)
{
}

void IN_Move(usercmd_t *cmd)
{
}
