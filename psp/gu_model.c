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
// models.c -- model loading and caching

#include "gu_local.h"

model_t	*loadmodel;
int		modfilelen;

void Mod_LoadSpriteModel (model_t *mod, void *buffer);
void Mod_LoadBrushModel (model_t *mod, FILE *file, long base);
void Mod_LoadAliasModel (model_t *mod, FILE *file, long base);
model_t *Mod_LoadModel (model_t *mod, qboolean crash);

byte	mod_novis[MAX_MAP_LEAFS/8];

#define	MAX_MOD_KNOWN	512
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

// the inline * models from the current map are kept seperate
model_t	mod_inline[MAX_MOD_KNOWN];

/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t		*node;
	float		d;
	cplane_t	*plane;

	if (!model || !model->nodes)
		ri.Sys_Error (ERR_DROP, "Mod_PointInLeaf: bad model");

	node = model->nodes;
	while (1)
	{
		if (node->contents != -1)
			return (mleaf_t *)node;
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return NULL;	// never reached
}


/*
===================
Mod_DecompressVis
===================
*/
byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->vis->numclusters+7)>>3;	
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;		
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);

	return decompressed;
}

/*
==============
Mod_ClusterPVS
==============
*/
byte *Mod_ClusterPVS (int cluster, model_t *model)
{
	if (cluster == -1 || !model->vis)
		return mod_novis;
	return Mod_DecompressVis ( (byte *)model->vis + model->vis->bitofs[cluster][DVIS_PVS],
		model);
}


//===============================================================================

/*
================
Mod_Modellist_f
================
*/
void Mod_Modellist_f (void)
{
	int		i;
	model_t	*mod;
	int		total;

	total = 0;
	ri.Con_Printf (PRINT_ALL,"Loaded models:\n");
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		ri.Con_Printf (PRINT_ALL, "%8i : %s\n",mod->extradatasize, mod->name);
		total += mod->extradatasize;
	}
	ri.Con_Printf (PRINT_ALL, "Total resident: %i\n", total);
}

/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	memset (mod_novis, 0xff, sizeof(mod_novis));
}



/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (char *name, qboolean crash)
{
	model_t	*mod;
	int		i;
	FILE *file;
	long base;
	int type;
	char *extradata;

	if (!name[0])
		ri.Sys_Error (ERR_DROP, "Mod_ForName: NULL name");

	//
	// inline models are grabbed only from worldmodel
	//
	if (name[0] == '*')
	{
		i = atoi(name+1);
		if (i < 1 || !r_worldmodel || i >= r_worldmodel->numsubmodels)
			ri.Sys_Error (ERR_DROP, "bad inline model number");
		return &mod_inline[i];
	}

	//
	// search the currently loaded models
	//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		if (!strcmp (mod->name, name) )
			return mod;
	}

	//
	// find a free model slot spot
	//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			break;	// free spot
	}
	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			ri.Sys_Error (ERR_DROP, "mod_numknown == MAX_MOD_KNOWN");
		mod_numknown++;
	}
	strcpy (mod->name, name);

	//
	// load the file
	//
	modfilelen = ri.FS_FOpenFile (mod->name, &file);
	if (modfilelen < 0)
	{
		if (crash)
			ri.Sys_Error (ERR_DROP, "Mod_NumForName: %s not found", mod->name);
		memset (mod->name, 0, sizeof(mod->name));
		return NULL;
	}

	loadmodel = mod;
	extradata = Hunk_Alloc(&hunk_ref, 0);

	//
	// fill it in
	//

	base = ftell(file);

	// call the apropriate loader
	ri.FS_Read(&type, sizeof(type), file);

	fseek(file, base, SEEK_SET);

	switch (LittleLong(type))
	{
	case IDALIASHEADER:
		Mod_LoadAliasModel (mod, file, base);
		break;

	case IDSPRITEHEADER:
		{
			void *buf;

			buf = Z_Malloc(modfilelen); // TODO PeterM This is totally wrong.
			ri.FS_Read(buf, modfilelen, file); // TODO PeterM This is totally wrong.

			Mod_LoadSpriteModel (mod, buf);

			Z_Free(buf); // TODO PeterM This is totally wrong.
		}
		break;

	case IDBSPHEADER:
		Mod_LoadBrushModel (mod, file, base);
		break;

	default:
		ri.Sys_Error (ERR_DROP,"Mod_NumForName: unknown fileid for %s", mod->name);
		break;
	}

	loadmodel->extradatasize = (char *)Hunk_Alloc (&hunk_ref, 0) - extradata;

	ri.FS_FCloseFile(file);

	return mod;
}

