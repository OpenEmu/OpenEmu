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

#ifndef TEREBI_H
#define TEREBI_H

#include "CrabEmu.h"

CLINKAGE

#define TEREBI_OEKAKI_PRESSED   1
#define TEREBI_OEKAKI_AXIS_Y    2

extern uint8 terebi_mread(uint16 addr);
extern void terebi_mwrite(uint16 addr, uint8 data);

extern uint16 terebi_mread16(uint16 addr);
extern void terebi_mwrite16(uint16 addr, uint16 data);

extern void terebi_update(int x, int y, int pressed);

ENDCLINK

#endif /* !TEREBI_H */
