/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gregimm.c                                               *
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

static void genbltz_test(void)
{
   int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   
   if (!rs_64bit)
     {
    int rs = allocate_register((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs, 0);
    jge_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else if (rs_64bit == -1)
     {
    cmp_m32_imm32(((unsigned int *)dst->f.i.rs)+1, 0);
    jge_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else
     {
    int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs2, 0);
    jge_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
}

void genbltz(void)
{
#ifdef INTERPRET_BLTZ
   gencallinterp((unsigned int)cached_interpreter_table.BLTZ, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZ, 1);
    return;
     }
   
   genbltz_test();
   gendelayslot();
   gentest();
#endif
}

void genbltz_out(void)
{
#ifdef INTERPRET_BLTZ_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BLTZ_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZ_OUT, 1);
    return;
     }
   
   genbltz_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbltz_idle(void)
{
#ifdef INTERPRET_BLTZ_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BLTZ_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZ_IDLE, 1);
    return;
     }
   
   genbltz_test();
   gentest_idle();
   genbltz();
#endif
}

static void genbgez_test(void)
{
   int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   
   if (!rs_64bit)
     {
    int rs = allocate_register((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs, 0);
    jl_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else if (rs_64bit == -1)
     {
    cmp_m32_imm32(((unsigned int *)dst->f.i.rs)+1, 0);
    jl_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else
     {
    int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs2, 0);
    jl_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
}

void genbgez(void)
{
#ifdef INTERPRET_BGEZ
   gencallinterp((unsigned int)cached_interpreter_table.BGEZ, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZ, 1);
    return;
     }
   
   genbgez_test();
   gendelayslot();
   gentest();
#endif
}

void genbgez_out(void)
{
#ifdef INTERPRET_BGEZ_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BGEZ_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZ_OUT, 1);
    return;
     }
   
   genbgez_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbgez_idle(void)
{
#ifdef INTERPRET_BGEZ_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BGEZ_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZ_IDLE, 1);
    return;
     }
   
   genbgez_test();
   gentest_idle();
   genbgez();
#endif
}

void genbltzl(void)
{
#ifdef INTERPRET_BLTZL
   gencallinterp((unsigned int)cached_interpreter_table.BLTZL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZL, 1);
    return;
     }
   
   genbltz_test();
   free_all_registers();
   gentestl();
#endif
}

void genbltzl_out(void)
{
#ifdef INTERPRET_BLTZL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BLTZL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZL_OUT, 1);
    return;
     }
   
   genbltz_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbltzl_idle(void)
{
#ifdef INTERPRET_BLTZL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BLTZL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZL_IDLE, 1);
    return;
     }
   
   genbltz_test();
   gentest_idle();
   genbltzl();
#endif
}

void genbgezl(void)
{
#ifdef INTERPRET_BGEZL
   gencallinterp((unsigned int)cached_interpreter_table.BGEZL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZL, 1);
    return;
     }
   
   genbgez_test();
   free_all_registers();
   gentestl();
#endif
}

void genbgezl_out(void)
{
#ifdef INTERPRET_BGEZL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BGEZL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZL_OUT, 1);
    return;
     }
   
   genbgez_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbgezl_idle(void)
{
#ifdef INTERPRET_BGEZL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BGEZL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZL_IDLE, 1);
    return;
     }
   
   genbgez_test();
   gentest_idle();
   genbgezl();
#endif
}

static void genbranchlink(void)
{
   int r31_64bit = is64((unsigned int*)&reg[31]);
   
   if (!r31_64bit)
     {
    int r31 = allocate_register_w((unsigned int *)&reg[31]);
    
    mov_reg32_imm32(r31, dst->addr+8);
     }
   else if (r31_64bit == -1)
     {
    mov_m32_imm32((unsigned int *)&reg[31], dst->addr + 8);
    if (dst->addr & 0x80000000)
      mov_m32_imm32(((unsigned int *)&reg[31])+1, 0xFFFFFFFF);
    else
      mov_m32_imm32(((unsigned int *)&reg[31])+1, 0);
     }
   else
     {
    int r311 = allocate_64_register1_w((unsigned int *)&reg[31]);
    int r312 = allocate_64_register2_w((unsigned int *)&reg[31]);
    
    mov_reg32_imm32(r311, dst->addr+8);
    if (dst->addr & 0x80000000)
      mov_reg32_imm32(r312, 0xFFFFFFFF);
    else
      mov_reg32_imm32(r312, 0);
     }
}

void genbltzal(void)
{
#ifdef INTERPRET_BLTZAL
   gencallinterp((unsigned int)cached_interpreter_table.BLTZAL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZAL, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gendelayslot();
   gentest();
#endif
}

void genbltzal_out(void)
{
#ifdef INTERPRET_BLTZAL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BLTZAL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZAL_OUT, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gendelayslot();
   gentest_out();
#endif
}

void genbltzal_idle(void)
{
#ifdef INTERPRET_BLTZAL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BLTZAL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZAL_IDLE, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gentest_idle();
   genbltzal();
#endif
}

void genbgezal(void)
{
#ifdef INTERPRET_BGEZAL
   gencallinterp((unsigned int)cached_interpreter_table.BGEZAL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZAL, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gendelayslot();
   gentest();
#endif
}

void genbgezal_out(void)
{
#ifdef INTERPRET_BGEZAL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BGEZAL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZAL_OUT, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gendelayslot();
   gentest_out();
#endif
}

void genbgezal_idle(void)
{
#ifdef INTERPRET_BGEZAL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BGEZAL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZAL_IDLE, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gentest_idle();
   genbgezal();
#endif
}

void genbltzall(void)
{
#ifdef INTERPRET_BLTZALL
   gencallinterp((unsigned int)cached_interpreter_table.BLTZALL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZALL, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   free_all_registers();
   gentestl();
#endif
}

void genbltzall_out(void)
{
#ifdef INTERPRET_BLTZALL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BLTZALL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZALL_OUT, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   free_all_registers();
   gentestl_out();
#endif
}

void genbltzall_idle(void)
{
#ifdef INTERPRET_BLTZALL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BLTZALL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BLTZALL_IDLE, 1);
    return;
     }
   
   genbltz_test();
   genbranchlink();
   gentest_idle();
   genbltzall();
#endif
}

void genbgezall(void)
{
#ifdef INTERPRET_BGEZALL
   gencallinterp((unsigned int)cached_interpreter_table.BGEZALL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZALL, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   free_all_registers();
   gentestl();
#endif
}

void genbgezall_out(void)
{
#ifdef INTERPRET_BGEZALL_OUT
   gencallinterp((unsigned int)cached_interpreter_table.BGEZALL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZALL_OUT, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   free_all_registers();
   gentestl_out();
#endif
}

void genbgezall_idle(void)
{
#ifdef INTERPRET_BGEZALL_IDLE
   gencallinterp((unsigned int)cached_interpreter_table.BGEZALL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)cached_interpreter_table.BGEZALL_IDLE, 1);
    return;
     }
   
   genbgez_test();
   genbranchlink();
   gentest_idle();
   genbgezall();
#endif
}

