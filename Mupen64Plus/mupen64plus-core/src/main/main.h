/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main.h                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Blight                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __MAIN_H__
#define __MAIN_H__

#include "api/m64p_types.h"

/* globals */
extern m64p_handle g_CoreConfig;

extern int g_MemHasBeenBSwapped;
extern int g_TakeScreenshot;
extern int g_EmulatorRunning;

extern m64p_frame_callback g_FrameCallback;

const char* get_savespath(void);

void new_frame(void);
void new_vi(void);

void main_set_core_defaults(void);
void main_message(m64p_msg_level level, unsigned int osd_corner, const char *format, ...);

m64p_error main_run(void);
void main_stop(void);
int  main_is_paused(void);
void main_toggle_pause(void);
void main_advance_one(void);

void main_speedup(int percent);
void main_speeddown(int percent);
void main_speedset(int percent);
void main_set_fastforward(int enable);
void main_set_speedlimiter(int enable);
int main_get_speedlimiter(void);

void main_draw_volume_osd(void);
void main_take_next_screenshot(void);

void main_state_set_slot(int slot);
void main_state_inc_slot(void);
void main_state_load(const char *filename);
void main_state_save(int format_pj64, const char *filename);

m64p_error main_core_state_query(m64p_core_param param, int *rval);

#endif /* __MAIN_H__ */

