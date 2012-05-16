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
// r_main.c
#include "gu_local.h"

void R_Clear (void);

refimport_t	ri;

model_t		*r_worldmodel;

float		gldepthmin, gldepthmax;

glstate_t  gl_state;

entity_t	*currententity;
model_t		*currentmodel;

cplane_t	frustum[4];

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

int			c_brush_polys, c_alias_polys;

float		v_blend[4];			// final blending color

//
// view origin
//
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

float	r_base_world_matrix[16];

vram_t *vram;
gu_pixel_t *gu_back_buffer;

//
// screen size info
//
refdef_t	r_newrefdef;

int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_lerpmodels;
cvar_t	*r_lefthand;

cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

cvar_t	*gl_lightmap;
cvar_t	*gl_shadows;
cvar_t	*gl_dynamic;
cvar_t  *gl_monolightmap;
cvar_t	*gl_modulate;
cvar_t	*gl_picmip;
cvar_t	*gl_skymip;
cvar_t	*gl_showtris;
cvar_t	*gl_clear;
cvar_t	*gl_cull;
cvar_t	*gl_polyblend;
cvar_t	*gl_flashblend;
cvar_t	*gl_playermip;
cvar_t  *gl_saturatelighting;
cvar_t	*gl_lockpvs;

/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	int		i;

	if (r_nocull->value)
		return false;

	for (i=0 ; i<4 ; i++)
		if ( BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}


