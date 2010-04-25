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

#include "api/m64p_types.h"

#include "r4300.h"
#include "macros.h"
#include "ops.h"
#include "recomph.h"
#include "interupt.h"

#include "api/callbacks.h"
#include "main/adler32.h"
#include "main/md5.h"
#include "memory/memory.h"

void TLBR(void)
{
   int index;
   index = Index & 0x1F;
   PageMask = tlb_e[index].mask << 13;
   EntryHi = ((tlb_e[index].vpn2 << 13) | tlb_e[index].asid);
   EntryLo0 = (tlb_e[index].pfn_even << 6) | (tlb_e[index].c_even << 3)
     | (tlb_e[index].d_even << 2) | (tlb_e[index].v_even << 1)
       | tlb_e[index].g;
   EntryLo1 = (tlb_e[index].pfn_odd << 6) | (tlb_e[index].c_odd << 3)
     | (tlb_e[index].d_odd << 2) | (tlb_e[index].v_odd << 1)
       | tlb_e[index].g;
   PC++;
}

void TLBWI(void)
{
   unsigned int i;

   if (tlb_e[Index&0x3F].v_even)
     {
    for (i=tlb_e[Index&0x3F].start_even>>12; i<=tlb_e[Index&0x3F].end_even>>12; i++)
      {
         if(!invalid_code[i] &&(invalid_code[tlb_LUT_r[i]>>12] ||
                    invalid_code[(tlb_LUT_r[i]>>12)+0x20000]))
           invalid_code[i] = 1;
         if (!invalid_code[i])
           {
          /*int j;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++) blocks[i]->md5[j] = digest[j];*/
          
          blocks[i]->adler32 = adler32(0, (const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4], 0x1000);
          
          invalid_code[i] = 1;
           }
         else if (blocks[i])
           {
          /*int j;
          for (j=0; j<16; j++) blocks[i]->md5[j] = 0;*/
          blocks[i]->adler32 = 0;
           }
         tlb_LUT_r[i] = 0;
      }
    if (tlb_e[Index&0x3F].d_even)
      for (i=tlb_e[Index&0x3F].start_even>>12; i<=tlb_e[Index&0x3F].end_even>>12; i++)
        tlb_LUT_w[i] = 0;
     }
   if (tlb_e[Index&0x3F].v_odd)
     {
    for (i=tlb_e[Index&0x3F].start_odd>>12; i<=tlb_e[Index&0x3F].end_odd>>12; i++)
      {
         if(!invalid_code[i] &&(invalid_code[tlb_LUT_r[i]>>12] ||
                    invalid_code[(tlb_LUT_r[i]>>12)+0x20000]))
           invalid_code[i] = 1;
         if (!invalid_code[i])
           {
          /*int j;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++) blocks[i]->md5[j] = digest[j];*/
          
          blocks[i]->adler32 = adler32(0, (const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4], 0x1000);
          
          invalid_code[i] = 1;
           }
         else if (blocks[i])
           {
          /*int j;
          for (j=0; j<16; j++) blocks[i]->md5[j] = 0;*/
          blocks[i]->adler32 = 0;
           }
         tlb_LUT_r[i] = 0;
      }
    if (tlb_e[Index&0x3F].d_odd)
      for (i=tlb_e[Index&0x3F].start_odd>>12; i<=tlb_e[Index&0x3F].end_odd>>12; i++)
        tlb_LUT_w[i] = 0;
     }
   tlb_e[Index&0x3F].g = (EntryLo0 & EntryLo1 & 1);
   tlb_e[Index&0x3F].pfn_even = (EntryLo0 & 0x3FFFFFC0) >> 6;
   tlb_e[Index&0x3F].pfn_odd = (EntryLo1 & 0x3FFFFFC0) >> 6;
   tlb_e[Index&0x3F].c_even = (EntryLo0 & 0x38) >> 3;
   tlb_e[Index&0x3F].c_odd = (EntryLo1 & 0x38) >> 3;
   tlb_e[Index&0x3F].d_even = (EntryLo0 & 0x4) >> 2;
   tlb_e[Index&0x3F].d_odd = (EntryLo1 & 0x4) >> 2;
   tlb_e[Index&0x3F].v_even = (EntryLo0 & 0x2) >> 1;
   tlb_e[Index&0x3F].v_odd = (EntryLo1 & 0x2) >> 1;
   tlb_e[Index&0x3F].asid = (EntryHi & 0xFF);
   tlb_e[Index&0x3F].vpn2 = (EntryHi & 0xFFFFE000) >> 13;
   //tlb_e[Index&0x3F].r = (EntryHi & 0xC000000000000000LL) >> 62;
   tlb_e[Index&0x3F].mask = (PageMask & 0x1FFE000) >> 13;
   
   tlb_e[Index&0x3F].start_even = tlb_e[Index&0x3F].vpn2 << 13;
   tlb_e[Index&0x3F].end_even = tlb_e[Index&0x3F].start_even+
     (tlb_e[Index&0x3F].mask << 12) + 0xFFF;
   tlb_e[Index&0x3F].phys_even = tlb_e[Index&0x3F].pfn_even << 12;
   
   if (tlb_e[Index&0x3F].v_even)
     {
    if (tlb_e[Index&0x3F].start_even < tlb_e[Index&0x3F].end_even &&
        !(tlb_e[Index&0x3F].start_even >= 0x80000000 &&
        tlb_e[Index&0x3F].end_even < 0xC0000000) &&
        tlb_e[Index&0x3F].phys_even < 0x20000000)
      {
         for (i=tlb_e[Index&0x3F].start_even;i<tlb_e[Index&0x3F].end_even;i++)
           tlb_LUT_r[i>>12] = 0x80000000 | 
           (tlb_e[Index&0x3F].phys_even + (i - tlb_e[Index&0x3F].start_even));
         if (tlb_e[Index&0x3F].d_even)
           for (i=tlb_e[Index&0x3F].start_even;i<tlb_e[Index&0x3F].end_even;i++)
         tlb_LUT_w[i>>12] = 0x80000000 | 
           (tlb_e[Index&0x3F].phys_even + (i - tlb_e[Index&0x3F].start_even));
      }
    
    for (i=tlb_e[Index&0x3F].start_even>>12; i<=tlb_e[Index&0x3F].end_even>>12; i++)
      {
         /*if (blocks[i] && (blocks[i]->md5[0] || blocks[i]->md5[1] ||
                   blocks[i]->md5[2] || blocks[i]->md5[3]))
           {
          int j;
          int equal = 1;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++)
            if (digest[j] != blocks[i]->md5[j])
              equal = 0;
          if (equal) invalid_code[i] = 0;
          }*/
         if(blocks[i] && blocks[i]->adler32)
           {
          if(blocks[i]->adler32 == adler32(0,(const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],0x1000))
            invalid_code[i] = 0;
           }
      }
     }
   tlb_e[Index&0x3F].start_odd = tlb_e[Index&0x3F].end_even+1;
   tlb_e[Index&0x3F].end_odd = tlb_e[Index&0x3F].start_odd+
     (tlb_e[Index&0x3F].mask << 12) + 0xFFF;
   tlb_e[Index&0x3F].phys_odd = tlb_e[Index&0x3F].pfn_odd << 12;
   
   if (tlb_e[Index&0x3F].v_odd)
     {
    if (tlb_e[Index&0x3F].start_odd < tlb_e[Index&0x3F].end_odd &&
        !(tlb_e[Index&0x3F].start_odd >= 0x80000000 &&
        tlb_e[Index&0x3F].end_odd < 0xC0000000) &&
        tlb_e[Index&0x3F].phys_odd < 0x20000000)
      {
         for (i=tlb_e[Index&0x3F].start_odd;i<tlb_e[Index&0x3F].end_odd;i++)
           tlb_LUT_r[i>>12] = 0x80000000 | 
           (tlb_e[Index&0x3F].phys_odd + (i - tlb_e[Index&0x3F].start_odd));
         if (tlb_e[Index&0x3F].d_odd)
           for (i=tlb_e[Index&0x3F].start_odd;i<tlb_e[Index&0x3F].end_odd;i++)
         tlb_LUT_w[i>>12] = 0x80000000 | 
           (tlb_e[Index&0x3F].phys_odd + (i - tlb_e[Index&0x3F].start_odd));
      }
    
    for (i=tlb_e[Index&0x3F].start_odd>>12; i<=tlb_e[Index&0x3F].end_odd>>12; i++)
      {
         /*if (blocks[i] && (blocks[i]->md5[0] || blocks[i]->md5[1] ||
                   blocks[i]->md5[2] || blocks[i]->md5[3]))
           {
          int j;
          int equal = 1;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++)
            if (digest[j] != blocks[i]->md5[j])
              equal = 0;
          if (equal) invalid_code[i] = 0;
           }*/
         if(blocks[i] && blocks[i]->adler32)
           {
          if(blocks[i]->adler32 == adler32(0,(const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],0x1000))
            invalid_code[i] = 0;
           }
      }
     }
   PC++;
}