/*
===============================================================================

BRUSHMODEL LOADING

===============================================================================
*/

/*
=================
Mod_LoadLighting
=================
*/
void Mod_LoadLighting (lump_t *l, FILE *file, long base)
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}

	fseek(file, base + l->fileofs, SEEK_SET);

	loadmodel->lightdata = Hunk_Alloc (&hunk_ref, l->filelen);

	ri.FS_Read(loadmodel->lightdata, l->filelen, file);
}


/*
=================
Mod_LoadVisibility
=================
*/
void Mod_LoadVisibility (lump_t *l, FILE *file, long base)
{
	int		i;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (!l->filelen)
	{
		loadmodel->vis = NULL;
		return;
	}

	loadmodel->vis = Hunk_Alloc (&hunk_ref, l->filelen);
	ri.FS_Read (loadmodel->vis, l->filelen, file);
}


/*
=================
Mod_LoadVertexes
=================
*/
void Mod_LoadVertexes (lump_t *l, FILE *file, long base)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc (&hunk_ref, count*sizeof(*out));	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	in = Z_Malloc(l->filelen);
	ri.FS_Read(in, l->filelen, file);
	
	for ( i=0 ; i<count ; i++)
	{
		VectorCopy(in[i].point, out[i].position);
	}

	Z_Free(in);
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (vec3_t mins, vec3_t maxs)
{
	int		i;
	vec3_t	corner;

	for (i=0 ; i<3 ; i++)
	{
		corner[i] = fabsf(mins[i]) > fabsf(maxs[i]) ? fabsf(mins[i]) : fabsf(maxs[i]);
	}

	return VectorLength (corner);
}


/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l, FILE *file, long base)
{
	dmodel_t	in[MAX_MAP_MODELS];
	mmodel_t	*out;
	int			i, j, count;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(dmodel_t))
		ri.Sys_Error (ERR_DROP, "Mod_LoadSubmodels: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(dmodel_t);
	if (count > MAX_MAP_MODELS)
		ri.Sys_Error(ERR_DROP, "%s: Too many (%d) in %s", __FUNCTION__, count, loadmodel->name);
	out = Hunk_Alloc (&hunk_ref, count*sizeof(*out));	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	ri.FS_Read(in, l->filelen, file);

	for ( i=0 ; i<count ; i++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in[i].mins[j]) - 1;
			out->maxs[j] = LittleFloat (in[i].maxs[j]) + 1;
			out->origin[j] = LittleFloat (in[i].origin[j]);
		}
		out->radius = RadiusFromBounds (out->mins, out->maxs);
		out->headnode = LittleLong (in[i].headnode);
		out->firstface = LittleLong (in[i].firstface);
		out->numfaces = LittleLong (in[i].numfaces);
	}
}

