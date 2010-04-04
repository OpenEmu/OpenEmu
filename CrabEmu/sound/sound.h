/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2007, 2008 Lawrence Sebald

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

#ifndef SOUND_H
#define SOUND_H

#include "CrabEmu.h"

CLINKAGE

int sound_init(void);
void sound_shutdown(void);
#ifndef _arch_dreamcast
void sound_update_buffer(int16 *buf, int length);
#else
void sound_update_buffer_noint(int16 *bufl, int16 *bufr,
                               int length);
#endif
void sound_reset_buffer(void);
void sound_pause(void);
void sound_unpause(void);

ENDCLINK

#endif /* !SOUND_H */
