/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - tlb.c                                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
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

#include <string.h>

#include "api/m64p_types.h"

#include "memory.h"

#include "r4300/r4300.h"
#include "r4300/exception.h"
#include "r4300/macros.h"
#include "main/rom.h"

unsigned int tlb_LUT_r[0x100000];
unsigned int tlb_LUT_w[0x100000];

void tlb_unmap(tlb *entry)
{
    unsigned int i;

    if (entry->v_even)
    {
        for (i=entry->start_even; i<entry->end_even; i += 0x1000)
            tlb_LUT_r[i>>12] = 0;
        if (entry->d_even)
            for (i=entry->start_even; i<entry->end_even; i += 0x1000)
                tlb_LUT_w[i>>12] = 0;
    }

    if (entry->v_odd)
    {
        for (i=entry->start_odd; i<entry->end_odd; i += 0x1000)
            tlb_LUT_r[i>>12] = 0;
        if (entry->d_odd)
            for (i=entry->start_odd; i<entry->end_odd; i += 0x1000)
                tlb_LUT_w[i>>12] = 0;
    }
}

void tlb_map(tlb *entry)
{
    unsigned int i;

    if (entry->v_even)
    {
        if (entry->start_even < entry->end_even &&
            !(entry->start_even >= 0x80000000 && entry->end_even < 0xC0000000) &&
            entry->phys_even < 0x20000000)
        {
            for (i=entry->start_even;i<entry->end_even;i+=0x1000)
                tlb_LUT_r[i>>12] = 0x80000000 | (entry->phys_even + (i - entry->start_even) + 0xFFF);
            if (entry->d_even)
                for (i=entry->start_even;i<entry->end_even;i+=0x1000)
                    tlb_LUT_w[i>>12] = 0x80000000 | (entry->phys_even + (i - entry->start_even) + 0xFFF);
        }
    }

    if (entry->v_odd)
    {
        if (entry->start_odd < entry->end_odd &&
            !(entry->start_odd >= 0x80000000 && entry->end_odd < 0xC0000000) &&
            entry->phys_odd < 0x20000000)
        {
            for (i=entry->start_odd;i<entry->end_odd;i+=0x1000)
                tlb_LUT_r[i>>12] = 0x80000000 | (entry->phys_odd + (i - entry->start_odd) + 0xFFF);
            if (entry->d_odd)
                for (i=entry->start_odd;i<entry->end_odd;i+=0x1000)
                    tlb_LUT_w[i>>12] = 0x80000000 | (entry->phys_odd + (i - entry->start_odd) + 0xFFF);
        }
    }
}

unsigned int virtual_to_physical_address(unsigned int addresse, int w)
{
    if (addresse >= 0x7f000000 && addresse < 0x80000000 && isGoldeneyeRom)
    {
        /**************************************************
         GoldenEye 007 hack allows for use of TLB.
         Recoded by okaygo to support all US, J, and E ROMS.
        **************************************************/
        switch (ROM_HEADER.Country_code & 0xFF)
        {
        case 0x45:
            // U
            return 0xb0034b30 + (addresse & 0xFFFFFF);
            break;
        case 0x4A:
            // J
            return 0xb0034b70 + (addresse & 0xFFFFFF);
            break;
        case 0x50:
            // E
            return 0xb00329f0 + (addresse & 0xFFFFFF);
            break;
        default:
            // UNKNOWN COUNTRY CODE FOR GOLDENEYE USING AMERICAN VERSION HACK
            return 0xb0034b30 + (addresse & 0xFFFFFF);
            break;
        }
    }
    if (w == 1)
    {
        if (tlb_LUT_w[addresse>>12])
            return (tlb_LUT_w[addresse>>12]&0xFFFFF000)|(addresse&0xFFF);
    }
    else
    {
        if (tlb_LUT_r[addresse>>12])
            return (tlb_LUT_r[addresse>>12]&0xFFFFF000)|(addresse&0xFFF);
    }
    //printf("tlb exception !!! @ %x, %x, add:%x\n", addresse, w, PC->addr);
    //getchar();
    TLB_refill_exception(addresse,w);
    //return 0x80000000;
    return 0x00000000;
}
