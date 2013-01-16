/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - tlb.h                                                   *
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

#ifndef TLB_H
#define TLB_H

typedef struct _tlb
{
   short mask;
   int vpn2;
   char g;
   unsigned char asid;
   int pfn_even;
   char c_even;
   char d_even;
   char v_even;
   int pfn_odd;
   char c_odd;
   char d_odd;
   char v_odd;
   char r;
   //int check_parity_mask;
   
   unsigned int start_even;
   unsigned int end_even;
   unsigned int phys_even;
   unsigned int start_odd;
   unsigned int end_odd;
   unsigned int phys_odd;
} tlb;

extern unsigned int tlb_LUT_r[0x100000];
extern unsigned int tlb_LUT_w[0x100000];
void tlb_unmap(tlb *entry);
void tlb_map(tlb *entry);
unsigned int virtual_to_physical_address(unsigned int addresse, int w);

#endif

