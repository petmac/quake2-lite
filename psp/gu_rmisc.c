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
==================
R_InitParticleTexture
==================
*/
byte	dottexture[8][8] =
{
	{0,0,0,0,0,0,0,0},
	{0,0,1,1,0,0,0,0},
	{0,1,1,1,1,0,0,0},
	{0,1,1,1,1,0,0,0},
	{0,0,1,1,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};

void R_InitParticleTexture (void)
{
	int		x,y;
	byte	data[8][8][4];

	//
	// particle texture
	//
	for (x=0 ; x<8 ; x++)
	{
		for (y=0 ; y<8 ; y++)
		{
			data[y][x][0] = 255;
			data[y][x][1] = 255;
			data[y][x][2] = 255;
			data[y][x][3] = dottexture[x][y]*255;
		}
	}
	r_particletexture = GL_LoadPic ("***particle***", (byte *)data, 8, 8, it_sprite, 32);

	//
	// also use this for bad textures, but without alpha
	//
	for (x=0 ; x<8 ; x++)
	{
		for (y=0 ; y<8 ; y++)
		{
			data[y][x][0] = dottexture[x&3][y&3]*255;
			data[y][x][1] = 0; // dottexture[x&3][y&3]*255;
			data[y][x][2] = 0; //dottexture[x&3][y&3]*255;
			data[y][x][3] = 255;
		}
	}
	r_notexture = GL_LoadPic ("***r_notexture***", (byte *)data, 8, 8, it_wall, 32);
}


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
	sceGuOffset(2048 - (GU_SCR_WIDTH / 2), 2048 - (GU_SCR_HEIGHT / 2));
	sceGuViewport(2048, 2048, GU_SCR_WIDTH, GU_SCR_HEIGHT);
	sceGuDepthRange(65535, 0);
	sceGuScissor(0, 0, GU_SCR_WIDTH, GU_SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);

	sceGuClearColor(GU_COLOR(1, 0, 0.5, 0.5));
	sceGuFrontFace(GU_CW); // TODO PeterM Is this right?
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_T8, 0, 0, GU_FALSE);
	sceGuTexLevelMode(GU_TEXTURE_AUTO, 0);
	sceGuTexMapMode(GU_TEXTURE_COORDS, 0, 0);

	//sceGuEnable(GU_ALPHA_TEST);
	sceGuAlphaFunc(GU_GREATER, 170, 0xff);

	sceGuDisable (GU_DEPTH_TEST);
	sceGuDisable (GU_CULL_FACE);
	sceGuDisable (GU_BLEND);
	sceGuDisable (GU_LIGHTING);

	sceGuColor(0xffffffff);

#ifndef PSP
	qglPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
#endif
	sceGuShadeModel (GU_FLAT);

	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);

	GL_SetTexturePalette( d_8to24table );
}
