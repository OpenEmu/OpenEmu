/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gcop1.c                                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2007 Richard Goedeken (Richard42)                       *
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
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[111]);
#endif
#ifdef INTERPRET_MFC1
   gencallinterp((unsigned long long)cached_interpreter_table.MFC1, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.r.nrd]));
   mov_reg32_preg64(EBX, RAX);
   mov_m32rel_xreg32((unsigned int*)dst->f.r.rt, EBX);
   sar_reg32_imm8(EBX, 31);
   mov_m32rel_xreg32(((unsigned int*)dst->f.r.rt)+1, EBX);
#endif
}

void gendmfc1(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[112]);
#endif
#ifdef INTERPRET_DMFC1
   gencallinterp((unsigned long long)cached_interpreter_table.DMFC1, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *) (&reg_cop1_double[dst->f.r.nrd]));
   mov_reg32_preg64(EBX, RAX);
   mov_reg32_preg64pimm32(ECX, RAX, 4);
   mov_m32rel_xreg32((unsigned int*)dst->f.r.rt, EBX);
   mov_m32rel_xreg32(((unsigned int*)dst->f.r.rt)+1, ECX);
#endif
}

void gencfc1(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[113]);
#endif
#ifdef INTERPRET_CFC1
   gencallinterp((unsigned long long)cached_interpreter_table.CFC1, 0);
#else
   gencheck_cop1_unusable();
   if(dst->f.r.nrd == 31) mov_xreg32_m32rel(EAX, (unsigned int*)&FCR31);
   else mov_xreg32_m32rel(EAX, (unsigned int*)&FCR0);
   mov_m32rel_xreg32((unsigned int*)dst->f.r.rt, EAX);
   sar_reg32_imm8(EAX, 31);
   mov_m32rel_xreg32(((unsigned int*)dst->f.r.rt)+1, EAX);
#endif
}

void genmtc1(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[114]);
#endif
#ifdef INTERPRET_MTC1
   gencallinterp((unsigned long long)cached_interpreter_table.MTC1, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg32_m32rel(EAX, (unsigned int*)dst->f.r.rt);
   mov_xreg64_m64rel(RBX, (unsigned long long *)(&reg_cop1_simple[dst->f.r.nrd]));
   mov_preg64_reg32(RBX, EAX);
#endif
}

void gendmtc1(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[115]);
#endif
#ifdef INTERPRET_DMTC1
   gencallinterp((unsigned long long)cached_interpreter_table.DMTC1, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg32_m32rel(EAX, (unsigned int*)dst->f.r.rt);
   mov_xreg32_m32rel(EBX, ((unsigned int*)dst->f.r.rt)+1);
   mov_xreg64_m64rel(RDX, (unsigned long long *)(&reg_cop1_double[dst->f.r.nrd]));
   mov_preg64_reg32(RDX, EAX);
   mov_preg64pimm32_reg32(RDX, 4, EBX);
#endif
}

void genctc1(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[116]);
#endif
#ifdef INTERPRET_CTC1
   gencallinterp((unsigned long long)cached_interpreter_table.CTC1, 0);
#else
   gencheck_cop1_unusable();
   
   if (dst->f.r.nrd != 31) return;
   mov_xreg32_m32rel(EAX, (unsigned int*)dst->f.r.rt);
   mov_m32rel_xreg32((unsigned int*)&FCR31, EAX);
   and_eax_imm32(3);
   
   cmp_eax_imm32(0);
   jne_rj(13);
   mov_m32rel_imm32((unsigned int*)&rounding_mode, 0x33F); // 11
   jmp_imm_short(51); // 2
   
   cmp_eax_imm32(1); // 5
   jne_rj(13); // 2
   mov_m32rel_imm32((unsigned int*)&rounding_mode, 0xF3F); // 11
   jmp_imm_short(31); // 2
   
   cmp_eax_imm32(2); // 5
   jne_rj(13); // 2
   mov_m32rel_imm32((unsigned int*)&rounding_mode, 0xB3F); // 11
   jmp_imm_short(11); // 2
   
   mov_m32rel_imm32((unsigned int*)&rounding_mode, 0x73F); // 11
   
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

