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
// r_misc.c

#include "gu_local.h"

/* 
============================================================================== 
 
						SCREEN SHOTS 
 
============================================================================== 
*/ 

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/* 
================== 
GL_ScreenShot_f
================== 
*/  
void GL_ScreenShot_f (void) 
{
	byte		*buffer;
	char		picname[80]; 
	char		checkname[MAX_OSPATH];
	int			i, c, temp;
	FILE		*f;

	// create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot", ri.FS_Gamedir());
	Sys_Mkdir (checkname);

// 
// find a file name to save it to 
// 
	strcpy(picname,"quake00.tga");

	for (i=0 ; i<=99 ; i++) 
	{ 
		picname[5] = i/10 + '0'; 
		picname[6] = i%10 + '0'; 
		Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot/%s", ri.FS_Gamedir(), picname);
		f = fopen (checkname, "rb");
		if (!f)
			break;	// file doesn't exist
		fclose (f);
	} 
	if (i==100) 
	{
		ri.Con_Printf (PRINT_ALL, "SCR_ScreenShot_f: Couldn't create a file\n"); 
		return;
 	}


	buffer = Z_Malloc(GU_SCR_WIDTH*GU_SCR_HEIGHT*3 + 18);
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = GU_SCR_WIDTH&255;
	buffer[13] = GU_SCR_WIDTH>>8;
	buffer[14] = GU_SCR_HEIGHT&255;
	buffer[15] = GU_SCR_HEIGHT>>8;
	buffer[16] = 24;	// pixel size

#ifndef PSP
	qglReadPixels (0, 0, GU_SCR_WIDTH, GU_SCR_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, buffer+18 ); 
#endif

	// swap rgb to bgr
	c = 18+GU_SCR_WIDTH*GU_SCR_HEIGHT*3;
	for (i=18 ; i<c ; i+=3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	f = fopen (checkname, "wb");
	fwrite (buffer, 1, c, f);
	fclose (f);

	Z_Free (buffer);
	ri.Con_Printf (PRINT_ALL, "Wrote %s\n", picname);
}

/*
** GL_SetDefaultState
*/
void GL_SetDefaultState( void )
{
	ASSERT(GU_IsInDisplayList());

	// Viewport.
	sceGuOffset(2048 - (GU_SCR_WIDTH / 2), 2048 - (GU_SCR_HEIGHT / 2));
	sceGuViewport(2048, 2048, GU_SCR_WIDTH, GU_SCR_HEIGHT);
	sceGuScissor(0, 0, GU_SCR_WIDTH, GU_SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuEnable(GU_CLIP_PLANES);

	// Depth.
	sceGuDepthRange(0, 65535);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_LEQUAL);

	// Shading.
	sceGuColor(0xffffffff);
	sceGuShadeModel(GU_SMOOTH);

	// Clear.
	sceGuClearColor(GU_COLOR(0.0625f, 0.125f, 0.25f, 1));
	sceGuClearDepth(65535);

	// Culling.
	sceGuFrontFace(GU_CW);
	sceGuDisable(GU_CULL_FACE);

	// Texturing.
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexMode(GU_PSM_T8, 0, 0, GU_FALSE);
	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMapMode(GU_TEXTURE_COORDS, 0, 0);
	sceGuTexScale(1.0f, 1.0f);
	sceGuTexOffset(0.0f, 0.0f);

	// Blending.
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuDisable(GU_BLEND);

	// Lighting.
	sceGuDisable(GU_LIGHTING);

	// Upload the CLUT.
	sceGuClutMode(GU_PSM_8888, 0, 255, 0);
	GL_SetTexturePalette(d_8to24table);

	// Matrices.
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_TEXTURE);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
}
