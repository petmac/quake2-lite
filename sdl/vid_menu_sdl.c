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
#include "../client/client.h"
#include "../client/qmenu.h"

#define REF_SOFT	0
#define REF_OPENGL	1

extern cvar_t *vid_fullscreen;
extern cvar_t *vid_gamma;
extern cvar_t *scr_viewsize;

static cvar_t *gl_mode;
static cvar_t *gl_driver;
static cvar_t *gl_picmip;
static cvar_t *gl_ext_palettedtexture;

static cvar_t *sw_mode;
static cvar_t *sw_stipplealpha;

static cvar_t *_windowed_mouse;

extern void M_ForceMenuOff( void );

/*
====================================================================

MENU INTERACTION

====================================================================
*/
static menuframework_s	s_menu;

static menulist_s		s_mode_list;
static menuslider_s		s_tq_slider;
static menuslider_s		s_screensize_slider;
static menuslider_s		s_brightness_slider;
static menulist_s  		s_fs_box;
static menulist_s  		s_stipple_box;
static menulist_s  		s_paletted_texture_box;
static menulist_s  		s_windowed_mouse;
static menuaction_s		s_apply_action;
static menuaction_s		s_defaults_action;

static void ScreenSizeCallback( void *s )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	Cvar_SetValue( "viewsize", slider->curvalue * 10 );
}

static void BrightnessCallback( void *s )
{
	menuslider_s *slider = ( menuslider_s * ) s;
}

static void ResetDefaults( void *unused )
{
	VID_MenuInit();
}

static void ApplyChanges( void *unused )
{
	float gamma;

	/*
	** invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
	*/
	gamma = ( 0.8 - ( s_brightness_slider.curvalue/10.0 - 0.5 ) ) + 0.5;

	Cvar_SetValue( "vid_gamma", gamma );
	Cvar_SetValue( "sw_stipplealpha", s_stipple_box.curvalue );
	Cvar_SetValue( "gl_picmip", 3 - s_tq_slider.curvalue );
	Cvar_SetValue( "vid_fullscreen", s_fs_box.curvalue );
	Cvar_SetValue( "gl_ext_palettedtexture", s_paletted_texture_box.curvalue );
	Cvar_SetValue( "gl_mode", s_mode_list.curvalue );
	Cvar_SetValue( "_windowed_mouse", s_windowed_mouse.curvalue);

	M_ForceMenuOff();
}

