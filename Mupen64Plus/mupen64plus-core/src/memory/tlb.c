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
extern unsigned int interp_addr;
unsigned int virtual_to_physical_address(unsigned int addresse, int w)
{
   if (addresse >= 0x7f000000 && addresse < 0x80000000 && isGoldeneyeRom)
   {
       /**************************************************
        GoldenEye 007 hack allows for use of TLB.
        Recoded by okaygo to support all US, J, and E ROMS. 
       **************************************************/    
       switch (ROM_HEADER->Country_code&0xFF) 
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
   //printf("tlb exception !!! @ %x, %x, add:%x\n", addresse, w, interp_addr);
   //getchar();
   TLB_refill_exception(addresse,w);
   //return 0x80000000;
   return 0x00000000;
   /*int i;
   for (i=0; i<32; i++)
     {
    if ((tlb_e[i].vpn2 & ~(tlb_e[i].mask))
        == ((addresse >> 13) & ~(tlb_e[i].mask)))
      {
         if (tlb_e[i].g || (tlb_e[i].asid == (EntryHi & 0xFF)))
           {
          if (addresse & tlb_e[i].check_parity_mask)
            {
               if (tlb_e[i].v_odd)
             {
                if (tlb_e[i].d_odd && w)
                  {
                 TLB_mod_exception();
                 return 0;
                  }
                return ((addresse & ((tlb_e[i].mask << 12)|0xFFF))
                    | ((tlb_e[i].pfn_odd << 12) &
                       ~((tlb_e[i].mask << 12)|0xFFF)) 
                    | 0x80000000);
             }
               else
             {
                TLB_invalid_exception();
                return 0;
             }
            }
          else
            {
               if (tlb_e[i].v_even)
             {
                if (tlb_e[i].d_even && w)
                  {
                 TLB_mod_exception();
                 return 0;
                  }
                return ((addresse & ((tlb_e[i].mask << 12)|0xFFF))
                    | ((tlb_e[i].pfn_even << 12) &
                       ~((tlb_e[i].mask << 12)|0xFFF))
                    | 0x80000000);
             }
               else
             {
                TLB_invalid_exception();
                return 0;
             }
            }
           }
         else
           {
          TLB_refill_exception(addresse,w);
           }
      }
     }
   BadVAddr = addresse;
   TLB_refill_exception(addresse,w);
   //printf("TLB refill exception\n");
   return 0x80000000;*/
}

int probe_nop(unsigned int address)
{
    unsigned int a;
   if (address < 0x80000000 || address > 0xc0000000)
     {
    if (tlb_LUT_r[address>>12])
      a = (tlb_LUT_r[address>>12]&0xFFFFF000)|(address&0xFFF);
    else
      return 0;
     }
   else
     a = address;
   
   if (a >= 0xa4000000 && a < 0xa4001000)
     {
    if (!SP_DMEM[(a&0xFFF)/4]) return 1;
    else return 0;
     }
   else if (a >= 0x80000000 && a < 0x80800000)
     {
    if (!rdram[(a&0x7FFFFF)/4]) return 1;
    else return 0;
     }
   else return 0;
}

