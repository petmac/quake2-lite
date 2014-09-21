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
	const gu_pixel_t *const front_buffer = (gu_back_buffer == vram->fb.fb1) ? vram->fb.fb2 : vram->fb.fb1;
	char		header[18];
	char		picname[80]; 
	char		checkname[MAX_OSPATH];
	int			i;
	FILE		*f;

// 
// find a file name to save it to 
// 
	strcpy(picname,"quake00.tga");

	for (i=0 ; i<=99 ; i++) 
	{ 
		picname[5] = i/10 + '0'; 
		picname[6] = i%10 + '0'; 
#ifdef _WIN32
		Com_sprintf(checkname, sizeof(checkname), "%s", picname);
#else
		Com_sprintf (checkname, sizeof(checkname), "ms0:/PICTURE/%s", picname);
#endif
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

	f = fopen(checkname, "wb");
	if (f == NULL)
	{
		ri.Con_Printf(PRINT_ALL, "SCR_ScreenShot_f: Couldn't open file %s\n", checkname);
		return;
	}

	memset(&header, 0, sizeof(header));
	header[2] = 2;		// uncompressed type
	header[12] = GU_SCR_WIDTH&255;
	header[13] = GU_SCR_WIDTH>>8;
	header[14] = GU_SCR_HEIGHT&255;
	header[15] = GU_SCR_HEIGHT>>8;
	header[16] = 24;	// pixel size

	fwrite(&header, 1, sizeof(header), f);

	for (i = 0; i < GU_SCR_HEIGHT; ++i)
	{
		const gu_pixel_t *const src_row = &front_buffer[(GU_SCR_HEIGHT - i - 1) * GU_SCR_BUF_WIDTH];
		uint8_t dst_row[GU_SCR_WIDTH][3];
		int x;

		for (x = 0; x < GU_SCR_WIDTH; ++x)
		{
			const gu_pixel_t src_pixel = src_row[x];
			const uint8_t r = ((src_pixel & 31) * 255) / 31;
			const uint8_t g = (((src_pixel >> 5) & 63) * 255) / 63;
			const uint8_t b = (((src_pixel >> 11) & 31) * 255) / 31;
			uint8_t *const dst_pixel = &dst_row[x][0];

			dst_pixel[0] = b;
			dst_pixel[1] = g;
			dst_pixel[2] = r;
		}

		fwrite(&dst_row[0][0], sizeof(dst_row), 1, f);
	}

	fclose (f);
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
