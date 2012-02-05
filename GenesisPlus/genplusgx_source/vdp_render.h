/***************************************************************************************
 *  Genesis Plus
 *  Video Display Processor (Mode 4 & Mode 5 rendering)
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007-2011), additional code & fixes for the GCN/Wii port
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************************/

#ifndef _RENDER_H_
#define _RENDER_H_

/* Global variables */
extern uint8 object_count;

/* Function prototypes */
extern void render_init(void);
extern void render_reset(void);
extern void render_line(int line);
extern void blank_line(int line, int offset, int width);
extern void remap_line(int line);
extern void window_clip(unsigned int data, unsigned int sw);
extern void render_bg_m4(int line, int width);
extern void render_bg_m5(int line, int width);
extern void render_bg_m5_vs(int line, int width);
extern void render_bg_m5_im2(int line, int width);
extern void render_bg_m5_im2_vs(int line, int width);
extern void render_obj_m4(int max_width);
extern void render_obj_m5(int max_width);
extern void render_obj_m5_ste(int max_width);
extern void render_obj_m5_im2(int max_width);
extern void render_obj_m5_im2_ste(int max_width);
extern void parse_satb_m4(int line);
extern void parse_satb_m5(int line);
extern void update_bg_pattern_cache_m4(int index);
extern void update_bg_pattern_cache_m5(int index);
#ifdef NGC
extern void color_update(int index, unsigned int data);
#endif

/* Function pointers */
extern void (*render_bg)(int line, int width);
extern void (*render_obj)(int max_width);
extern void (*parse_satb)(int line);
extern void (*update_bg_pattern_cache)(int index);
#ifndef NGC
extern void (*color_update)(int index, unsigned int data);
#endif

#endif /* _RENDER_H_ */

