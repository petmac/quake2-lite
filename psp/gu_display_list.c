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

// gu_display_list.c

#include "gu_local.h"

static qboolean in_display_list;
static qboolean pending_finish;

void GU_StartDisplayList(void)
{
	GU_SyncDisplayList();

	sceGuStart(GU_DIRECT, vram->display_list);

	in_display_list = true;
}

void GU_FinishDisplayList(void)
{
	int size;
	
	if (!in_display_list)
	{
		Sys_Error("%s: Missing call to GU_StartDisplayList.", __FUNCTION__);
	}

	size = sceGuFinish();
	if (size > R_DISPLAY_LIST_SIZE)
	{
		Sys_Error("%s: GU Display list overflow. %d > %d.", __FUNCTION__, size, R_DISPLAY_LIST_SIZE);
	}

	pending_finish = true;
	in_display_list = false;
}

void GU_SyncDisplayList(void)
{
	if (pending_finish)
	{
		sceGuSync(GU_SYNC_FINISH, GU_SYNC_WHAT_DONE);
		pending_finish = false;
	}
}

qboolean GU_IsInDisplayList(void)
{
	return in_display_list;
}
