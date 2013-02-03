/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators
// Project started on December 29th, 2001
//
// Authors:
// Dave2001, original author, founded the project in 2001, left it in 2002
// Gugaman, joined the project in 2002, left it in 2002
// Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
// Hiroshi 'KoolSmoky' Morii, joined the project in 2007
//
//****************************************************************
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
//****************************************************************

#define SELECTED_NONE	0x00000000
#define SELECTED_TRI	0x00000001
#define SELECTED_TEX	0x00000002

typedef struct TEX_INFO_t
{
	wxUint32 cur_cache[2];	// Current cache #
	wxUint8 format;
	wxUint8 size;
	wxUint32 width, height;
	wxUint16 line, wid;
	wxUint8 palette;
	wxUint8 clamp_s, clamp_t;
	wxUint8 mirror_s, mirror_t;
	wxUint8 mask_s, mask_t;
	wxUint8 shift_s, shift_t;
	wxUint16 ul_s, ul_t, lr_s, lr_t;
	wxUint16 t_ul_s, t_ul_t, t_lr_s, t_lr_t;
	float scale_s, scale_t;
	int tmu;
} TEX_INFO;

typedef struct TRI_INFO_t
{
	wxUint32	nv;			// Number of vertices
	VERTEX	*v;			// Vertices (2d screen coords) of the triangle, used to outline
	wxUint32	cycle1, cycle2, cycle_mode;	// Combine mode at the time of rendering
	wxUint8	uncombined;	// which is uncombined: 0x01=color 0x02=alpha 0x03=both
	wxUint32	geom_mode;	// geometry mode flags
	wxUint32	othermode_h;	// setothermode_h flags
	wxUint32	othermode_l;	// setothermode_l flags
	wxUint32	tri_n;		// Triangle number
	wxUint32	flags;

	int		type;	// 0-normal, 1-texrect, 2-fillrect

	// texture info
	TEX_INFO t[2];

	// colors
	wxUint32 fog_color;
	wxUint32 fill_color;
	wxUint32 prim_color;
	wxUint32 blend_color;
	wxUint32 env_color;
	wxUint32 prim_lodmin, prim_lodfrac;

	TRI_INFO_t	*pNext;
} TRI_INFO;

typedef struct DEBUGGER_t
{
	int capture;	// Capture moment for debugging?

	wxUint32 selected;	// Selected object (see flags above)
	TRI_INFO *tri_sel;

	wxUint32 tex_scroll;	// texture scrolling
	wxUint32 tex_sel;

	// CAPTURE INFORMATION
	wxUint8 *screen;		// Screen capture
	TRI_INFO *tri_list;	// Triangle information list
	TRI_INFO *tri_last;	// Last in the list (first in)

	wxUint32 tmu;	// tmu #

	wxUint32 draw_mode;

	// Page number
	int page;

} GLIDE64_DEBUGGER;

#define PAGE_GENERAL	0
#define PAGE_TEX1		1
#define PAGE_TEX2		2
#define PAGE_COLORS		3
#define PAGE_FBL		4
#define PAGE_OTHERMODE_L	5
#define PAGE_OTHERMODE_H	6
#define PAGE_TEXELS		7
#define PAGE_COORDS		8
#define PAGE_TEX_INFO	9

#define TRI_TRIANGLE	0
#define TRI_TEXRECT		1
#define TRI_FILLRECT	2
#define TRI_BACKGROUND	3

extern GLIDE64_DEBUGGER _debugger;

void debug_init ();
void debug_capture ();
void debug_cacheviewer ();
void debug_mouse ();
void debug_keys ();
void output (float x, float y, int scale, const char *fmt, ...);
