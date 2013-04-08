/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2007, 2008 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TMS9918A_H
#define TMS9918A_H

#include "CrabEmu.h"
#include "smsvdp.h"

CLINKAGE

extern void tms9918a_m0_draw_bg(int line, sms_vdp_color_t *px);
extern void tms9918a_m1_draw_bg(int line, sms_vdp_color_t *px);
extern void tms9918a_m2_draw_bg(int line, sms_vdp_color_t *px);
extern void tms9918a_m3_draw_bg(int line, sms_vdp_color_t *px);
extern void tms9918a_m023_draw_spr(int line, sms_vdp_color_t *px);

ENDCLINK

#endif /* !TMS9918A_H */