void R_RotateForEntity (entity_t *e)
{
	ScePspFVector3 translation;

	ASSERT(GU_IsInDisplayList());

	translation.x = e->origin[0];
	translation.y = e->origin[1];
	translation.z = e->origin[2];

	sceGumTranslate(&translation);

    sceGumRotateZ (DEG2RAD(e->angles[1]));
    sceGumRotateY (DEG2RAD(-e->angles[0]));
    sceGumRotateX (DEG2RAD(-e->angles[2]));
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/


/*
=================
R_DrawSpriteModel

=================
*/
void R_DrawSpriteModel (entity_t *e)
{
#ifndef PSP
	float alpha = 1.0F;
	vec3_t	point;
	dsprframe_t	*frame;
	float		*up, *right;
	dsprite_t		*psprite;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	psprite = (dsprite_t *)currentmodel->extradata;

#if 0
	if (e->frame < 0 || e->frame >= psprite->numframes)
	{
		ri.Con_Printf (PRINT_ALL, "no such sprite frame %i\n", e->frame);
		e->frame = 0;
	}
#endif
	e->frame %= psprite->numframes;

	frame = &psprite->frames[e->frame];

#if 0
	if (psprite->type == SPR_ORIENTED)
	{	// bullet marks on walls
	vec3_t		v_forward, v_right, v_up;

	AngleVectors (currententity->angles, v_forward, v_right, v_up);
		up = v_up;
		right = v_right;
	}
	else
#endif
	{	// normal sprite
		up = vup;
		right = vright;
	}

	if ( e->flags & RF_TRANSLUCENT )
		alpha = e->alpha;

	if ( alpha != 1.0F )
		qglEnable( GU_BLEND );

	qglColor4f( 1, 1, 1, alpha );

    GL_Bind(currentmodel->skins[e->frame]->texnum);

	GL_TexEnv( GU_TFX_MODULATE );

	if ( alpha == 1.0 )
		qglEnable (GL_ALPHA_TEST);
	else
		qglDisable( GL_ALPHA_TEST );

	qglBegin (GL_QUADS);

	qglTexCoord2f (0, 1);
	VectorMA (e->origin, -frame->origin_y, up, point);
	VectorMA (point, -frame->origin_x, right, point);
	qglVertex3fv (point);

	qglTexCoord2f (0, 0);
	VectorMA (e->origin, frame->height - frame->origin_y, up, point);
	VectorMA (point, -frame->origin_x, right, point);
	qglVertex3fv (point);

	qglTexCoord2f (1, 0);
	VectorMA (e->origin, frame->height - frame->origin_y, up, point);
	VectorMA (point, frame->width - frame->origin_x, right, point);
	qglVertex3fv (point);

	qglTexCoord2f (1, 1);
	VectorMA (e->origin, -frame->origin_y, up, point);
	VectorMA (point, frame->width - frame->origin_x, right, point);
	qglVertex3fv (point);
	
	qglEnd ();

	qglDisable (GL_ALPHA_TEST);
	GL_TexEnv( GU_TFX_REPLACE );

	if ( alpha != 1.0F )
		qglDisable( GU_BLEND );

	qglColor4f( 1, 1, 1, 1 );
#endif
}

//==================================================================================

/*
=============
R_DrawNullModel
=============
*/
void R_DrawNullModel (void)
{
	vec3_t	shadelight;
	int		i;

	if ( currententity->flags & RF_FULLBRIGHT )
		shadelight[0] = shadelight[1] = shadelight[2] = 1.0F;
	else
		R_LightPoint (currententity->origin, shadelight);

#ifndef PSP
    qglPushMatrix ();
	R_RotateForEntity (currententity);

	qglDisable (GL_TEXTURE_2D);
	qglColor3fv (shadelight);

	qglBegin (GL_TRIANGLE_FAN);
	qglVertex3f (0, 0, -16);
	for (i=0 ; i<=4 ; i++)
		qglVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	qglEnd ();

	qglBegin (GL_TRIANGLE_FAN);
	qglVertex3f (0, 0, 16);
	for (i=4 ; i>=0 ; i--)
		qglVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	qglEnd ();

	qglColor3f (1,1,1);
	qglPopMatrix ();
	qglEnable (GL_TEXTURE_2D);
#endif
}

/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawEntitiesOnList (void)
{
	int		i;

	ASSERT(GU_IsInDisplayList());

	if (!r_drawentities->value)
		return;

	// draw non-transparent first
	for (i=0 ; i<r_newrefdef.num_entities ; i++)
	{
		currententity = &r_newrefdef.entities[i];
		if (currententity->flags & RF_TRANSLUCENT)
			continue;	// solid

		if ( currententity->flags & RF_BEAM )
		{
			R_DrawBeam( currententity );
		}
		else
		{
			currentmodel = currententity->model;
			if (!currentmodel)
			{
				R_DrawNullModel ();
				continue;
			}
			switch (currentmodel->type)
			{
			case mod_alias:
				R_DrawAliasModel (currententity);
				break;
			case mod_brush:
				R_DrawBrushModel (currententity);
				break;
			case mod_sprite:
				R_DrawSpriteModel (currententity);
				break;
			default:
				ri.Sys_Error (ERR_DROP, "Bad modeltype");
				break;
			}
		}
	}

	// draw transparent entities
	// we could sort these if it ever becomes a problem...
	sceGuDepthMask(GU_TRUE);		// no z writes
	for (i=0 ; i<r_newrefdef.num_entities ; i++)
	{
		currententity = &r_newrefdef.entities[i];
		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;	// solid

		if ( currententity->flags & RF_BEAM )
		{
			R_DrawBeam( currententity );
		}
		else
		{
			currentmodel = currententity->model;

			if (!currentmodel)
			{
				R_DrawNullModel ();
				continue;
			}
			switch (currentmodel->type)
			{
			case mod_alias:
				R_DrawAliasModel (currententity);
				break;
			case mod_brush:
				R_DrawBrushModel (currententity);
				break;
			case mod_sprite:
				R_DrawSpriteModel (currententity);
				break;
			default:
				ri.Sys_Error (ERR_DROP, "Bad modeltype");
				break;
			}
		}
	}
	sceGuDepthMask(GU_FALSE);		// back to writing
}

/*
===============
R_DrawParticles
===============
*/
typedef struct
{
	ScePspRGBA8888 colour;
	ScePspFVector3 position;
} gu_particle_vertex_t;

void R_DrawParticles (void)
{
	int i;
	unsigned char color[4];
	const particle_t *p;
	gu_particle_vertex_t *vertices;

	ASSERT(GU_IsInDisplayList());

	if (r_newrefdef.num_particles <= 0)
	{
		return;
	}

	vertices = sceGuGetMemory(sizeof(gu_particle_vertex_t) * r_newrefdef.num_particles);
	if (vertices == NULL)
	{
		return;
	}

	sceGuDepthMask(GU_TRUE);
	sceGuEnable(GU_BLEND);
	sceGuDisable(GU_TEXTURE_2D);

	for ( i = 0, p = r_newrefdef.particles; i < r_newrefdef.num_particles; i++, p++ )
	{
		gu_particle_vertex_t *const vertex = &vertices[i];
		const ScePspRGBA8888 rgbx = d_8to24table[p->color];
		const ScePspRGBA8888 rgbz = rgbx & GU_RGBA(255, 255, 255, 0);
		const ScePspRGBA8888 zzza = GU_RGBA(0, 0, 0, (int)(p->alpha * 255));

		vertex->colour = rgbz | zzza;
		vertex->position.x = p->origin[0];
		vertex->position.y = p->origin[1];
		vertex->position.z = p->origin[2];
	}

	sceGumDrawArray(GU_POINTS, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, r_newrefdef.num_particles, NULL, vertices);

	sceGuDisable(GU_BLEND);
	sceGuDepthMask(GU_FALSE);
	sceGuEnable(GU_TEXTURE_2D);
}

/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	if (!gl_polyblend->value)
		return;
	if (!v_blend[3])
		return;

	ASSERT(GU_IsInDisplayList());

#ifndef PSP
	qglDisable (GL_ALPHA_TEST);
	qglEnable (GU_BLEND);
	qglDisable (GL_DEPTH_TEST);
	qglDisable (GL_TEXTURE_2D);

    qglLoadIdentity ();

	// FIXME: get rid of these
    qglRotatef (-90,  1, 0, 0);	    // put Z going up
    qglRotatef (90,  0, 0, 1);	    // put Z going up

	qglColor4fv (v_blend);

	qglBegin (GL_QUADS);

	qglVertex3f (10, 100, 100);
	qglVertex3f (10, -100, 100);
	qglVertex3f (10, -100, -100);
	qglVertex3f (10, 100, -100);
	qglEnd ();

	qglDisable (GU_BLEND);
	qglEnable (GL_TEXTURE_2D);
	qglEnable (GL_ALPHA_TEST);

	qglColor4f(1,1,1,1);
#endif
}

