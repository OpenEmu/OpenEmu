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

#include "TexLoad4b.h"
#include "TexLoad8b.h"
#include "TexLoad16b.h"
#include "TexLoad32b.h"

wxUint32 LoadNone (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
	memset (texture, 0, 4096*4);
	return (1 << 16) | GR_TEXFMT_ARGB_1555;
}

typedef wxUint32 (*texfunc)(wxUIntPtr, wxUIntPtr, int, int, int, int, int);
texfunc load_table [4][5] = {	// [size][format]
{	Load4bSelect,
	LoadNone,
	Load4bCI,
	Load4bIA,
	Load4bI  },

{	Load8bCI,
	LoadNone,
	Load8bCI,
	Load8bIA,
	Load8bI  },

{	Load16bRGBA,
	Load16bYUV,
	Load16bRGBA,
	Load16bIA,
	LoadNone },

{	Load32bRGBA,
	LoadNone,
	LoadNone,
	LoadNone,
	LoadNone }
};