void TLBWR(void)
{
   unsigned int i;
   update_count();
   Random = (Count/2 % (32 - Wired)) + Wired;

   if (tlb_e[Random].v_even)
     {
    for (i=tlb_e[Random].start_even>>12; i<=tlb_e[Random].end_even>>12; i++)
      {
         if(!invalid_code[i] &&(invalid_code[tlb_LUT_r[i]>>12] ||
                    invalid_code[(tlb_LUT_r[i]>>12)+0x20000]))
           invalid_code[i] = 1;
         if (!invalid_code[i])
           {
          /*int j;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++) blocks[i]->md5[j] = digest[j];*/
          
          blocks[i]->adler32 = adler32(0, (const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4], 0x1000);
          
          invalid_code[i] = 1;
           }
         else if (blocks[i])
           {
          /*int j;
          for (j=0; j<16; j++) blocks[i]->md5[j] = 0;*/
          blocks[i]->adler32 = 0;
           }
         tlb_LUT_r[i] = 0;
      }
    if (tlb_e[Random].d_even)
      for (i=tlb_e[Random].start_even>>12; i<=tlb_e[Random].end_even>>12; i++)
        tlb_LUT_w[i] = 0;
     }
   if (tlb_e[Random].v_odd)
     {
    for (i=tlb_e[Random].start_odd>>12; i<=tlb_e[Random].end_odd>>12; i++)
      {
         if(!invalid_code[i] &&(invalid_code[tlb_LUT_r[i]>>12] ||
                    invalid_code[(tlb_LUT_r[i]>>12)+0x20000]))
           invalid_code[i] = 1;
         if (!invalid_code[i])
           {
          /*int j;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++) blocks[i]->md5[j] = digest[j];*/
          
          blocks[i]->adler32 = adler32(0, (const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4], 0x1000);
          
          invalid_code[i] = 1;
           }
         else if (blocks[i])
           {
          /*int j;
          for (j=0; j<16; j++) blocks[i]->md5[j] = 0;*/
          blocks[i]->adler32 = 0;
           }
         tlb_LUT_r[i] = 0;
      }
    if (tlb_e[Random].d_odd)
      for (i=tlb_e[Random].start_odd>>12; i<=tlb_e[Random].end_odd>>12; i++)
        tlb_LUT_w[i] = 0;
     }
   tlb_e[Random].g = (EntryLo0 & EntryLo1 & 1);
   tlb_e[Random].pfn_even = (EntryLo0 & 0x3FFFFFC0) >> 6;
   tlb_e[Random].pfn_odd = (EntryLo1 & 0x3FFFFFC0) >> 6;
   tlb_e[Random].c_even = (EntryLo0 & 0x38) >> 3;
   tlb_e[Random].c_odd = (EntryLo1 & 0x38) >> 3;
   tlb_e[Random].d_even = (EntryLo0 & 0x4) >> 2;
   tlb_e[Random].d_odd = (EntryLo1 & 0x4) >> 2;
   tlb_e[Random].v_even = (EntryLo0 & 0x2) >> 1;
   tlb_e[Random].v_odd = (EntryLo1 & 0x2) >> 1;
   tlb_e[Random].asid = (EntryHi & 0xFF);
   tlb_e[Random].vpn2 = (EntryHi & 0xFFFFE000) >> 13;
   //tlb_e[Random].r = (EntryHi & 0xC000000000000000LL) >> 62;
   tlb_e[Random].mask = (PageMask & 0x1FFE000) >> 13;
   
   tlb_e[Random].start_even = tlb_e[Random].vpn2 << 13;
   tlb_e[Random].end_even = tlb_e[Random].start_even+
     (tlb_e[Random].mask << 12) + 0xFFF;
   tlb_e[Random].phys_even = tlb_e[Random].pfn_even << 12;
   
   if (tlb_e[Random].v_even)
     {
    if (tlb_e[Random].start_even < tlb_e[Random].end_even &&
        !(tlb_e[Random].start_even >= 0x80000000 &&
        tlb_e[Random].end_even < 0xC0000000) &&
        tlb_e[Random].phys_even < 0x20000000)
      {
         for (i=tlb_e[Random].start_even;i<tlb_e[Random].end_even;i+=0x1000)
           tlb_LUT_r[i>>12] = 0x80000000 | 
           (tlb_e[Random].phys_even + (i - tlb_e[Random].start_even) + 0xFFF);
         if (tlb_e[Random].d_even)
           for (i=tlb_e[Random].start_even;i<tlb_e[Random].end_even;i+=0x1000)
         tlb_LUT_w[i>>12] = 0x80000000 | 
           (tlb_e[Random].phys_even + (i - tlb_e[Random].start_even) + 0xFFF);
      }
    
    for (i=tlb_e[Random].start_even>>12; i<=tlb_e[Random].end_even>>12; i++)
      {
         /*if (blocks[i] && (blocks[i]->md5[0] || blocks[i]->md5[1] ||
                   blocks[i]->md5[2] || blocks[i]->md5[3]))
           {
          int j;
          int equal = 1;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++)
            if (digest[j] != blocks[i]->md5[j])
              equal = 0;
          if (equal) invalid_code[i] = 0;
           }*/
         if(blocks[i] && blocks[i]->adler32)
           {
          if(blocks[i]->adler32 == adler32(0,(const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],0x1000))
             invalid_code[i] = 0;
           }
      }
     }
   tlb_e[Random].start_odd = tlb_e[Random].end_even+1;
   tlb_e[Random].end_odd = tlb_e[Random].start_odd+
     (tlb_e[Random].mask << 12) + 0xFFF;
   tlb_e[Random].phys_odd = tlb_e[Random].pfn_odd << 12;
   
   if (tlb_e[Random].v_odd)
     {
    if (tlb_e[Random].start_odd < tlb_e[Random].end_odd &&
        !(tlb_e[Random].start_odd >= 0x80000000 &&
        tlb_e[Random].end_odd < 0xC0000000) &&
        tlb_e[Random].phys_odd < 0x20000000)
      {
         for (i=tlb_e[Random].start_odd;i<tlb_e[Random].end_odd;i+=0x1000)
           tlb_LUT_r[i>>12] = 0x80000000 | 
           (tlb_e[Random].phys_odd + (i - tlb_e[Random].start_odd) + 0xFFF);
         if (tlb_e[Random].d_odd)
           for (i=tlb_e[Random].start_odd;i<tlb_e[Random].end_odd;i+=0x1000)
         tlb_LUT_w[i>>12] = 0x80000000 | 
           (tlb_e[Random].phys_odd + (i - tlb_e[Random].start_odd) + 0xFFF);
      }
    
    for (i=tlb_e[Random].start_odd>>12; i<=tlb_e[Random].end_odd>>12; i++)
      {
         /*if (blocks[i] && (blocks[i]->md5[0] || blocks[i]->md5[1] ||
          blocks[i]->md5[2] || blocks[i]->md5[3]))
           {
          int j;
          int equal = 1;
          md5_state_t state;
          md5_byte_t digest[16];
          md5_init(&state);
          md5_append(&state, 
                 (const md5_byte_t*)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],
                 0x1000);
          md5_finish(&state, digest);
          for (j=0; j<16; j++)
            if (digest[j] != blocks[i]->md5[j])
              equal = 0;
          if (equal) invalid_code[i] = 0;
          }*/
         if(blocks[i] && blocks[i]->adler32)
           {
          if(blocks[i]->adler32 == adler32(0,(const unsigned char *)&rdram[(tlb_LUT_r[i]&0x7FF000)/4],0x1000))
            invalid_code[i] = 0;
           }
      }
     }
   PC++;
}

void TLBP(void)
{
   int i;
   Index |= 0x80000000;
   for (i=0; i<32; i++)
     {
    if (((tlb_e[i].vpn2 & (~tlb_e[i].mask)) ==
         (((EntryHi & 0xFFFFE000) >> 13) & (~tlb_e[i].mask))) &&
        ((tlb_e[i].g) ||
         (tlb_e[i].asid == (EntryHi & 0xFF))))
      {
         Index = i;
         break;
      }
     }
   PC++;
}

int jump_marker = 0;

void ERET(void)
{
   update_count();
   if (Status & 0x4)
   {
     DebugMessage(M64MSG_ERROR, "error in ERET");
     stop=1;
   }
   else
   {
     Status &= 0xFFFFFFFD;
     jump_to(EPC);
   }
   llbit = 0;
   check_interupt();
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