//=======================================================================

int SignbitsForPlane (cplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}


void R_SetFrustum (void)
{
	int		i;

#if 0
	/*
	** this code is wrong, since it presume a 90 degree FOV both in the
	** horizontal and vertical plane
	*/
	// front side is visible
	VectorAdd (vpn, vright, frustum[0].normal);
	VectorSubtract (vpn, vright, frustum[1].normal);
	VectorAdd (vpn, vup, frustum[2].normal);
	VectorSubtract (vpn, vup, frustum[3].normal);

	// we theoretically don't need to normalize these vectors, but I do it
	// anyway so that debugging is a little easier
	VectorNormalize( frustum[0].normal );
	VectorNormalize( frustum[1].normal );
	VectorNormalize( frustum[2].normal );
	VectorNormalize( frustum[3].normal );
#else
	// rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector( frustum[0].normal, vup, vpn, -(90-r_newrefdef.fov_x / 2 ) );
	// rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector( frustum[1].normal, vup, vpn, 90-r_newrefdef.fov_x / 2 );
	// rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector( frustum[2].normal, vright, vpn, 90-r_newrefdef.fov_y / 2 );
	// rotate VPN down by FOV_X/2 degrees
	RotatePointAroundVector( frustum[3].normal, vright, vpn, -( 90 - r_newrefdef.fov_y / 2 ) );
#endif

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
	}
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	int i;
	mleaf_t	*leaf;

	r_framecount++;

// build the transformation matrix for the given view angles
	VectorCopy (r_newrefdef.vieworg, r_origin);

	AngleVectors (r_newrefdef.viewangles, vpn, vright, vup);

