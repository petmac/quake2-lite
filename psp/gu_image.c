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

#include "gu_local.h"

#include <pspkernel.h>

image_t		gltextures[MAX_GLTEXTURES];
int			numgltextures;

static byte			 intensitytable[256];
static unsigned char gammatable[256];

cvar_t		*intensity;

ScePspRGBA8888 __attribute__((aligned(16))) d_8to24table[256];

void GL_SetTexturePalette(const ScePspRGBA8888 *palette)
{
	Prof_Begin(__FUNCTION__);

	ASSERT(GU_IsInDisplayList());

	sceKernelDcacheWritebackAll();
	sceGuClutLoad(32, palette);

	Prof_End();
}

void GL_TexEnv( int mode )
{
	static int lastmode = -1;

	if ( mode != lastmode )
	{
		//sceGuTexFunc(mode, GU_TCC_RGBA);
		lastmode = mode;
	}
}

void GL_Bind (const image_t *texnum)
{
	Prof_Begin(__FUNCTION__);

	ASSERT(GU_IsInDisplayList());

#ifndef PSP
	if ( gl_state.currenttexture == texnum)
		return;
#endif
	gl_state.currenttexture = texnum;

	if (texnum && texnum->data)
	{
		sceGuTexImage(0, texnum->buffer_width, texnum->buffer_height, texnum->buffer_width, texnum->data);
	}

	Prof_End();
}

/*
===============
GL_ImageList_f
===============
*/
void	GL_ImageList_f (void)
{
	int		i;
	image_t	*image;
	const char *palstrings[2] =
	{
		"RGB",
		"PAL"
	};

	ri.Con_Printf (PRINT_ALL, "------------------\n");

	for (i=0, image=gltextures ; i<numgltextures ; i++, image++)
	{
		if (image->data == NULL)
			continue;
		switch (image->type)
		{
		case it_skin:
			ri.Con_Printf (PRINT_ALL, "M");
			break;
		case it_sprite:
			ri.Con_Printf (PRINT_ALL, "S");
			break;
		case it_wall:
			ri.Con_Printf (PRINT_ALL, "W");
			break;
		case it_pic:
			ri.Con_Printf (PRINT_ALL, "P");
			break;
		default:
			ri.Con_Printf (PRINT_ALL, " ");
			break;
		}

		ri.Con_Printf (PRINT_ALL,  " %3i %3i %s\n",
			image->width, image->height, image->name);
	}
}

/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/
static void LoadPCX (char *filename, byte **pic, byte **palette, short *width, short *height)
{
	byte	*raw;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*out, *pix;

	Prof_Begin(__FUNCTION__);

	*pic = NULL;
	if (palette)
	{
		*palette = NULL;
	}

	//
	// load the file
	//
	len = ri.FS_LoadFile (filename, (void **)&raw);
	if (!raw)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		Prof_End();
		return;
	}

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;

    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ri.Con_Printf (PRINT_ALL, "Bad pcx file %s\n", filename);
		Prof_End();
		return;
	}

	out = Z_Malloc ( (pcx->ymax+1) * (pcx->xmax+1) );

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = Z_Malloc(768);
		memcpy (*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax+1;
	if (height)
		*height = pcx->ymax+1;

	for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1)
	{
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = *raw++;

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if ( raw - (byte *)pcx > len)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "PCX file %s was malformed", filename);
		Z_Free (*pic);
		*pic = NULL;
	}

	ri.FS_FreeFile (pcx);

	Prof_End();
}

/*
=========================================================

TARGA LOADING

=========================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
=============
LoadTGA
=============
*/
void LoadTGA (char *name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;
	byte tmp[2];

	Prof_Begin(__FUNCTION__);

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile (name, (void **)&buffer);
	if (!buffer)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "Bad tga file %s\n", name);
		Prof_End();
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10) 
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only type 2 and 10 targa RGB images supported\n");

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = Z_Malloc (numPixels*4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
					case 24:
							
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
					case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	ri.FS_FreeFile (buffer);

	Prof_End();
}


/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct
{
	short		x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void R_FloodFillSkin( byte *skin, int skinwidth, int skinheight )
{
#ifndef PSP
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t			fifo[FLOODFILL_FIFO_SIZE];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP( -1, -1, 0 );
		if (x < skinwidth - 1)	FLOODFILL_STEP( 1, 1, 0 );
		if (y > 0)				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if (y < skinheight - 1)	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	}
#endif
}

//=======================================================