/*
=================
Mod_LoadEdges
=================
*/
void Mod_LoadEdges (lump_t *l, FILE *file, long base)
{
	dedge_t *out;
	int 	i, count;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(dedge_t))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(dedge_t);
	out = Hunk_Alloc (&hunk_ref, l->fileofs);	

	loadmodel->edges = out;
	loadmodel->numedges = count;

	ri.FS_Read(out, l->fileofs, file);
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Mod_LoadTexinfo (lump_t *l, FILE *file, long base)
{
	texinfo_t in[MAX_MAP_TEXINFO];
	mtexinfo_t *out, *step;
	int 	i, j, count;
	char	name[MAX_QPATH];
	int		next;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(texinfo_t))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(texinfo_t);
	if (count > MAX_MAP_TEXINFO)
		ri.Sys_Error(ERR_DROP, "%s: Too many (%d) in %s", __FUNCTION__, count, loadmodel->name);
	out = Hunk_Alloc (&hunk_ref, count*sizeof(*out));	

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	ri.FS_Read(in, l->filelen, file);

	for ( i=0 ; i<count ; i++, out++)
	{
		for (j=0 ; j<8 ; j++)
			out->vecs[0][j] = LittleFloat (in[i].vecs[0][j]);

		out->flags = LittleLong (in[i].flags);
		next = LittleLong (in[i].nexttexinfo);
		if (next > 0)
			out->next = loadmodel->texinfo + next;
		else
			out->next = NULL;
		Com_sprintf (name, sizeof(name), "textures/%s.wal", in[i].texture);

		out->image = GL_FindImage (name, it_wall);
		if (!out->image)
		{
			ri.Con_Printf (PRINT_ALL, "Couldn't load %s\n", name);
		}
	}

	// count animation frames
	for (i=0 ; i<count ; i++)
	{
		out = &loadmodel->texinfo[i];
		out->numframes = 1;
		for (step = out->next ; step && step != out ; step=step->next)
			out->numframes++;
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
static void CalcSurfaceExtents (msurface_t *s)
{
	float	mins[2], maxs[2], val;
	int		i,j, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;

	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];

		for (j=0 ; j<2 ; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + 
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{	
		bmins[i] = floorf(mins[i]/16);
		bmaxs[i] = ceilf(maxs[i]/16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;

		//		if ( !(tex->flags & TEX_SPECIAL) && s->extents[i] > 512 /* 256 */ )
		//			ri.Sys_Error (ERR_DROP, "Bad surface extents");
	}
}


void GL_BuildPolygonFromSurface(msurface_t *fa);
void GL_CreateSurfaceLightmap (msurface_t *surf);
void GL_EndBuildingLightmaps (void);
void GL_BeginBuildingLightmaps (model_t *m);

/*
=================
Mod_LoadFaces
=================
*/
void Mod_LoadFaces (lump_t *l, FILE *file, long base)
{
	dface_t		*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;
	int			ti;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc (&hunk_ref, count*sizeof(*out));	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	currentmodel = loadmodel;

	in = Z_Malloc(l->filelen);
	ri.FS_Read(in, l->filelen, file);

	GL_BeginBuildingLightmaps (loadmodel);

	for ( surfnum=0 ; surfnum<count ; surfnum++, out++)
	{
		out->firstedge = LittleLong(in[surfnum].firstedge);
		out->numedges = LittleShort(in[surfnum].numedges);		
		out->flags = 0;
		out->polys = NULL;

		planenum = LittleShort(in[surfnum].planenum);
		side = LittleShort(in[surfnum].side);
		if (side)
			out->flags |= SURF_PLANEBACK;			

		out->plane = loadmodel->planes + planenum;

		ti = LittleShort (in[surfnum].texinfo);
		if (ti < 0 || ti >= loadmodel->numtexinfo)
			ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: bad texinfo number");
		out->texinfo = loadmodel->texinfo + ti;

		CalcSurfaceExtents (out);

		// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in[surfnum].styles[i];
		i = LittleLong(in[surfnum].lightofs);
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + i;

		// set the drawing flags

		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			GL_SubdivideSurface (out);	// cut up polygon for warps
		}

		// create lightmaps and polygons
		if ( !(out->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP) ) )
			GL_CreateSurfaceLightmap (out);

		if (! (out->texinfo->flags & SURF_WARP) ) 
			GL_BuildPolygonFromSurface(out);

	}

	GL_EndBuildingLightmaps ();

	Z_Free(in);
}