// current viewcluster
	if ( !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf (r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;

		// check above and below so crossing solid water doesn't draw wrong
		if (!leaf->contents)
		{	// look down a bit
			vec3_t	temp;

			VectorCopy (r_origin, temp);
			temp[2] -= 16;
			leaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2) )
				r_viewcluster2 = leaf->cluster;
		}
		else
		{	// look up a bit
			vec3_t	temp;

			VectorCopy (r_origin, temp);
			temp[2] += 16;
			leaf = Mod_PointInLeaf (temp, r_worldmodel);
			if ( !(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2) )
				r_viewcluster2 = leaf->cluster;
		}
	}

	for (i=0 ; i<4 ; i++)
		v_blend[i] = r_newrefdef.blend[i];

	c_brush_polys = 0;
	c_alias_polys = 0;

	// clear out the portion of the screen that the NOWORLDMODEL defines
	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
#ifndef PSP
		qglEnable( GL_SCISSOR_TEST );
		qglClearColor( 0.3, 0.3, 0.3, 1 );
		qglScissor( r_newrefdef.x, GU_SCR_HEIGHT - r_newrefdef.height - r_newrefdef.y, r_newrefdef.width, r_newrefdef.height );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		qglClearColor( 1, 0, 0.5, 0.5 );
		qglDisable( GL_SCISSOR_TEST );
#endif
	}
}

/*
=============
R_SetupGL
=============
*/
void R_SetupGL (void)
{
	float	screenaspect;
//	float	yfov;
	int		x, x2, y2, y, w, h;
	ScePspFVector3 view_translation;

	ASSERT(GU_IsInDisplayList());

	//
	// set up viewport
	//
	x = floor(r_newrefdef.x * GU_SCR_WIDTH / GU_SCR_WIDTH);
	x2 = ceil((r_newrefdef.x + r_newrefdef.width) * GU_SCR_WIDTH / GU_SCR_WIDTH);
	y = floor(GU_SCR_HEIGHT - r_newrefdef.y * GU_SCR_HEIGHT / GU_SCR_HEIGHT);
	y2 = ceil(GU_SCR_HEIGHT - (r_newrefdef.y + r_newrefdef.height) * GU_SCR_HEIGHT / GU_SCR_HEIGHT);

	w = x2 - x;
	h = y - y2;

#ifndef PSP
	qglViewport (x, y2, w, h);
#endif

	//
	// set up projection matrix
	//
    screenaspect = (float)r_newrefdef.width/r_newrefdef.height;
//	yfov = 2*atan((float)r_newrefdef.height/r_newrefdef.width)*180/M_PI;

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective (r_newrefdef.fov_y,  screenaspect,  4,  4096);

#ifndef PSP
	qglCullFace(GL_FRONT);
#endif

	sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

	sceGumRotateX (DEG2RAD(-90));	    // put Z going up
    sceGumRotateZ (DEG2RAD(90));	    // put Z going up
    sceGumRotateX (DEG2RAD(-r_newrefdef.viewangles[2]));
    sceGumRotateY (DEG2RAD(-r_newrefdef.viewangles[0]));
    sceGumRotateZ (DEG2RAD(-r_newrefdef.viewangles[1]));
	
	view_translation.x = -r_newrefdef.vieworg[0];
	view_translation.y = -r_newrefdef.vieworg[1];
	view_translation.z = -r_newrefdef.vieworg[2];
    sceGumTranslate (&view_translation);

//	if ( gl_state.camera_separation != 0 && gl_state.stereo_enabled )
//		qglTranslatef ( gl_state.camera_separation, 0, 0 );

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();

	//
	// set drawing parms
	//
	if (gl_cull->value)
		sceGuEnable(GU_CULL_FACE);
	else
		sceGuDisable(GU_CULL_FACE);

	sceGuDisable(GU_BLEND);
	sceGuEnable(GU_DEPTH_TEST);
}

