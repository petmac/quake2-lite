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

#include <stdio.h>
#include <math.h>

#include "../client/ref.h"

/*
====================================================================

PSP specifics.

====================================================================
*/

// PSP SDK.
#include <pspdisplay.h>
#include <pspge.h>
#include <pspgu.h>
#include <pspgum.h>

// Vertex type.
typedef struct {
	short u, v;
	short x, y, z;
} gu_2d_vertex_t;

#define GU_2D_VERTEX_TYPE (GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D)

// Debugging/profiling.
#if 0
#	define LOG_FUNCTION_ENTRY ri.Con_Printf(PRINT_DEVELOPER, "%s {\n", __FUNCTION__)
#	define LOG_FUNCTION_EXIT ri.Con_Printf(PRINT_DEVELOPER, "}\n")
#else
#	define LOG_FUNCTION_ENTRY do { } while (0)
#	define LOG_FUNCTION_EXIT do { } while (0)
#endif
#if 1
#	define ASSERT(x) do { if (!(x)) Sys_Error("ASSERT(%s) failed!. %s(%d)", #x, __FUNCTION__, __LINE__); } while (0)
#else
#	define ASSERT(x) do {} while (0)
#endif

#define STATIC_ASSERT(expr, msg)   \
	extern char STATIC_ASSERTION__##msg[1]; \
	extern char STATIC_ASSERTION__##msg[(expr)?1:2]

// Constants.
#define GU_SCR_WIDTH 480
#define GU_SCR_HEIGHT 272

// Display lists.
void GU_StartDisplayList(void);
void GU_FinishDisplayList(void);
void GU_SyncDisplayList(void);
qboolean GU_IsInDisplayList(void);

// VRAM.
#define GU_SCR_BUF_WIDTH 512

typedef ScePspRGB565 gu_pixel_t;

typedef struct framebuffers_s
{
	gu_pixel_t fb1[GU_SCR_BUF_WIDTH * GU_SCR_HEIGHT];
	gu_pixel_t fb2[GU_SCR_BUF_WIDTH * GU_SCR_HEIGHT];
	u16 depth[GU_SCR_BUF_WIDTH * GU_SCR_HEIGHT];
} framebuffers_t;

#define R_DISPLAY_LIST_SIZE ((2 * 1024 * 1024) - sizeof(framebuffers_t))

typedef struct vram_s
{
	framebuffers_t fb;
	char display_list[R_DISPLAY_LIST_SIZE];
} vram_t;

extern vram_t *vram;
extern gu_pixel_t *gu_back_buffer;

/*
====================================================================

Modified gl_local.h follows.

====================================================================
*/

#define	REF_VERSION	"GU 0.01"

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

*/

typedef enum 
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky
} imagetype_t;

typedef struct image_s
{
	// Set by GL_FindImage.
	char name[MAX_QPATH];			// game path, including extension
	imagetype_t type;

	// Set by GL_LoadPic.
	int width;
	int height;

	// Set by GL_Upload.
	int buffer_width;
	int buffer_height;
	void *data;						// texture address in VRAM.

	// Set at runtime.
	struct msurface_s *texturechain;	// for sort-by-texture world drawing
} image_t;

#define		MAX_GLTEXTURES	1024

//===================================================================

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "gu_model.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

void GL_SetDefaultState( void );

extern	float	gldepthmin, gldepthmax;

typedef struct
{
	float	s, t;
	float	x, y, z;
} glvert_t;

#define GLVERT_TYPE (GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D)

#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01


//====================================================

extern	image_t		gltextures[MAX_GLTEXTURES];
extern	int			numgltextures;

extern	entity_t	*currententity;
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_lefthand;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

extern	cvar_t	*gl_lightmap;
extern	cvar_t	*gl_shadows;
extern	cvar_t	*gl_dynamic;
extern  cvar_t  *gl_monolightmap;
extern	cvar_t	*gl_picmip;
extern	cvar_t	*gl_skymip;
extern	cvar_t	*gl_showtris;
extern	cvar_t	*gl_clear;
extern	cvar_t	*gl_cull;
extern	cvar_t	*gl_polyblend;
extern	cvar_t	*gl_flashblend;
extern	cvar_t	*gl_lightmaptype;
extern	cvar_t	*gl_modulate;
extern	cvar_t	*gl_playermip;
extern  cvar_t  *gl_saturatelighting;
extern  cvar_t  *gl_lockpvs;

extern	cvar_t		*intensity;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

void R_TranslatePlayerSkin (int playernum);
void GL_Bind (const image_t *texnum);
void GL_TexEnv( int value );

void R_LightPoint (vec3_t p, vec3_t color);
void R_PushDlights (void);

//====================================================================

extern	model_t	*r_worldmodel;

extern ScePspRGBA8888 d_8to24table[256];

void V_AddBlend (float r, float g, float b, float a, float *v_blend);

int 	R_Init( void *hinstance, void *hWnd );
void	R_Shutdown( void );

void R_RenderView (refdef_t *fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t *e);
void R_DrawBrushModel (entity_t *e);
void R_DrawSpriteModel (entity_t *e);
void R_DrawBeam( entity_t *e );
void R_DrawWorld (void);
void R_RenderDlights (void);
void R_DrawAlphaSurfaces (void);
void R_RenderBrushPoly (msurface_t *fa);
void Draw_InitLocal (void);
void GL_SubdivideSurface (msurface_t *fa);
qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_RotateForEntity (entity_t *e);
void R_MarkLeaves (void);

glpoly_t *WaterWarpPolyVerts (glpoly_t *p);
void EmitWaterPolys (msurface_t *fa);
void R_AddSkySurface (msurface_t *fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);

#if 0
short LittleShort (short l);
short BigShort (short l);
int	LittleLong (int l);
float LittleFloat (float f);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer
#endif

void COM_StripExtension (char *in, char *out);

void	Draw_GetPicSize (int *w, int *h, char *name);
void	Draw_Pic (int x, int y, char *name);
void	Draw_StretchPic (int x, int y, int w, int h, char *name);
void	Draw_Char (int x, int y, int c);
void	Draw_TileClear (int x, int y, int w, int h, char *name);
void	Draw_Fill (int x, int y, int w, int h, int c);
void	Draw_FadeScreen (void);
void	Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);

void	R_BeginFrame( float camera_separation );
void	R_SwapBuffers( int );
void	R_SetPalette ( const unsigned char *palette);

int		Draw_GetPalette (void);

void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight);

struct image_s *R_RegisterSkin (char *name);

image_t	*GL_FindImage (char *name, imagetype_t type);
void	GL_ImageList_f (void);

void	GL_SetTexturePalette(const ScePspRGBA8888 *palette);

void	GL_InitImages (void);
void	GL_ShutdownImages (void);
void	GL_FreeImages (void);

void GL_TextureAlphaMode( char *string );
void GL_TextureSolidMode( char *string );

typedef struct
{
	float inverse_intensity;

	unsigned char *d_16to8table;

	int lightmap_textures;

	const image_t *currenttexture;

	float camera_separation;
	qboolean stereo_enabled;
} glstate_t;

extern glstate_t   gl_state;

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern	refimport_t	ri;