/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents != -1)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
void Mod_LoadNodes (lump_t *l, FILE *file, long base)
{
	int			i, j, count, p;
	dnode_t		in[MAX_MAP_NODES];
	mnode_t 	*out;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(dnode_t))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(dnode_t);
	if (count > MAX_MAP_NODES)
		ri.Sys_Error(ERR_DROP, "%s: Too many (%d) in %s", __FUNCTION__, count, loadmodel->name);
	out = Hunk_Alloc (&hunk_ref, count*sizeof(*out));	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	ri.FS_Read(in, l->filelen, file);

	for ( i=0 ; i<count ; i++, out++)
	{
		dnode_t *pin = &in[i];

		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (pin->mins[j]);
			out->minmaxs[3+j] = LittleShort (pin->maxs[j]);
		}

		p = LittleLong(pin->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (pin->firstface);
		out->numsurfaces = LittleShort (pin->numfaces);
		out->contents = -1;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (pin->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}

	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
void Mod_LoadLeafs (lump_t *l, FILE *file, long base)
{
	dleaf_t 	in[MAX_MAP_LEAFS];
	mleaf_t 	*out;
	int			i, j, count, p;
	//	glpoly_t	*poly;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(dleaf_t))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(dleaf_t);
	if (count > MAX_MAP_LEAFS)
		ri.Sys_Error(ERR_DROP, "%s: Too many (%d) in %s", __FUNCTION__, count, loadmodel->name);
	out = Hunk_Alloc (&hunk_ref, count*sizeof(*out));	

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	ri.FS_Read(in, l->filelen, file);

	for ( i=0 ; i<count ; i++, out++)
	{
		const dleaf_t *const pin = &in[i];

		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (pin->mins[j]);
			out->minmaxs[3+j] = LittleShort (pin->maxs[j]);
		}

		p = LittleLong(pin->contents);
		out->contents = p;

		out->cluster = LittleShort(pin->cluster);
		out->area = LittleShort(pin->area);

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort(pin->firstleafface);
		out->nummarksurfaces = LittleShort(pin->numleaffaces);

		// gl underwater warp
#if 0
		if (out->contents & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_THINWATER) )
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
			{
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
				for (poly = out->firstmarksurface[j]->polys ; poly ; poly=poly->next)
					poly->flags |= SURF_UNDERWATER;
			}
		}
#endif
	}	
}