/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	ASSERT(GU_IsInDisplayList());

	if (gl_clear->value)
	{
		sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	}
	else
	{
		sceGuClear(GU_DEPTH_BUFFER_BIT);
	}
}

void R_Flash( void )
{
	R_PolyBlend ();
}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/
void R_RenderView (refdef_t *fd)
{
	if (r_norefresh->value)
		return;

	r_newrefdef = *fd;

	if (!r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
		ri.Sys_Error (ERR_DROP, "R_RenderView: NULL worldmodel");

	if (r_speeds->value)
	{
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	R_PushDlights ();

#ifndef PSP
	if (gl_finish->value)
		qglFinish ();
#endif

	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGL ();

	R_MarkLeaves ();	// done here so we know if we're in water

	R_DrawWorld ();

	R_DrawEntitiesOnList ();

	R_RenderDlights ();

	R_DrawParticles ();

	R_DrawAlphaSurfaces ();

	R_Flash();

	if (r_speeds->value)
	{
		ri.Con_Printf (PRINT_ALL, "%4i wpoly %4i epoly %i tex %i lmaps\n",
			c_brush_polys, 
			c_alias_polys, 
			c_visible_textures, 
			c_visible_lightmaps); 
	}
}


void	R_SetGL2D (void)
{
	ASSERT(GU_IsInDisplayList());

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumOrtho(0, GU_SCR_WIDTH, GU_SCR_HEIGHT, 0, -99999, 99999);
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDisable(GU_CULL_FACE);
	sceGuEnable(GU_BLEND);
	sceGuColor(0xffffffff);
}

static void GL_DrawColoredStereoLinePair( float r, float g, float b, float y )
{
#ifndef PSP
	qglColor3f( r, g, b );
	qglVertex2f( 0, y );
	qglVertex2f( GU_SCR_WIDTH, y );
	qglColor3f( 0, 0, 0 );
	qglVertex2f( 0, y + 1 );
	qglVertex2f( GU_SCR_WIDTH, y + 1 );
#endif
}

static void GL_DrawStereoPattern( void )
{
#ifndef PSP
	int i;

	if ( !( gl_config.renderer & GL_RENDERER_INTERGRAPH ) )
		return;

	if ( !gl_state.stereo_enabled )
		return;

	R_SetGL2D();

	qglDrawBuffer( GL_BACK_LEFT );

	for ( i = 0; i < 20; i++ )
	{
		qglBegin( GL_LINES );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 0 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 2 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 4 );
			GL_DrawColoredStereoLinePair( 1, 0, 0, 6 );
			GL_DrawColoredStereoLinePair( 0, 1, 0, 8 );
			GL_DrawColoredStereoLinePair( 1, 1, 0, 10);
			GL_DrawColoredStereoLinePair( 1, 1, 0, 12);
			GL_DrawColoredStereoLinePair( 0, 1, 0, 14);
		qglEnd();

		GLimp_EndFrame();
	}
#endif
}


/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel (void)
{
	vec3_t		shadelight;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)

	R_LightPoint (r_newrefdef.vieworg, shadelight);

	// pick the greatest component, which should be the same
	// as the mono value returned by software
	if (shadelight[0] > shadelight[1])
	{
		if (shadelight[0] > shadelight[2])
			r_lightlevel->value = 150*shadelight[0];
		else
			r_lightlevel->value = 150*shadelight[2];
	}
	else
	{
		if (shadelight[1] > shadelight[2])
			r_lightlevel->value = 150*shadelight[1];
		else
			r_lightlevel->value = 150*shadelight[2];
	}

}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void R_RenderFrame (refdef_t *fd)
{
	LOG_FUNCTION_ENTRY;

	R_RenderView( fd );
	R_SetLightLevel ();
	R_SetGL2D ();

	LOG_FUNCTION_EXIT;
}


