/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gtlb.c                                                  *
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

#include "r4300/recomph.h"
#include "r4300/r4300.h"
#include "r4300/ops.h"

void gentlbwi(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[104]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.TLBWI, 0);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   mov_reg32_imm32(EAX, (unsigned int)(TLBWI));
   call_reg32(EAX);
   genupdate_system(0);*/
}

void gentlbp(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[105]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.TLBP, 0);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   mov_reg32_imm32(EAX, (unsigned int)(TLBP));
   call_reg32(EAX);
   genupdate_system(0);*/
}

void gentlbr(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[106]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.TLBR, 0);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   mov_reg32_imm32(EAX, (unsigned int)(TLBR));
   call_reg32(EAX);
   genupdate_system(0);*/
}

void generet(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[108]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.ERET, 1);
   /*dst->local_addr = code_length;
   mov_m32_imm32((void *)(&PC), (unsigned int)(dst));
   genupdate_system(0);
   mov_reg32_imm32(EAX, (unsigned int)(ERET));
   call_reg32(EAX);
   mov_reg32_imm32(EAX, (unsigned int)(jump_code));
   jmp_reg32(EAX);*/
}

void gentlbwr(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[107]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.TLBWR, 0);
}

