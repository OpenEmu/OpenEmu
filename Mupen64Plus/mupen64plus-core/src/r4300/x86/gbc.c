/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gbc.c                                                   *
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
#include "r4300/r4300.h"
#include "r4300/ops.h"

static void genbc1f_test(void)
{
   test_m32_imm32((unsigned int*)&FCR31, 0x800000);
   jne_rj(12);
   mov_m32_imm32((unsigned int*)(&branch_taken), 1); // 10
   jmp_imm_short(10); // 2
   mov_m32_imm32((unsigned int*)(&branch_taken), 0); // 10
}

void genbc1f(void)
{
#ifdef INTERPRET_BC1F
   gencallinterp((unsigned int)cached_interpreter_table.BC1F, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1F, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1f_test();
   gendelayslot();
   gentest();
#endif
}

void genbc1f_out(void)
{
#ifdef INTERPRET_BC1F_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BC1F_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1F_OUT, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1f_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbc1f_idle(void)
{
#ifdef INTERPRET_BC1F_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BC1F_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1F_IDLE, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1f_test();
   gentest_idle();
   genbc1f();
#endif
}

static void genbc1t_test(void)
{
   test_m32_imm32((unsigned int*)&FCR31, 0x800000);
   je_rj(12);
   mov_m32_imm32((unsigned int*)(&branch_taken), 1); // 10
   jmp_imm_short(10); // 2
   mov_m32_imm32((unsigned int*)(&branch_taken), 0); // 10
}

void genbc1t(void)
{
#ifdef INTERPRET_BC1T
   gencallinterp((unsigned int)cached_interpreter_table.BC1T, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1T, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1t_test();
   gendelayslot();
   gentest();
#endif
}

void genbc1t_out(void)
{
#ifdef INTERPRET_BC1T_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BC1T_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1T_OUT, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1t_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbc1t_idle(void)
{
#ifdef INTERPRET_BC1T_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BC1T_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1T_IDLE, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1t_test();
   gentest_idle();
   genbc1t();
#endif
}

void genbc1fl(void)
{
#ifdef INTERPRET_BC1FL
   gencallinterp((unsigned int)cached_interpreter_table.BC1FL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1FL, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1f_test();
   free_all_registers();
   gentestl();
#endif
}

void genbc1fl_out(void)
{
#ifdef INTERPRET_BC1FL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BC1FL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1FL_OUT, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1f_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbc1fl_idle(void)
{
#ifdef INTERPRET_BC1FL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BC1FL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1FL_IDLE, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1f_test();
   gentest_idle();
   genbc1fl();
#endif
}

void genbc1tl(void)
{
#ifdef INTERPRET_BC1TL
   gencallinterp((unsigned int)cached_interpreter_table.BC1TL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1TL, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1t_test();
   free_all_registers();
   gentestl();
#endif
}

void genbc1tl_out(void)
{
#ifdef INTERPRET_BC1TL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BC1TL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1TL_OUT, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1t_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbc1tl_idle(void)
{
#ifdef INTERPRET_BC1TL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BC1TL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC &&
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BC1TL_IDLE, 1);
    return;
     }
   
   gencheck_cop1_unusable();
   genbc1t_test();
   gentest_idle();
   genbc1tl();
#endif
}