void R_Register( void )
{
	r_lefthand = ri.Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );
	r_norefresh = ri.Cvar_Get ("r_norefresh", "0", 0);
	r_fullbright = ri.Cvar_Get ("r_fullbright", "0", 0);
	r_drawentities = ri.Cvar_Get ("r_drawentities", "1", 0);
	r_drawworld = ri.Cvar_Get ("r_drawworld", "1", 0);
	r_novis = ri.Cvar_Get ("r_novis", "0", 0);
	r_nocull = ri.Cvar_Get ("r_nocull", "0", 0);
	r_lerpmodels = ri.Cvar_Get ("r_lerpmodels", "1", 0);
	r_speeds = ri.Cvar_Get ("r_speeds", "0", 0);

	r_lightlevel = ri.Cvar_Get ("r_lightlevel", "0", 0);

#ifndef PSP
	gl_particle_min_size = ri.Cvar_Get( "gl_particle_min_size", "2", CVAR_ARCHIVE );
	gl_particle_max_size = ri.Cvar_Get( "gl_particle_max_size", "40", CVAR_ARCHIVE );
	gl_particle_size = ri.Cvar_Get( "gl_particle_size", "40", CVAR_ARCHIVE );
	gl_particle_att_a = ri.Cvar_Get( "gl_particle_att_a", "0.01", CVAR_ARCHIVE );
	gl_particle_att_b = ri.Cvar_Get( "gl_particle_att_b", "0.0", CVAR_ARCHIVE );
	gl_particle_att_c = ri.Cvar_Get( "gl_particle_att_c", "0.01", CVAR_ARCHIVE );
#endif

	gl_modulate = ri.Cvar_Get ("gl_modulate", "1", CVAR_ARCHIVE );
	gl_lightmap = ri.Cvar_Get ("gl_lightmap", "0", 0);
	gl_shadows = ri.Cvar_Get ("gl_shadows", "0", CVAR_ARCHIVE );
	gl_dynamic = ri.Cvar_Get ("gl_dynamic", "1", 0);
	gl_picmip = ri.Cvar_Get ("gl_picmip", "0", 0);
	gl_skymip = ri.Cvar_Get ("gl_skymip", "0", 0);
	gl_showtris = ri.Cvar_Get ("gl_showtris", "0", 0);
	gl_clear = ri.Cvar_Get ("gl_clear", "1", 0); // TODO PeterM reset to 0.
	gl_cull = ri.Cvar_Get ("gl_cull", "1", 0);
	gl_polyblend = ri.Cvar_Get ("gl_polyblend", "1", 0);
	gl_flashblend = ri.Cvar_Get ("gl_flashblend", "0", 0);
	gl_playermip = ri.Cvar_Get ("gl_playermip", "0", 0);
	gl_monolightmap = ri.Cvar_Get( "gl_monolightmap", "0", 0 );
	gl_lockpvs = ri.Cvar_Get( "gl_lockpvs", "0", 0 );

	gl_saturatelighting = ri.Cvar_Get( "gl_saturatelighting", "0", 0 );

	ri.Cmd_AddCommand( "imagelist", GL_ImageList_f );
	ri.Cmd_AddCommand( "screenshot", GL_ScreenShot_f );
	ri.Cmd_AddCommand( "modellist", Mod_Modellist_f );
}

/*
===============
R_Init
===============
*/

