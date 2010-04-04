/*
    This file is part of CrabEmu.

    Copyright (C) 2008 Lawrence Sebald

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

#ifndef _93C46_H
#define _93C46_H

#include "CrabEmu.h"

CLINKAGE

#define EEPROM93c46_MODE_STANDBY 0
#define EEPROM93c46_MODE_START   1
#define EEPROM93c46_MODE_READ    2
#define EEPROM93c46_MODE_WRITE   3
#define EEPROM93c46_MODE_OPCODE  4
#define EEPROM93c46_MODE_DONE    5

#define EEPROM93c46_LINE_DATA_IN  1
#define EEPROM93c46_LINE_CLOCK    2
#define EEPROM93c46_LINE_CS       4
#define EEPROM93c46_LINE_DATA_OUT 8

typedef struct eeprom93c46_s    {
    uint16 data[64];
    int mode;
    int bit;
    uint16 data_in;
    uint8 opcode;

    int enabled;
    int readwrite;
    uint8 lines;
} eeprom93c46_t;

extern void eeprom93c46_init(void);
extern void eeprom93c46_reset(void);

extern void eeprom93c46_ctl_write(uint8 data);
extern void eeprom93c46_write(uint8 data);
extern uint8 eeprom93c46_read(void);

ENDCLINK

#endif /* !_93C46_H */
