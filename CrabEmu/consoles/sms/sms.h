/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2007, 2009 Lawrence Sebald

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

#ifndef SMS_H
#define SMS_H

#include "CrabEmu.h"

CLINKAGE

extern int sms_init(int video_system, int region);
extern int sms_reset(void);
extern void sms_soft_reset(void);
extern int sms_shutdown(void);

extern int sms_frame(int run);

extern void gui_set_aspect(float x, float y);
extern void gui_set_title(const char *str);

extern void sms_button_pressed(int button);
extern void sms_button_released(int button);

extern void sms_set_console(int console);

extern int sms_save_state(const char *filename);
extern int sms_load_state(const char *filename);

/* Button defines */
#define SMS_PAD1_UP     0x0001
#define SMS_PAD1_DOWN   0x0002
#define SMS_PAD1_LEFT   0x0004
#define SMS_PAD1_RIGHT  0x0008
#define SMS_PAD1_A      0x0010
#define SMS_PAD1_B      0x0020
#define SMS_PAD2_UP     0x0040
#define SMS_PAD2_DOWN   0x0080
#define SMS_PAD2_LEFT   0x0100
#define SMS_PAD2_RIGHT  0x0200
#define SMS_PAD2_A      0x0400
#define SMS_PAD2_B      0x0800
#define SMS_RESET       0x1000
#define GG_START        0xFFFF

#define SMS_PAD1_TL     SMS_PAD1_A
#define SMS_PAD1_TR     SMS_PAD1_B
#define SMS_PAD2_TL     SMS_PAD2_A
#define SMS_PAD2_TR     SMS_PAD2_B
#define SMS_PAD1_TH     0x4000
#define SMS_PAD2_TH     0x8000

#define SMS_TH_MASK     0xC000

/* Console types */
#define CONSOLE_SMS    1
#define CONSOLE_GG     2
#define CONSOLE_SG1000 3
#define CONSOLE_SC3000 4

/* Region types */
#define SMS_REGION_DOMESTIC 0x01
#define SMS_REGION_EXPORT   0x02

/* Video Standards */
#define SMS_VIDEO_NTSC 0x10
#define SMS_VIDEO_PAL  0x20

#define SMS_CYCLES_PER_LINE 228

ENDCLINK

#endif /* !SMS_H */