int R_Init( void *hinstance, void *hWnd )
{	
	int		j;
	extern float r_turbsin[256];

	LOG_FUNCTION_ENTRY;

	// TODO PeterM This may get done more than once.
	for ( j = 0; j < 256; j++ )
	{
		r_turbsin[j] *= 0.5;
	}

	ri.Con_Printf (PRINT_ALL, "ref_gl version: "REF_VERSION"\n");

	Draw_GetPalette ();

	R_Register();

	vram = sceGeEdramGetAddr();
	gu_back_buffer = vram->fb.fb1;

	sceGuInit();
	GU_StartDisplayList();
	sceGuDrawBuffer(GU_PSM_5650, ((vram_t *)NULL)->fb.fb1, GU_SCR_BUF_WIDTH);
	sceGuDispBuffer(GU_SCR_WIDTH, GU_SCR_HEIGHT, ((vram_t *)NULL)->fb.fb2, GU_SCR_BUF_WIDTH);
	sceGuDepthBuffer(((vram_t *)NULL)->fb.depth, GU_SCR_BUF_WIDTH);

	GL_SetDefaultState();

	GU_FinishDisplayList();
	GU_SyncDisplayList();

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	ri.Vid_MenuInit();

#ifndef PSP
	if ( toupper( gl_monolightmap->string[1] ) != 'F' )
	{
		if ( gl_config.renderer == GL_RENDERER_PERMEDIA2 )
		{
			ri.Cvar_Set( "gl_monolightmap", "A" );
			ri.Con_Printf( PRINT_ALL, "...using gl_monolightmap 'a'\n" );
		}
		else if ( gl_config.renderer & GL_RENDERER_POWERVR ) 
		{
			ri.Cvar_Set( "gl_monolightmap", "0" );
		}
		else
		{
			ri.Cvar_Set( "gl_monolightmap", "0" );
		}
	}
#endif

#ifndef PSP
	// power vr can't have anything stay in the framebuffer, so
	// the screen needs to redraw the tiled background every frame
	if ( gl_config.renderer & GL_RENDERER_POWERVR ) 
	{
		ri.Cvar_Set( "scr_drawall", "1" );
	}
	else
#endif
	{
		ri.Cvar_Set( "scr_drawall", "0" );
	}

#ifdef __linux__
	ri.Cvar_SetValue( "gl_finish", 1 );
#endif

	/*
	** draw our stereo patterns
	*/
#if 0 // commented out until H3D pays us the money they owe us
	GL_DrawStereoPattern();
#endif

	GL_InitImages ();
	Mod_Init ();
	Draw_InitLocal ();

#ifndef PSP
	err = qglGetError();
	if ( err != GL_NO_ERROR )
		ri.Con_Printf (PRINT_ALL, "glGetError() = 0x%x\n", err);
#endif

	LOG_FUNCTION_EXIT;

	return true;
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown (void)
{
	LOG_FUNCTION_ENTRY;

	ri.Cmd_RemoveCommand ("modellist");
	ri.Cmd_RemoveCommand ("screenshot");
	ri.Cmd_RemoveCommand ("imagelist");
	ri.Cmd_RemoveCommand ("gl_strings");

	Mod_FreeAll ();

	GL_ShutdownImages ();

	sceGuTerm();

	LOG_FUNCTION_EXIT;
}



/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void R_BeginFrame( float camera_separation )
{
	LOG_FUNCTION_ENTRY;

	gl_state.camera_separation = camera_separation;

	GU_StartDisplayList();

	/*
	** go into 2D mode
	*/
	R_SetGL2D();

	//
	// clear screen if desired
	//
	R_Clear ();

	LOG_FUNCTION_EXIT;
}

/*
=============
R_SetPalette
=============
*/
gu_pixel_t r_rawpalette[256];

void R_SetPalette ( const unsigned char *palette)
{
	int		i;

	LOG_FUNCTION_ENTRY;

	if ( palette )
	{
		for ( i = 0; i < 256; i++ )
		{
			const int r = *palette++;
			const int g = *palette++;
			const int b = *palette++;
			r_rawpalette[i] = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
		}
	}

	LOG_FUNCTION_EXIT;
}

/*
** R_DrawBeam
*/
void R_DrawBeam( entity_t *e )
{
#define NUM_BEAM_SEGS 6

	int	i;
	float r, g, b;

	vec3_t perpvec;
	vec3_t direction, normalized_direction;
	vec3_t	start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	vec3_t oldorigin, origin;

	oldorigin[0] = e->oldorigin[0];
	oldorigin[1] = e->oldorigin[1];
	oldorigin[2] = e->oldorigin[2];

	origin[0] = e->origin[0];
	origin[1] = e->origin[1];
	origin[2] = e->origin[2];

	normalized_direction[0] = direction[0] = oldorigin[0] - origin[0];
	normalized_direction[1] = direction[1] = oldorigin[1] - origin[1];
	normalized_direction[2] = direction[2] = oldorigin[2] - origin[2];

	if ( VectorNormalize( normalized_direction ) == 0 )
		return;

	PerpendicularVector( perpvec, normalized_direction );
	VectorScale( perpvec, e->frame / 2, perpvec );

	for ( i = 0; i < 6; i++ )
	{
		RotatePointAroundVector( start_points[i], normalized_direction, perpvec, (360.0/NUM_BEAM_SEGS)*i );
		VectorAdd( start_points[i], origin, start_points[i] );
		VectorAdd( start_points[i], direction, end_points[i] );
	}

#ifndef PSP
	qglDisable( GL_TEXTURE_2D );
	qglEnable( GU_BLEND );
	qglDepthMask( GL_FALSE );
#endif

	r = ( d_8to24table[e->skinnum & 0xFF] ) & 0xFF;
	g = ( d_8to24table[e->skinnum & 0xFF] >> 8 ) & 0xFF;
	b = ( d_8to24table[e->skinnum & 0xFF] >> 16 ) & 0xFF;

	r *= 1/255.0F;
	g *= 1/255.0F;
	b *= 1/255.0F;

#ifndef PSP
	qglColor4f( r, g, b, e->alpha );

	qglBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i < NUM_BEAM_SEGS; i++ )
	{
		qglVertex3fv( start_points[i] );
		qglVertex3fv( end_points[i] );
		qglVertex3fv( start_points[(i+1)%NUM_BEAM_SEGS] );
		qglVertex3fv( end_points[(i+1)%NUM_BEAM_SEGS] );
	}
	qglEnd();

	qglEnable( GL_TEXTURE_2D );
	qglDisable( GU_BLEND );
	qglDepthMask( GL_TRUE );
#endif
}