/*
=================
Mod_LoadMarksurfaces
=================
*/
void Mod_LoadMarksurfaces (lump_t *l, FILE *file, long base)
{	
	int		i, j, count;
	short		*in;
	msurface_t **out;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(short))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(short);
	out = Hunk_Alloc (&hunk_ref, count*sizeof(*out));	

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	in = Z_Malloc(l->filelen);
	ri.FS_Read(in, l->filelen, file);

	for ( i=0 ; i<count ; i++)
	{
		j = LittleShort(in[i]);
		if (j < 0 ||  j >= loadmodel->numsurfaces)
			ri.Sys_Error (ERR_DROP, "Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}

	Z_Free(in);
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Mod_LoadSurfedges (lump_t *l, FILE *file, long base)
{	
	int		i, count;
	int		*out;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(int))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(int);

	out = Hunk_Alloc (&hunk_ref, l->filelen);	

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	ri.FS_Read(out, l->filelen, file);
}


/*
=================
Mod_LoadPlanes
=================
*/
void Mod_LoadPlanes (lump_t *l, FILE *file, long base)
{
	int			i, j;
	cplane_t	*out;
	dplane_t 	in[MAX_MAP_PLANES];
	int			count;

	fseek(file, base + l->fileofs, SEEK_SET);

	if (l->filelen % sizeof(dplane_t))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(dplane_t);
	if (count > MAX_MAP_PLANES)
		ri.Sys_Error(ERR_DROP, "%s: Too many (%d) in %s", __FUNCTION__, count, loadmodel->name);
	out = Hunk_Alloc (&hunk_ref, count * sizeof(*out));	

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	ri.FS_Read(in, l->filelen, file);

	for ( i=0 ; i<count ; i++, out++)
	{
		VectorCopy(in[i].normal, out->normal);
		out->dist = LittleFloat (in[i].dist);
	}
}

/*
=================
Mod_LoadBrushModel
=================
*/
void Mod_LoadBrushModel (model_t *mod, FILE *file, long base)
{
	int			i;
	dheader_t	header;
	mmodel_t 	*bm;

	loadmodel->type = mod_brush;
	if (loadmodel != mod_known)
		ri.Sys_Error (ERR_DROP, "Loaded a brush model after the world");

	ri.FS_Read(&header, sizeof(header), file);

	i = LittleLong (header.version);
	if (i != BSPVERSION)
		ri.Sys_Error (ERR_DROP, "Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, i, BSPVERSION);

	// swap all the lumps

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);

	// load into heap

	Mod_LoadVertexes (&header.lumps[LUMP_VERTEXES], file, base);
	Mod_LoadEdges (&header.lumps[LUMP_EDGES], file, base);
	Mod_LoadSurfedges (&header.lumps[LUMP_SURFEDGES], file, base);
	Mod_LoadLighting (&header.lumps[LUMP_LIGHTING], file, base);
	Mod_LoadPlanes (&header.lumps[LUMP_PLANES], file, base);
	Mod_LoadTexinfo (&header.lumps[LUMP_TEXINFO], file, base);
	Mod_LoadFaces (&header.lumps[LUMP_FACES], file, base);
	Mod_LoadMarksurfaces (&header.lumps[LUMP_LEAFFACES], file, base);
	Mod_LoadVisibility (&header.lumps[LUMP_VISIBILITY], file, base);
	Mod_LoadLeafs (&header.lumps[LUMP_LEAFS], file, base);
	Mod_LoadNodes (&header.lumps[LUMP_NODES], file, base);
	Mod_LoadSubmodels (&header.lumps[LUMP_MODELS], file, base);
	mod->numframes = 2;		// regular and alternate animation

	//
	// set up the submodels
	//
	for (i=0 ; i<mod->numsubmodels ; i++)
	{
		model_t	*starmod;

		bm = &mod->submodels[i];
		starmod = &mod_inline[i];

		*starmod = *loadmodel;

		starmod->firstmodelsurface = bm->firstface;
		starmod->nummodelsurfaces = bm->numfaces;
		starmod->firstnode = bm->headnode;
		if (starmod->firstnode >= loadmodel->numnodes)
			ri.Sys_Error (ERR_DROP, "Inline model %i has bad firstnode", i);

		VectorCopy (bm->maxs, starmod->maxs);
		VectorCopy (bm->mins, starmod->mins);
		starmod->radius = bm->radius;

		if (i == 0)
			*loadmodel = *starmod;

		starmod->numleafs = bm->visleafs;
	}
}

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

/*
=================
Mod_LoadAliasModel
=================
*/
void Mod_LoadAliasModel (model_t *mod, FILE *file, long base)
{
	int				i, j;
	dmdl_t			inmodel;
	mmdl_t			*pheader;
	daliasframe_t	*poutframe;
	int				*poutcmd;

	ri.FS_Read(&inmodel, sizeof(inmodel), file);

	if (inmodel.version != ALIAS_VERSION)
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)",
		mod->name, inmodel.version, ALIAS_VERSION);

	pheader = &mod->alias;

	pheader->framesize = inmodel.framesize;
	pheader->num_xyz = inmodel.num_xyz;
	pheader->num_tris = inmodel.num_tris;
	pheader->num_frames = inmodel.num_frames;

	if (pheader->num_xyz <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no vertices", mod->name);

	if (pheader->num_xyz > MAX_VERTS)
		ri.Sys_Error (ERR_DROP, "model %s has too many vertices", mod->name);

	if (pheader->num_tris <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no triangles", mod->name);

	if (pheader->num_frames <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no frames", mod->name);

	mod->type = mod_alias;

	//
	// load the frames
	//
	pheader->frames = Hunk_Alloc(&hunk_ref, pheader->num_frames * pheader->framesize);

	fseek(file, base + inmodel.ofs_frames, SEEK_SET);
	ri.FS_Read(pheader->frames, pheader->num_frames * pheader->framesize, file);

	mod->type = mod_alias;

	//
	// load the glcmds
	//

	pheader->glcmds = Hunk_Alloc(&hunk_ref, inmodel.num_glcmds * sizeof(int));

	fseek(file, base + inmodel.ofs_glcmds, SEEK_SET);
	ri.FS_Read(pheader->glcmds, inmodel.num_glcmds * sizeof(int), file);

	// Count the number of vertices.
	pheader->num_vertices = 0;
	poutcmd = pheader->glcmds;
	while (1)
	{
		int count;
		
		count = *poutcmd;
		if (count < 0)
		{
			count = -count;
		}
		else if (count == 0)
		{
			break;
		}
		
		pheader->num_vertices += count;

		poutcmd += ((count * 3) + 1);
	}

	// register all skins
	fseek(file, base + inmodel.ofs_skins, SEEK_SET);
	for (i=0 ; i<inmodel.num_skins ; i++)
	{
		char name[MAX_SKINNAME];

		ri.FS_Read(name, MAX_SKINNAME, file);

		assert(mod->skins[i] == NULL);
		mod->skins[i] = GL_FindImage (name, it_skin);
	}

	// TODO PeterM Is this used anywhere?
	mod->numframes = pheader->num_frames;

	mod->mins[0] = -32;
	mod->mins[1] = -32;
	mod->mins[2] = -32;
	mod->maxs[0] = 32;
	mod->maxs[1] = 32;
	mod->maxs[2] = 32;
}

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/

/*
=================
Mod_LoadSpriteModel
=================
*/
void Mod_LoadSpriteModel (model_t *mod, void *buffer)
{
	dsprite_t	*sprin, *sprout;
	int			i;

	sprin = (dsprite_t *)buffer;
	sprout = Hunk_Alloc (&hunk_ref, modfilelen);
	mod->sprite = sprout;

	sprout->ident = LittleLong (sprin->ident);
	sprout->version = LittleLong (sprin->version);
	sprout->numframes = LittleLong (sprin->numframes);

	if (sprout->version != SPRITE_VERSION)
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)",
		mod->name, sprout->version, SPRITE_VERSION);

	if (sprout->numframes > MAX_MD2SKINS)
		ri.Sys_Error (ERR_DROP, "%s has too many frames (%i > %i)",
		mod->name, sprout->numframes, MAX_MD2SKINS);

	// byte swap everything
	for (i=0 ; i<sprout->numframes ; i++)
	{
		sprout->frames[i].width = LittleLong (sprin->frames[i].width);
		sprout->frames[i].height = LittleLong (sprin->frames[i].height);
		sprout->frames[i].origin_x = LittleLong (sprin->frames[i].origin_x);
		sprout->frames[i].origin_y = LittleLong (sprin->frames[i].origin_y);
		memcpy (sprout->frames[i].name, sprin->frames[i].name, MAX_SKINNAME);
		mod->skins[i] = GL_FindImage (sprout->frames[i].name,
			it_sprite);
	}

	mod->type = mod_sprite;
}

