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

// draw.c

#include "gu_local.h"

static image_t *draw_chars;

/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal (void)
{
	// load console characters (don't bilerp characters)
	draw_chars = GL_FindImage ("pics/conchars.pcx", it_pic);
}



/*
================
Draw_Char

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Char (int x, int y, int num)
{
	int u;
	int v;
	gu_2d_vertex_t *vertices;

	LOG_FUNCTION_ENTRY;
	ASSERT(GU_IsInDisplayList());

	// Skip spaces.
	if ((num & 127) == 32)
	{
		LOG_FUNCTION_EXIT;
		return;
	}

	// Skip characters off the top of the screen.
	if (y <= -8)
	{
		LOG_FUNCTION_EXIT;
		return;
	}

	// Keep in the texture page.
	num &= 255;
	
	// Calculate texture coordinates.
	u = (num & 15) << 3;
	v = (num >> 4) << 3;

	// Set the texture image.
	GL_Bind(draw_chars);

	// Fill the vertices.
	vertices = sceGuGetMemory(sizeof(gu_2d_vertex_t) * 2);
	vertices[0].u = u;
	vertices[0].v = v;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[1].u = u + 8;
	vertices[1].v = v + 8;
	vertices[1].x = x + 8;
	vertices[1].y = y + 8;
	vertices[1].z = 0;

	// Draw the character.
	sceGumDrawArray(GU_SPRITES, GU_2D_VERTEX_TYPE, 2, NULL, vertices);

	LOG_FUNCTION_EXIT;
}

/*
=============
Draw_FindPic
=============
*/
image_t	*Draw_FindPic (char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	LOG_FUNCTION_ENTRY;

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = GL_FindImage (fullname, it_pic);
	}
	else
		gl = GL_FindImage (name+1, it_pic);

	LOG_FUNCTION_EXIT;

	return gl;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize (int *w, int *h, char *pic)
{
	image_t *gl;

	LOG_FUNCTION_ENTRY;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		*w = *h = -1;

		LOG_FUNCTION_EXIT;

		return;
	}
	*w = gl->width;
	*h = gl->height;

	LOG_FUNCTION_EXIT;
}

/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic (int x, int y, int w, int h, char *pic)
{
	image_t *gl;
	gu_2d_vertex_t *vertices;

	LOG_FUNCTION_ENTRY;

	ASSERT(GU_IsInDisplayList());

	gl = Draw_FindPic (pic);
	if (!gl || !gl->data)
	{
		sceGuDisable(GU_TEXTURE_2D);
	}

	// Set the texture image.
	GL_Bind(gl);

	// Fill the vertices.
	vertices = sceGuGetMemory(sizeof(gu_2d_vertex_t) * 2);
	vertices[0].u = 0;
	vertices[0].v = 0;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[1].u = gl->width;
	vertices[1].v = gl->height;
	vertices[1].x = x + w;
	vertices[1].y = y + h;
	vertices[1].z = 0;

	// Draw the pic.
	sceGumDrawArray(GU_SPRITES, GU_2D_VERTEX_TYPE, 2, NULL, vertices);

	if (!gl || !gl->data)
	{
		sceGuEnable(GU_TEXTURE_2D);
	}

	LOG_FUNCTION_EXIT;
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, char *pic)
{
	image_t *gl;
	gu_2d_vertex_t *vertices;

	LOG_FUNCTION_ENTRY;

	ASSERT(GU_IsInDisplayList());

	gl = Draw_FindPic (pic);
	if (!gl || !gl->data)
	{
		sceGuDisable(GU_TEXTURE_2D);
	}

	// Set the texture image.
	GL_Bind(gl);

	// Fill the vertices.
	vertices = sceGuGetMemory(sizeof(gu_2d_vertex_t) * 2);
	vertices[0].u = 0;
	vertices[0].v = 0;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[1].u = gl->width;
	vertices[1].v = gl->height;
	vertices[1].x = x + gl->width;
	vertices[1].y = y + gl->height;
	vertices[1].z = 0;

	// Draw the pic.
	sceGumDrawArray(GU_SPRITES, GU_2D_VERTEX_TYPE, 2, NULL, vertices);

	if (!gl || !gl->data)
	{
		sceGuEnable(GU_TEXTURE_2D);
	}

	LOG_FUNCTION_EXIT;
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h, char *pic)
{
	// TODO Do the tiling.
	Draw_StretchPic(x, y, w, h, pic);
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	gu_2d_vertex_t *vertices;

	LOG_FUNCTION_ENTRY;
	ASSERT(GU_IsInDisplayList());

	if ( (unsigned)c > 255)
		ri.Sys_Error (ERR_FATAL, "Draw_Fill: bad color");

	// Set state.
	sceGuDisable(GU_TEXTURE_2D);
	sceGuColor(d_8to24table[c]);

	// Fill the vertices.
	vertices = sceGuGetMemory(sizeof(gu_2d_vertex_t) * 2);
	vertices[0].u = 0;
	vertices[0].v = 0;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[1].u = 0;
	vertices[1].v = 0;
	vertices[1].x = x + w;
	vertices[1].y = y + h;
	vertices[1].z = 0;

	// Draw the fill.
	sceGumDrawArray(GU_SPRITES, GU_2D_VERTEX_TYPE, 2, NULL, vertices);

	// Restore state.
	sceGuColor(0xffffffff);
	sceGuEnable(GU_TEXTURE_2D);

	LOG_FUNCTION_EXIT;
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	LOG_FUNCTION_ENTRY;

#ifndef PSP
	qglEnable (GU_BLEND);
	qglDisable (GL_TEXTURE_2D);
	qglColor4f (0, 0, 0, 0.8);
	qglBegin (GL_QUADS);

	qglVertex2f (0,0);
	qglVertex2f (GU_SCR_WIDTH, 0);
	qglVertex2f (GU_SCR_WIDTH, GU_SCR_HEIGHT);
	qglVertex2f (0, GU_SCR_HEIGHT);

	qglEnd ();
	qglColor4f (1,1,1,1);
	qglEnable (GL_TEXTURE_2D);
	qglDisable (GU_BLEND);
#endif

	LOG_FUNCTION_EXIT;
}


//====================================================================


/*
=============
Draw_StretchRaw
=============
*/
extern gu_pixel_t r_rawpalette[256];

void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
	const int src_row_step = (rows << 8) / h;
	const int src_col_step = (cols << 8) / w;
	int dst_row;
	int src_row;

	LOG_FUNCTION_ENTRY;

	// Finish up any previous drawing commands.
	GU_FinishDisplayList();
	GU_SyncDisplayList();

	// For each row...
	src_row = 0;
	dst_row = 0;
	while (dst_row < h)
	{
		const byte *const src = &data[(src_row >> 8) * cols];
		gu_pixel_t *dst = &gu_back_buffer[((dst_row + y) * GU_SCR_BUF_WIDTH) + x];
		const gu_pixel_t *const dst_end = dst + w;

		int src_col = 0;

		while (dst < dst_end)
		{
			const int index = src[src_col >> 8];
			const gu_pixel_t pixel = r_rawpalette[index];

			*dst = pixel;

			src_col += src_col_step;
			++dst;
		}

		src_row += src_row_step;
		++dst_row;
	}

	// Write back the cache.
	sceKernelDcacheWritebackAll();

	// Start a new display list.
	GU_StartDisplayList();

	LOG_FUNCTION_EXIT;
}