//===================================================================


void	R_BeginRegistration (char *map);
struct model_s	*R_RegisterModel (char *name);
struct image_s	*R_RegisterSkin (char *name);
void R_SetSky (char *name, float rotate, vec3_t axis);
void	R_EndRegistration (void);

void	R_RenderFrame (refdef_t *fd);

struct image_s	*Draw_FindPic (char *name);

void	Draw_Pic (int x, int y, char *name);
void	Draw_Char (int x, int y, int c);
void	Draw_TileClear (int x, int y, int w, int h, char *name);
void	Draw_Fill (int x, int y, int w, int h, int c);
void	Draw_FadeScreen (void);

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
static void R_EndFrame (void)
{
	GU_FinishDisplayList();
	GU_SyncDisplayList();

	sceDisplayWaitVblankStart();
	gu_back_buffer = (gu_pixel_t *)((u32)sceGeEdramGetAddr() + (u32)sceGuSwapBuffers());
}

static void R_AppActivate (qboolean activate)
{
}

refexport_t GetRefAPI (refimport_t rimp )
{
	refexport_t	re;

	ri = rimp;

	re.api_version = API_VERSION;

	re.BeginRegistration = R_BeginRegistration;
	re.RegisterModel = R_RegisterModel;
	re.RegisterSkin = R_RegisterSkin;
	re.RegisterPic = Draw_FindPic;
	re.SetSky = R_SetSky;
	re.EndRegistration = R_EndRegistration;

	re.RenderFrame = R_RenderFrame;

	re.DrawGetPicSize = Draw_GetPicSize;
	re.DrawPic = Draw_Pic;
	re.DrawStretchPic = Draw_StretchPic;
	re.DrawChar = Draw_Char;
	re.DrawTileClear = Draw_TileClear;
	re.DrawFill = Draw_Fill;
	re.DrawFadeScreen= Draw_FadeScreen;

	re.DrawStretchRaw = Draw_StretchRaw;

	re.Init = R_Init;
	re.Shutdown = R_Shutdown;

	re.CinematicSetPalette = R_SetPalette;
	re.BeginFrame = R_BeginFrame;
	re.EndFrame = R_EndFrame;

	re.AppActivate = R_AppActivate;

	return re;
}


#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	ri.Sys_Error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	ri.Con_Printf (PRINT_ALL, "%s", text);
}

#endif