/*
================
GL_ResampleTexture
================
*/
void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[1024], p2[1024];
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for (i=0 ; i<outwidth ; i++)
	{
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for (i=0 ; i<outwidth ; i++)
	{
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out+j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			((byte *)(out+j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			((byte *)(out+j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			((byte *)(out+j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}
}

/*
================
GL_LightScaleTexture

Scale up the pixel values in a texture to increase the
lighting range
================
*/
void GL_LightScaleTexture (unsigned *in, int inwidth, int inheight, qboolean only_gamma )
{
	if ( only_gamma )
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[p[0]];
			p[1] = gammatable[p[1]];
			p[2] = gammatable[p[2]];
		}
	}
	else
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[intensitytable[p[0]]];
			p[1] = gammatable[intensitytable[p[1]]];
			p[2] = gammatable[intensitytable[p[2]]];
		}
	}
}

static u32 next_power_of_two(u32 x)
{
	x--;
	x |= x >> 1;  // handle  2 bit numbers
	x |= x >> 2;  // handle  4 bit numbers
	x |= x >> 4;  // handle  8 bit numbers
	x |= x >> 8;  // handle 16 bit numbers
	x |= x >> 16; // handle 32 bit numbers
	x++;

	return x;
}

static void GL_UploadPic (image_t *image, byte *pic)
{
	Prof_Begin(__FUNCTION__);

	// Calculate buffer size.
	if (image->width <= 16)
	{
		image->buffer_width = 16;
	}
	else
	{
		image->buffer_width = next_power_of_two(image->width);
	}
	image->buffer_height = next_power_of_two(image->height);

	image->data = Hunk_AllocAllowFail(&hunk_ref, image->buffer_width * image->height);
	if (image->data != NULL)
	{
		int i;

		for (i = 0; i < image->height; ++i)
		{
			char *const dst = ((char *)(image->data)) + (i * image->buffer_width);
			const char *const src = pic + (i * image->width);
			memcpy(dst, src, image->width);
		}

		sceKernelDcacheWritebackRange(image->data, image->buffer_width * image->height);
	}

#if 0
	ri.Con_Printf(
		PRINT_DEVELOPER,
		"GL_Upload: %s ... %d x %d (%d x %d), %d bytes.\n",
		name,
		width,
		height,
		buffer_width,
		buffer_height,
		(buffer_width * height));
#endif

	Prof_End();
}

static void GL_UploadSkin (image_t *image, byte *pic)
{
	int bufw;
	int bufh;

	Prof_Begin(__FUNCTION__);

	// Calculate buffer size.
	if (image->width <= 16)
	{
		bufw = 16;
	}
	else
	{
		bufw = 1 << Q_log2(image->width);
	}
	bufh = 1 << Q_log2(image->height);

	image->buffer_width = bufw;
	image->buffer_height = bufh;
	image->data = Hunk_AllocAllowFail(&hunk_ref, bufw * bufh);
	if (image->data != NULL)
	{
		const int x_scale = (image->width << 8) / bufw;
		const int y_scale = (image->height << 8) / bufh;
		int dst_y;

		for (dst_y = 0; dst_y < bufh; ++dst_y)
		{
			const int src_y = (dst_y * y_scale) >> 8;
			char *const dst = ((char *)(image->data)) + (dst_y * bufw);
			const char *const src = pic + (src_y * image->width);
			int dst_x;

			for (dst_x = 0; dst_x < bufw; ++dst_x)
			{
				const int src_x = (dst_x * x_scale) >> 8;

				dst[dst_x] = src[src_x];
			}
		}

		sceKernelDcacheWritebackRange(image->data, bufw * bufh);
	}

#if 0
	ri.Con_Printf(
		PRINT_DEVELOPER,
		"GL_Upload: %s ... %d x %d (%d x %d), %d bytes.\n",
		name,
		width,
		height,
		buffer_width,
		buffer_height,
		(buffer_width * height));
#endif

	Prof_End();
}

static void GL_Upload (image_t *image, byte *pic)
{
	if (image->type == it_pic)
	{
		GL_UploadPic(image, pic);
	}
	else
	{
		GL_UploadSkin(image, pic);
	}
}

/*
================
GL_LoadWal
================
*/
static image_t *GL_LoadWal (image_t *image)
{
	miptex_t	*mt;
	int			ofs;

	Prof_Begin(__FUNCTION__);

	ri.FS_LoadFile (image->name, (void **)&mt);
	if (!mt)
	{
		ri.Con_Printf (PRINT_ALL, "GL_FindImage: can't load %s\n", image->name);
		return NULL;
	}

	image->width = LittleLong (mt->width);
	image->height = LittleLong (mt->height);
	ofs = LittleLong (mt->offsets[0]);

	GL_Upload(image, (byte *)mt + ofs);

	ri.FS_FreeFile ((void *)mt);

	Prof_End();

	return image;
}

/*
===============
GL_FindImage

Finds or loads the given image
===============
*/
image_t	*GL_FindImage (char *name, imagetype_t type)
{
	image_t	*image;
	int		i, len;
	const char *ext;

	if (!name)
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GL_FindImage: NULL name");
	len = strlen(name);
	if (len<5)
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GL_FindImage: bad name: %s", name);

	//ri.Con_Printf(PRINT_DEVELOPER, "Looking for %s.\n", name);

	// look for it
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!strcmp(name, image->name))
		{
			//ri.Con_Printf(PRINT_DEVELOPER, "\tFound in slot %d\n", i);
			return image;
		}
	}

	//ri.Con_Printf(PRINT_DEVELOPER, "\tDidn't find %s in %d images.\n", name, numgltextures);

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->name[0])
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			ri.Sys_Error (ERR_DROP, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	//ri.Con_Printf(PRINT_DEVELOPER, "\tPut in slot %d.\n", i);

	memset(image, 0, sizeof(*image));
	strcpy(image->name, name);
	image->type = type;

	ext = name + len - 4;

	//
	// load the pic from disk
	//
	if (!strcmp(ext, ".pcx"))
	{
		byte *pic = NULL;

		LoadPCX (image->name, &pic, NULL, &image->width, &image->height);
		if (pic)
		{
			GL_Upload(image, pic);
			Z_Free(pic);
		}
	}
	else if (!strcmp(ext, ".wal"))
	{
		GL_LoadWal (image);
	}
	else if (!strcmp(ext, ".tga"))
	{
#ifndef PSP
		LoadTGA (name, &pic, &width, &height);
		if (!pic)
			return NULL; // ri.Sys_Error (ERR_DROP, "GL_FindImage: can't load %s", name);
		image = GL_LoadPic (name, pic, width, height, type, 32);
#endif
	}

	return image;
}



