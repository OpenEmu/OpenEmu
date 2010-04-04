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

#ifndef MAPPERS_H
#define MAPPERS_H

#include "CrabEmu.h"

CLINKAGE

/* Support code for detecting roms that their mapper is not easily detected
   otherwise. */

extern int sms_find_mapper(const uint8 *rom, uint32 len);

ENDCLINK

#endif /* !MAPPERS_H */