//=============================================================================

/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginRegistration

Specifies the model that will be used as the world
@@@@@@@@@@@@@@@@@@@@@
*/

void R_BeginRegistration (char *model)
{
	char	fullname[MAX_QPATH];

	LOG_FUNCTION_ENTRY;

	r_oldviewcluster = -1;		// force markleafs

	Com_sprintf (fullname, sizeof(fullname), "maps/%s.bsp", model);

	Mod_FreeAll ();

	GL_FreeImages();

	Hunk_Begin (&hunk_ref);

	Draw_InitLocal ();

	r_worldmodel = Mod_ForName(fullname, true);

	r_viewcluster = -1;

	LOG_FUNCTION_EXIT;
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_RegisterModel

@@@@@@@@@@@@@@@@@@@@@
*/
struct model_s *R_RegisterModel (char *name)
{
	model_t	*mod;
	int		i;

	LOG_FUNCTION_ENTRY;

	mod = Mod_ForName (name, false);
	if (mod)
	{
		// register any images used by the models
		if (mod->type == mod_brush)
		{
#ifndef PSP
			for (i=0 ; i<mod->numtexinfo ; i++)
				mod->texinfo[i].image->registration_sequence = registration_sequence;
#endif
		}
	}

	LOG_FUNCTION_EXIT;

	return mod;
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_EndRegistration

@@@@@@@@@@@@@@@@@@@@@
*/
void R_EndRegistration (void)
{
	LOG_FUNCTION_ENTRY;

	Hunk_End(&hunk_ref);

	LOG_FUNCTION_EXIT;
}


//=============================================================================


/*
================
Mod_Free
================
*/
void Mod_Free (model_t *mod)
{
	memset (mod, 0, sizeof(*mod));
}

/*
================
Mod_FreeAll
================
*/
void Mod_FreeAll (void)
{
	int		i;

	for (i=0 ; i<mod_numknown ; i++)
	{
		Mod_Free (&mod_known[i]);
	}
}
