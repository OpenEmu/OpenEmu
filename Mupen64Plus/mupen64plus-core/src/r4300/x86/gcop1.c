/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gcop1.c                                                 *
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

#include <stdio.h>

#include "assemble.h"
#include "interpret.h"

#include "r4300/recomph.h"
#include "r4300/recomp.h"
#include "r4300/r4300.h"
#include "r4300/ops.h"
#include "r4300/macros.h"

#include "memory/memory.h"

void genmfc1(void)
{
#ifdef INTERPRET_MFC1
   gencallinterp((unsigned int)cached_interpreter_table.MFC1, 0);
#else
   gencheck_cop1_unusable();
   mov_eax_memoffs32((unsigned int*)(&reg_cop1_simple[dst->f.r.nrd]));
   mov_reg32_preg32(EBX, EAX);
   mov_m32_reg32((unsigned int*)dst->f.r.rt, EBX);
   sar_reg32_imm8(EBX, 31);
   mov_m32_reg32(((unsigned int*)dst->f.r.rt)+1, EBX);
#endif
}

void gendmfc1(void)
{
#ifdef INTERPRET_DMFC1
   gencallinterp((unsigned int)cached_interpreter_table.DMFC1, 0);
#else
   gencheck_cop1_unusable();
   mov_eax_memoffs32((unsigned int*)(&reg_cop1_double[dst->f.r.nrd]));
   mov_reg32_preg32(EBX, EAX);
   mov_reg32_preg32pimm32(ECX, EAX, 4);
   mov_m32_reg32((unsigned int*)dst->f.r.rt, EBX);
   mov_m32_reg32(((unsigned int*)dst->f.r.rt)+1, ECX);
#endif
}

void gencfc1(void)
{
#ifdef INTERPRET_CFC1
   gencallinterp((unsigned int)cached_interpreter_table.CFC1, 0);
#else
   gencheck_cop1_unusable();
   if(dst->f.r.nrd == 31) mov_eax_memoffs32((unsigned int*)&FCR31);
   else mov_eax_memoffs32((unsigned int*)&FCR0);
   mov_memoffs32_eax((unsigned int*)dst->f.r.rt);
   sar_reg32_imm8(EAX, 31);
   mov_memoffs32_eax(((unsigned int*)dst->f.r.rt)+1);
#endif
}

void genmtc1(void)
{
#ifdef INTERPRET_MTC1
   gencallinterp((unsigned int)cached_interpreter_table.MTC1, 0);
#else
   gencheck_cop1_unusable();
   mov_eax_memoffs32((unsigned int*)dst->f.r.rt);
   mov_reg32_m32(EBX, (unsigned int*)(&reg_cop1_simple[dst->f.r.nrd]));
   mov_preg32_reg32(EBX, EAX);
#endif
}

void gendmtc1(void)
{
#ifdef INTERPRET_DMTC1
   gencallinterp((unsigned int)cached_interpreter_table.DMTC1, 0);
#else
   gencheck_cop1_unusable();
   mov_eax_memoffs32((unsigned int*)dst->f.r.rt);
   mov_reg32_m32(EBX, ((unsigned int*)dst->f.r.rt)+1);
   mov_reg32_m32(EDX, (unsigned int*)(&reg_cop1_double[dst->f.r.nrd]));
   mov_preg32_reg32(EDX, EAX);
   mov_preg32pimm32_reg32(EDX, 4, EBX);
#endif
}

void genctc1(void)
{
#ifdef INTERPRET_CTC1
   gencallinterp((unsigned int)cached_interpreter_table.CTC1, 0);
#else
   gencheck_cop1_unusable();
   
   if (dst->f.r.nrd != 31) return;
   mov_eax_memoffs32((unsigned int*)dst->f.r.rt);
   mov_memoffs32_eax((unsigned int*)&FCR31);
   and_eax_imm32(3);
   
   cmp_eax_imm32(0);
   jne_rj(12);
   mov_m32_imm32((unsigned int*)&rounding_mode, 0x33F); // 10
   jmp_imm_short(48); // 2
   
   cmp_eax_imm32(1); // 5
   jne_rj(12); // 2
   mov_m32_imm32((unsigned int*)&rounding_mode, 0xF3F); // 10
   jmp_imm_short(29); // 2
   
   cmp_eax_imm32(2); // 5
   jne_rj(12); // 2
   mov_m32_imm32((unsigned int*)&rounding_mode, 0xB3F); // 10
   jmp_imm_short(10); // 2
   
   mov_m32_imm32((unsigned int*)&rounding_mode, 0x73F); // 10
   
   fldcw_m16((unsigned short*)&rounding_mode);
#endif
}