/*
===============
R_RegisterSkin
===============
*/
struct image_s *R_RegisterSkin (char *name)
{
	struct image_s *s;

	Prof_Begin(__FUNCTION__);

	s = GL_FindImage (name, it_skin);

	Prof_End();

	return s;
}


/*
===============
Draw_GetPalette
===============
*/
int Draw_GetPalette (void)
{
	int		i;
	int		r, g, b;
	unsigned	v;
	byte	*pic, *pal;
	short		width, height;

	Prof_Begin(__FUNCTION__);

	// get the palette

	LoadPCX ("pics/colormap.pcx", &pic, &pal, &width, &height);
	if (!pal)
		ri.Sys_Error (ERR_FATAL, "Couldn't load pics/colormap.pcx");

	for (i=0 ; i<256 ; i++)
	{
		r = pal[i*3+0];
		g = pal[i*3+1];
		b = pal[i*3+2];
		d_8to24table[i] = GU_RGBA(r, g, b, 255);
	}

	d_8to24table[255] &= GU_RGBA(255, 255, 255, 0);	// 255 is transparent

	Z_Free (pic);
	Z_Free (pal);

	Prof_End();

	return 0;
}


/*
===============
GL_InitImages
===============
*/
void	GL_InitImages (void)
{
	int		i, j;

	Prof_Begin(__FUNCTION__);

	// init intensity conversions
	intensity = ri.Cvar_Get ("intensity", "2", 0);

	if ( intensity->value <= 1 )
		ri.Cvar_Set( "intensity", "1" );

	gl_state.inverse_intensity = 1 / intensity->value;

	ri.FS_LoadFile( "pics/16to8.dat", (void **)&gl_state.d_16to8table );
	if ( !gl_state.d_16to8table )
		ri.Sys_Error( ERR_FATAL, "Couldn't load pics/16to8.pcx");

	for ( i = 0; i < 256; i++ )
	{
		gammatable[i] = i;
	}

	for (i=0 ; i<256 ; i++)
	{
		j = i*intensity->value;
		if (j > 255)
			j = 255;
		intensitytable[i] = j;
	}

	Prof_End();
}

/*
===============
GL_ShutdownImages
===============
*/
void	GL_ShutdownImages (void)
{
	Prof_Begin(__FUNCTION__);

	GL_FreeImages();

	ri.FS_FreeFile(gl_state.d_16to8table);

	Prof_End();
}

void GL_FreeImages (void)
{
	int i;

	ri.Con_Printf(PRINT_DEVELOPER, "Freeing %d images.\n", numgltextures);

	for (i = 0; i < numgltextures; ++i)
	{
		image_t *const image = &gltextures[i];

		memset (image, 0, sizeof(*image));
	}

	numgltextures = 0;
}