/*
** VID_MenuInit
*/
void VID_MenuInit( void )
{
	static const char *resolutions[] = 
	{
		"[320 240  ]",
		"[400 300  ]",
		"[512 384  ]",
		"[640 480  ]",
		"[800 600  ]",
		"[960 720  ]",
		"[1024 768 ]",
		"[1152 864 ]",
		"[1280 1024]",
		"[1600 1200]",
		"[2048 1536]",
		0
	};
	static const char *yesno_names[] =
	{
		"no",
		"yes",
		0
	};

	if ( !gl_driver )
		gl_driver = Cvar_Get( "gl_driver", "opengl32", 0 );
	if ( !gl_picmip )
		gl_picmip = Cvar_Get( "gl_picmip", "0", 0 );
	if ( !gl_mode )
		gl_mode = Cvar_Get( "gl_mode", "3", 0 );
	if ( !sw_mode )
		sw_mode = Cvar_Get( "sw_mode", "0", 0 );
	if ( !gl_ext_palettedtexture )
		gl_ext_palettedtexture = Cvar_Get( "gl_ext_palettedtexture", "1", CVAR_ARCHIVE );

	if ( !sw_stipplealpha )
		sw_stipplealpha = Cvar_Get( "sw_stipplealpha", "0", CVAR_ARCHIVE );

	if ( !_windowed_mouse)
        _windowed_mouse = Cvar_Get( "_windowed_mouse", "0", CVAR_ARCHIVE );

	s_mode_list.curvalue = gl_mode->value;

	if ( !scr_viewsize )
		scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);

	s_screensize_slider.curvalue = scr_viewsize->value/10;

	s_menu.x = viddef.width * 0.50;
	s_menu.nitems = 0;

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.name = "video mode";
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = 10;
	s_mode_list.itemnames = resolutions;

	s_screensize_slider.generic.type	= MTYPE_SLIDER;
	s_screensize_slider.generic.x		= 0;
	s_screensize_slider.generic.y		= 20;
	s_screensize_slider.generic.name	= "screen size";
	s_screensize_slider.minvalue = 3;
	s_screensize_slider.maxvalue = 12;
	s_screensize_slider.generic.callback = ScreenSizeCallback;

	s_brightness_slider.generic.type	= MTYPE_SLIDER;
	s_brightness_slider.generic.x	= 0;
	s_brightness_slider.generic.y	= 30;
	s_brightness_slider.generic.name	= "brightness";
	s_brightness_slider.generic.callback = BrightnessCallback;
	s_brightness_slider.minvalue = 5;
	s_brightness_slider.maxvalue = 13;
	s_brightness_slider.curvalue = ( 1.3 - vid_gamma->value + 0.5 ) * 10;

	s_fs_box.generic.type = MTYPE_SPINCONTROL;
	s_fs_box.generic.x	= 0;
	s_fs_box.generic.y	= 40;
	s_fs_box.generic.name	= "fullscreen";
	s_fs_box.itemnames = yesno_names;
	s_fs_box.curvalue = vid_fullscreen->value;

	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.name = "reset to default";
	s_defaults_action.generic.x    = 0;
	s_defaults_action.generic.y    = 90;
	s_defaults_action.generic.callback = ResetDefaults;

	s_apply_action.generic.type = MTYPE_ACTION;
	s_apply_action.generic.name = "apply";
	s_apply_action.generic.x    = 0;
	s_apply_action.generic.y    = 100;
	s_apply_action.generic.callback = ApplyChanges;

	s_stipple_box.generic.type = MTYPE_SPINCONTROL;
	s_stipple_box.generic.x	= 0;
	s_stipple_box.generic.y	= 60;
	s_stipple_box.generic.name	= "stipple alpha";
	s_stipple_box.curvalue = sw_stipplealpha->value;
	s_stipple_box.itemnames = yesno_names;

	s_windowed_mouse.generic.type = MTYPE_SPINCONTROL;
	s_windowed_mouse.generic.x  = 0;
	s_windowed_mouse.generic.y  = 72;
	s_windowed_mouse.generic.name   = "windowed mouse";
	s_windowed_mouse.curvalue = _windowed_mouse->value;
	s_windowed_mouse.itemnames = yesno_names;

	s_tq_slider.generic.type	= MTYPE_SLIDER;
	s_tq_slider.generic.x		= 0;
	s_tq_slider.generic.y		= 60;
	s_tq_slider.generic.name	= "texture quality";
	s_tq_slider.minvalue = 0;
	s_tq_slider.maxvalue = 3;
	s_tq_slider.curvalue = 3-gl_picmip->value;

	s_paletted_texture_box.generic.type = MTYPE_SPINCONTROL;
	s_paletted_texture_box.generic.x	= 0;
	s_paletted_texture_box.generic.y	= 70;
	s_paletted_texture_box.generic.name	= "8-bit textures";
	s_paletted_texture_box.itemnames = yesno_names;
	s_paletted_texture_box.curvalue = gl_ext_palettedtexture->value;

	Menu_AddItem( &s_menu, ( void * ) &s_mode_list );
	Menu_AddItem( &s_menu, ( void * ) &s_screensize_slider );
	Menu_AddItem( &s_menu, ( void * ) &s_brightness_slider );
	Menu_AddItem( &s_menu, ( void * ) &s_fs_box );
	Menu_AddItem( &s_menu, ( void * ) &s_tq_slider );
	Menu_AddItem( &s_menu, ( void * ) &s_paletted_texture_box );

	Menu_AddItem( &s_menu, ( void * ) &s_defaults_action );
	Menu_AddItem( &s_menu, ( void * ) &s_apply_action );

	Menu_Center( &s_menu );
	s_menu.x -= 8;
}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	int w, h;

	/*
	** draw the banner
	*/
	re.DrawGetPicSize( &w, &h, "m_banner_video" );
	re.DrawPic( viddef.width / 2 - w / 2, viddef.height /2 - 110, "m_banner_video" );

	/*
	** move cursor to a reasonable starting position
	*/
	Menu_AdjustCursor( &s_menu, 1 );

	/*
	** draw the menu
	*/
	Menu_Draw( &s_menu );
}

/*
================
VID_MenuKey
================
*/
const char *VID_MenuKey( int key )
{
	extern void M_PopMenu( void );

	menuframework_s *m = &s_menu;
	static const char *sound = "misc/menu1.wav";

	switch ( key )
	{
	case K_ESCAPE:
		M_PopMenu();
		return NULL;
	case K_UPARROW:
		m->cursor--;
		Menu_AdjustCursor( m, -1 );
		break;
	case K_DOWNARROW:
		m->cursor++;
		Menu_AdjustCursor( m, 1 );
		break;
	case K_LEFTARROW:
		Menu_SlideItem( m, -1 );
		break;
	case K_RIGHTARROW:
		Menu_SlideItem( m, 1 );
		break;
	case K_ENTER:
		Menu_SelectItem( m );
		break;
	}

	return sound;
}


