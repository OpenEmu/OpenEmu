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

//****************************************************************
// uCode 4 - RSP SW 2.0D EXT
//****************************************************************

static void uc4_vertex()
{
  int v0 = 0;     // Current vertex
  int n = ((rdp.cmd0 >> 4) & 0xFFF) / 33 + 1; // Number of vertices to copy
  rsp_vertex(v0, n);
}

static void uc4_tri1()
{
  int v1 = ((rdp.cmd1 >> 16) & 0xFF) / 5;
  int v2 = ((rdp.cmd1 >> 8) & 0xFF) / 5;
  int v3 = (rdp.cmd1 & 0xFF) / 5;
  FRDP("uc4:tri1 #%d - %d, %d, %d\n", rdp.tri_n,
    v1, v2, v3);

  VERTEX *v[3] = {
    &rdp.vtx[v1],
    &rdp.vtx[v2],
    &rdp.vtx[v3]
  };

  rsp_tri1(v);
}

static void uc4_quad3d()
{
  FRDP("uc4:quad3d #%d, #%d\n", rdp.tri_n, rdp.tri_n+1);

  VERTEX *v[6] = {
    &rdp.vtx[((rdp.cmd1 >> 24) & 0xFF) / 5],
    &rdp.vtx[((rdp.cmd1 >> 16) & 0xFF) / 5],
    &rdp.vtx[((rdp.cmd1 >> 8) & 0xFF) / 5],
    &rdp.vtx[((rdp.cmd1 >> 24) & 0xFF) / 5],
    &rdp.vtx[((rdp.cmd1 >> 8) & 0xFF) / 5],
    &rdp.vtx[(rdp.cmd1 & 0xFF) / 5]
  };

  rsp_tri2(v);
}
