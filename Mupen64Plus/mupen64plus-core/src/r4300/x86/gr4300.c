/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gr4300.c                                                *
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

#include "assemble.h"
#include "interpret.h"
#include "regcache.h"

#include "api/debugger.h"

#include "r4300/r4300.h"
#include "r4300/macros.h"
#include "r4300/interupt.h"
#include "r4300/ops.h"
#include "r4300/recomph.h"
#include "r4300/exception.h"

#include "memory/memory.h"

extern unsigned int op;

static precomp_instr fake_instr;
static int eax, ebx, ecx, edx, esp, ebp, esi, edi;

int branch_taken;
int dynarec_stack_initialized = 0;

/* static functions */

static void genupdate_count(unsigned int addr)
{
#ifndef COMPARE_CORE
#ifndef DBG
   mov_reg32_imm32(EAX, addr);
   sub_reg32_m32(EAX, (unsigned int*)(&last_addr));
   shr_reg32_imm8(EAX, 1);
   add_m32_reg32((unsigned int*)(&Count), EAX);
#else
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst+1));
   mov_reg32_imm32(EAX, (unsigned int)update_count);
   call_reg32(EAX);
#endif
#else
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst+1));
   mov_reg32_imm32(EAX, (unsigned int)update_count);
   call_reg32(EAX);
#endif
}

static void gencheck_interupt(unsigned int instr_structure)
{
   mov_eax_memoffs32(&next_interupt);
   cmp_reg32_m32(EAX, &Count);
   ja_rj(17);
   mov_m32_imm32((unsigned int*)(&PC), instr_structure); // 10
   mov_reg32_imm32(EAX, (unsigned int)gen_interupt); // 5
   call_reg32(EAX); // 2
}

static void gencheck_interupt_out(unsigned int addr)
{
   mov_eax_memoffs32(&next_interupt);
   cmp_reg32_m32(EAX, &Count);
   ja_rj(27);
   mov_m32_imm32((unsigned int*)(&fake_instr.addr), addr);
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(&fake_instr));
   mov_reg32_imm32(EAX, (unsigned int)gen_interupt);
   call_reg32(EAX);
}

static void genbeq_test(void)
{
   int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   int rt_64bit = is64((unsigned int *)dst->f.i.rt);
   
   if (!rs_64bit && !rt_64bit)
     {
    int rs = allocate_register((unsigned int *)dst->f.i.rs);
    int rt = allocate_register((unsigned int *)dst->f.i.rt);
    
    cmp_reg32_reg32(rs, rt);
    jne_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else if (rs_64bit == -1)
     {
    int rt1 = allocate_64_register1((unsigned int *)dst->f.i.rt);
    int rt2 = allocate_64_register2((unsigned int *)dst->f.i.rt);
    
    cmp_reg32_m32(rt1, (unsigned int *)dst->f.i.rs);
    jne_rj(20);
    cmp_reg32_m32(rt2, ((unsigned int *)dst->f.i.rs)+1); // 6
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else if (rt_64bit == -1)
     {
    int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
    int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_m32(rs1, (unsigned int *)dst->f.i.rt);
    jne_rj(20);
    cmp_reg32_m32(rs2, ((unsigned int *)dst->f.i.rt)+1); // 6
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else
     {
    int rs1, rs2, rt1, rt2;
    if (!rs_64bit)
      {
         rt1 = allocate_64_register1((unsigned int *)dst->f.i.rt);
         rt2 = allocate_64_register2((unsigned int *)dst->f.i.rt);
         rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
         rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
      }
    else
      {
         rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
         rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
         rt1 = allocate_64_register1((unsigned int *)dst->f.i.rt);
         rt2 = allocate_64_register2((unsigned int *)dst->f.i.rt);
      }
    cmp_reg32_reg32(rs1, rt1);
    jne_rj(16);
    cmp_reg32_reg32(rs2, rt2); // 2
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
}

static void genbne_test(void)
{
   int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   int rt_64bit = is64((unsigned int *)dst->f.i.rt);
   
   if (!rs_64bit && !rt_64bit)
     {
    int rs = allocate_register((unsigned int *)dst->f.i.rs);
    int rt = allocate_register((unsigned int *)dst->f.i.rt);
    
    cmp_reg32_reg32(rs, rt);
    je_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else if (rs_64bit == -1)
     {
    int rt1 = allocate_64_register1((unsigned int *)dst->f.i.rt);
    int rt2 = allocate_64_register2((unsigned int *)dst->f.i.rt);
    
    cmp_reg32_m32(rt1, (unsigned int *)dst->f.i.rs);
    jne_rj(20);
    cmp_reg32_m32(rt2, ((unsigned int *)dst->f.i.rs)+1); // 6
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
     }
   else if (rt_64bit == -1)
     {
    int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
    int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_m32(rs1, (unsigned int *)dst->f.i.rt);
    jne_rj(20);
    cmp_reg32_m32(rs2, ((unsigned int *)dst->f.i.rt)+1); // 6
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
     }
   else
     {
    int rs1, rs2, rt1, rt2;
    if (!rs_64bit)
      {
         rt1 = allocate_64_register1((unsigned int *)dst->f.i.rt);
         rt2 = allocate_64_register2((unsigned int *)dst->f.i.rt);
         rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
         rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
      }
    else
      {
         rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
         rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
         rt1 = allocate_64_register1((unsigned int *)dst->f.i.rt);
         rt2 = allocate_64_register2((unsigned int *)dst->f.i.rt);
      }
    cmp_reg32_reg32(rs1, rt1);
    jne_rj(16);
    cmp_reg32_reg32(rs2, rt2); // 2
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
     }
}

static void genblez_test(void)
{
   int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   
   if (!rs_64bit)
     {
    int rs = allocate_register((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs, 0);
    jg_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else if (rs_64bit == -1)
     {
    cmp_m32_imm32(((unsigned int *)dst->f.i.rs)+1, 0);
    jg_rj(14);
    jne_rj(24); // 2
    cmp_m32_imm32((unsigned int *)dst->f.i.rs, 0); // 10
    je_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
     }
   else
     {
    int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
    int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs2, 0);
    jg_rj(10);
    jne_rj(20); // 2
    cmp_reg32_imm32(rs1, 0); // 6
    je_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
     }
}

static void genbgtz_test(void)
{
   int rs_64bit = is64((unsigned int *)dst->f.i.rs);
   
   if (!rs_64bit)
     {
    int rs = allocate_register((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs, 0);
    jle_rj(12);
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
     }
   else if (rs_64bit == -1)
     {
    cmp_m32_imm32(((unsigned int *)dst->f.i.rs)+1, 0);
    jl_rj(14);
    jne_rj(24); // 2
    cmp_m32_imm32((unsigned int *)dst->f.i.rs, 0); // 10
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
     }
   else
     {
    int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
    int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
    
    cmp_reg32_imm32(rs2, 0);
    jl_rj(10);
    jne_rj(20); // 2
    cmp_reg32_imm32(rs1, 0); // 6
    jne_rj(12); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 0); // 10
    jmp_imm_short(10); // 2
    mov_m32_imm32((unsigned int *)(&branch_taken), 1); // 10
     }
}


/* global functions */

void gennotcompiled(void)
{
    free_all_registers();
    simplify_access();
   
    if (dst->addr == 0xa4000040 && dynarec_stack_initialized == 0)
    {
        dynarec_stack_initialized = 1;
        sub_reg32_imm32(ESP, 0x10); /* save 16 bytes of padding just in case */
        and_reg32_imm32(ESP, 0xfffffff0); /* align stack on 16-byte boundary for OSX */
        mov_m32_reg32((unsigned int*)(&return_address), ESP);
        sub_m32_imm32((unsigned int*)(&return_address), 4);
    }
    mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst));
    mov_reg32_imm32(EAX, (unsigned int)NOTCOMPILED);
    call_reg32(EAX);
}

void genlink_subblock(void)
{
   free_all_registers();
   jmp(dst->addr+4);
}

void gendebug(void)
{
   free_all_registers();
   mov_m32_reg32((unsigned int*)&eax, EAX);
   mov_m32_reg32((unsigned int*)&ebx, EBX);
   mov_m32_reg32((unsigned int*)&ecx, ECX);
   mov_m32_reg32((unsigned int*)&edx, EDX);
   mov_m32_reg32((unsigned int*)&esp, ESP);
   mov_m32_reg32((unsigned int*)&ebp, EBP);
   mov_m32_reg32((unsigned int*)&esi, ESI);
   mov_m32_reg32((unsigned int*)&edi, EDI);
   
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst));
   mov_m32_imm32((unsigned int*)(&op), (unsigned int)(src));
   mov_reg32_imm32(EAX, (unsigned int) CoreCompareCallback);
   call_reg32(EAX);
   
   mov_reg32_m32(EAX, (unsigned int*)&eax);
   mov_reg32_m32(EBX, (unsigned int*)&ebx);
   mov_reg32_m32(ECX, (unsigned int*)&ecx);
   mov_reg32_m32(EDX, (unsigned int*)&edx);
   mov_reg32_m32(ESP, (unsigned int*)&esp);
   mov_reg32_m32(EBP, (unsigned int*)&ebp);
   mov_reg32_m32(ESI, (unsigned int*)&esi);
   mov_reg32_m32(EDI, (unsigned int*)&edi);
}

void gencallinterp(unsigned long addr, int jump)
{
   free_all_registers();
   simplify_access();
   if (jump)
     mov_m32_imm32((unsigned int*)(&dyna_interp), 1);
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst));
   mov_reg32_imm32(EAX, addr);
   call_reg32(EAX);
   if (jump)
     {
    mov_m32_imm32((unsigned int*)(&dyna_interp), 0);
    mov_reg32_imm32(EAX, (unsigned int)dyna_jump);
    call_reg32(EAX);
     }
}

void gendelayslot(void)
{
   mov_m32_imm32(&delay_slot, 1);
   recompile_opcode();
   
   free_all_registers();
   genupdate_count(dst->addr+4);
   
   mov_m32_imm32(&delay_slot, 0);
}

void genni(void)
{
   gencallinterp((unsigned int)NI, 0);
}

void genreserved(void)
{
   gencallinterp((unsigned int)RESERVED, 0);
}

void genfin_block(void)
{
   gencallinterp((unsigned int)FIN_BLOCK, 0);
}

void gencheck_interupt_reg(void) // addr is in EAX
{
   mov_reg32_m32(EBX, &next_interupt);
   cmp_reg32_m32(EBX, &Count);
   ja_rj(22);
   mov_memoffs32_eax((unsigned int*)(&fake_instr.addr)); // 5
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(&fake_instr)); // 10
   mov_reg32_imm32(EAX, (unsigned int)gen_interupt); // 5
   call_reg32(EAX); // 2
}

void gennop(void)
{
}

void genj(void)
{
#ifdef INTERPRET_J
   gencallinterp((unsigned int)J, 1);
#else
   unsigned int naddr;
   
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)J, 1);
    return;
     }
   
   gendelayslot();
   naddr = ((dst-1)->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   
   mov_m32_imm32(&last_addr, naddr);
   gencheck_interupt((unsigned int)&actual->block[(naddr-actual->start)/4]);
   jmp(naddr);
#endif
}

void genj_out(void)
{
#ifdef INTERPRET_J_OUT
   gencallinterp((unsigned int)J_OUT, 1);
#else
   unsigned int naddr;
   
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)J_OUT, 1);
    return;
     }
   
   gendelayslot();
   naddr = ((dst-1)->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   
   mov_m32_imm32(&last_addr, naddr);
   gencheck_interupt_out(naddr);
   mov_m32_imm32(&jump_to_address, naddr);
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst+1));
   mov_reg32_imm32(EAX, (unsigned int)jump_to_func);
   call_reg32(EAX);
#endif
}

void genj_idle(void)
{
#ifdef INTERPRET_J_IDLE
   gencallinterp((unsigned int)J_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)J_IDLE, 1);
    return;
     }
   
   mov_eax_memoffs32((unsigned int *)(&next_interupt));
   sub_reg32_m32(EAX, (unsigned int *)(&Count));
   cmp_reg32_imm8(EAX, 3);
   jbe_rj(11);
   
   and_eax_imm32(0xFFFFFFFC);  // 5
   add_m32_reg32((unsigned int *)(&Count), EAX); // 6
  
   genj();
#endif
}

void genjal(void)
{
#ifdef INTERPRET_JAL
   gencallinterp((unsigned int)JAL, 1);
#else
   unsigned int naddr;
   
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)JAL, 1);
    return;
     }
   
   gendelayslot();
   
   mov_m32_imm32((unsigned int *)(reg + 31), dst->addr + 4);
   if (((dst->addr + 4) & 0x80000000))
     mov_m32_imm32((unsigned int *)(&reg[31])+1, 0xFFFFFFFF);
   else
     mov_m32_imm32((unsigned int *)(&reg[31])+1, 0);
   
   naddr = ((dst-1)->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   
   mov_m32_imm32(&last_addr, naddr);
   gencheck_interupt((unsigned int)&actual->block[(naddr-actual->start)/4]);
   jmp(naddr);
#endif
}

void genjal_out(void)
{
#ifdef INTERPRET_JAL_OUT
   gencallinterp((unsigned int)JAL_OUT, 1);
#else
   unsigned int naddr;
   
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)JAL_OUT, 1);
    return;
     }
   
   gendelayslot();
   
   mov_m32_imm32((unsigned int *)(reg + 31), dst->addr + 4);
   if (((dst->addr + 4) & 0x80000000))
     mov_m32_imm32((unsigned int *)(&reg[31])+1, 0xFFFFFFFF);
   else
     mov_m32_imm32((unsigned int *)(&reg[31])+1, 0);
   
   naddr = ((dst-1)->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   
   mov_m32_imm32(&last_addr, naddr);
   gencheck_interupt_out(naddr);
   mov_m32_imm32(&jump_to_address, naddr);
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst+1));
   mov_reg32_imm32(EAX, (unsigned int)jump_to_func);
   call_reg32(EAX);
#endif
}

void genjal_idle(void)
{
#ifdef INTERPRET_JAL_IDLE
   gencallinterp((unsigned int)JAL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)JAL_IDLE, 1);
    return;
     }
   
   mov_eax_memoffs32((unsigned int *)(&next_interupt));
   sub_reg32_m32(EAX, (unsigned int *)(&Count));
   cmp_reg32_imm8(EAX, 3);
   jbe_rj(11);
   
   and_eax_imm32(0xFFFFFFFC);
   add_m32_reg32((unsigned int *)(&Count), EAX);
  
   genjal();
#endif
}

void gentest(void)
{
   cmp_m32_imm32((unsigned int *)(&branch_taken), 0);
   je_near_rj(0);

   jump_start_rel32();

   mov_m32_imm32(&last_addr, dst->addr + (dst-1)->f.i.immediate*4);
   gencheck_interupt((unsigned int)(dst + (dst-1)->f.i.immediate));
   jmp(dst->addr + (dst-1)->f.i.immediate*4);

   jump_end_rel32();

   mov_m32_imm32(&last_addr, dst->addr + 4);
   gencheck_interupt((unsigned int)(dst + 1));
   jmp(dst->addr + 4);
}

void genbeq(void)
{
#ifdef INTERPRET_BEQ
   gencallinterp((unsigned int)BEQ, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BEQ, 1);
    return;
     }
   
   genbeq_test();
   gendelayslot();
   gentest();
#endif
}

void gentest_out(void)
{
   cmp_m32_imm32((unsigned int *)(&branch_taken), 0);
   je_near_rj(0);

   jump_start_rel32();

   mov_m32_imm32(&last_addr, dst->addr + (dst-1)->f.i.immediate*4);
   gencheck_interupt_out(dst->addr + (dst-1)->f.i.immediate*4);
   mov_m32_imm32(&jump_to_address, dst->addr + (dst-1)->f.i.immediate*4);
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst+1));
   mov_reg32_imm32(EAX, (unsigned int)jump_to_func);
   call_reg32(EAX);
   
   jump_end_rel32();

   mov_m32_imm32(&last_addr, dst->addr + 4);
   gencheck_interupt((unsigned int)(dst + 1));
   jmp(dst->addr + 4);
}

void genbeq_out(void)
{
#ifdef INTERPRET_BEQ_OUT
   gencallinterp((unsigned int)BEQ_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BEQ_OUT, 1);
    return;
     }
   
   genbeq_test();
   gendelayslot();
   gentest_out();
#endif
}

void gentest_idle(void)
{
   int reg;
   
   reg = lru_register();
   free_register(reg);
   
   cmp_m32_imm32((unsigned int *)(&branch_taken), 0);
   je_near_rj(0);

   jump_start_rel32();
   
   mov_reg32_m32(reg, (unsigned int *)(&next_interupt));
   sub_reg32_m32(reg, (unsigned int *)(&Count));
   cmp_reg32_imm8(reg, 3);
   jbe_rj(12);
   
   and_reg32_imm32(reg, 0xFFFFFFFC); // 6
   add_m32_reg32((unsigned int *)(&Count), reg); // 6
   
   jump_end_rel32();
}

void genbeq_idle(void)
{
#ifdef INTERPRET_BEQ_IDLE
   gencallinterp((unsigned int)BEQ_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BEQ_IDLE, 1);
    return;
     }
   
   genbeq_test();
   gentest_idle();
   genbeq();
#endif
}

void genbne(void)
{
#ifdef INTERPRET_BNE
   gencallinterp((unsigned int)BNE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BNE, 1);
    return;
     }
   
   genbne_test();
   gendelayslot();
   gentest();
#endif
}

void genbne_out(void)
{
#ifdef INTERPRET_BNE_OUT
   gencallinterp((unsigned int)BNE_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BNE_OUT, 1);
    return;
     }
   
   genbne_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbne_idle(void)
{
#ifdef INTERPRET_BNE_IDLE
   gencallinterp((unsigned int)BNE_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BNE_IDLE, 1);
    return;
     }
   
   genbne_test();
   gentest_idle();
   genbne();
#endif
}

void genblez(void)
{
#ifdef INTERPRET_BLEZ
   gencallinterp((unsigned int)BLEZ, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BLEZ, 1);
    return;
     }
   
   genblez_test();
   gendelayslot();
   gentest();
#endif
}

void genblez_out(void)
{
#ifdef INTERPRET_BLEZ_OUT
   gencallinterp((unsigned int)BLEZ_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BLEZ_OUT, 1);
    return;
     }
   
   genblez_test();
   gendelayslot();
   gentest_out();
#endif
}

void genblez_idle(void)
{
#ifdef INTERPRET_BLEZ_IDLE
   gencallinterp((unsigned int)BLEZ_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BLEZ_IDLE, 1);
    return;
     }
   
   genblez_test();
   gentest_idle();
   genblez();
#endif
}

void genbgtz(void)
{
#ifdef INTERPRET_BGTZ
   gencallinterp((unsigned int)BGTZ, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BGTZ, 1);
    return;
     }
   
   genbgtz_test();
   gendelayslot();
   gentest();
#endif
}

void genbgtz_out(void)
{
#ifdef INTERPRET_BGTZ_OUT
   gencallinterp((unsigned int)BGTZ_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BGTZ_OUT, 1);
    return;
     }
   
   genbgtz_test();
   gendelayslot();
   gentest_out();
#endif
}

void genbgtz_idle(void)
{
#ifdef INTERPRET_BGTZ_IDLE
   gencallinterp((unsigned int)BGTZ_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BGTZ_IDLE, 1);
    return;
     }
   
   genbgtz_test();
   gentest_idle();
   genbgtz();
#endif
}

void genaddi(void)
{
#ifdef INTERPRET_ADDI
   gencallinterp((unsigned int)ADDI, 0);
#else
   int rs = allocate_register((unsigned int *)dst->f.i.rs);
   int rt = allocate_register_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_reg32(rt, rs);
   add_reg32_imm32(rt,(int)dst->f.i.immediate);
#endif
}

void genaddiu(void)
{
#ifdef INTERPRET_ADDIU
   gencallinterp((unsigned int)ADDIU, 0);
#else
   int rs = allocate_register((unsigned int *)dst->f.i.rs);
   int rt = allocate_register_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_reg32(rt, rs);
   add_reg32_imm32(rt,(int)dst->f.i.immediate);
#endif
}

void genslti(void)
{
#ifdef INTERPRET_SLTI
   gencallinterp((unsigned int)SLTI, 0);
#else
   int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
   int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
   int rt = allocate_register_w((unsigned int *)dst->f.i.rt);
   long long imm = (long long)dst->f.i.immediate;
   
   cmp_reg32_imm32(rs2, (unsigned int)(imm >> 32));
   jl_rj(17);
   jne_rj(8); // 2
   cmp_reg32_imm32(rs1, (unsigned int)imm); // 6
   jl_rj(7); // 2
   mov_reg32_imm32(rt, 0); // 5
   jmp_imm_short(5); // 2
   mov_reg32_imm32(rt, 1); // 5
#endif
}

void gensltiu(void)
{
#ifdef INTERPRET_SLTIU
   gencallinterp((unsigned int)SLTIU, 0);
#else
   int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
   int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
   int rt = allocate_register_w((unsigned int *)dst->f.i.rt);
   long long imm = (long long)dst->f.i.immediate;
   
   cmp_reg32_imm32(rs2, (unsigned int)(imm >> 32));
   jb_rj(17);
   jne_rj(8); // 2
   cmp_reg32_imm32(rs1, (unsigned int)imm); // 6
   jb_rj(7); // 2
   mov_reg32_imm32(rt, 0); // 5
   jmp_imm_short(5); // 2
   mov_reg32_imm32(rt, 1); // 5
#endif
}

void genandi(void)
{
#ifdef INTERPRET_ANDI
   gencallinterp((unsigned int)ANDI, 0);
#else
   int rs = allocate_register((unsigned int *)dst->f.i.rs);
   int rt = allocate_register_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_reg32(rt, rs);
   and_reg32_imm32(rt, (unsigned short)dst->f.i.immediate);
#endif
}

void genori(void)
{
#ifdef INTERPRET_ORI
   gencallinterp((unsigned int)ORI, 0);
#else
   int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
   int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
   int rt1 = allocate_64_register1_w((unsigned int *)dst->f.i.rt);
   int rt2 = allocate_64_register2_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_reg32(rt1, rs1);
   mov_reg32_reg32(rt2, rs2);
   or_reg32_imm32(rt1, (unsigned short)dst->f.i.immediate);
#endif
}

void genxori(void)
{
#ifdef INTERPRET_XORI
   gencallinterp((unsigned int)XORI, 0);
#else
   int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
   int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
   int rt1 = allocate_64_register1_w((unsigned int *)dst->f.i.rt);
   int rt2 = allocate_64_register2_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_reg32(rt1, rs1);
   mov_reg32_reg32(rt2, rs2);
   xor_reg32_imm32(rt1, (unsigned short)dst->f.i.immediate);
#endif
}

void genlui(void)
{
#ifdef INTERPRET_LUI
   gencallinterp((unsigned int)LUI, 0);
#else
   int rt = allocate_register_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_imm32(rt, (unsigned int)dst->f.i.immediate << 16);
#endif
}

void gentestl(void)
{
   cmp_m32_imm32((unsigned int *)(&branch_taken), 0);
   je_near_rj(0);

   jump_start_rel32();

   gendelayslot();
   mov_m32_imm32(&last_addr, dst->addr + (dst-1)->f.i.immediate*4);
   gencheck_interupt((unsigned int)(dst + (dst-1)->f.i.immediate));
   jmp(dst->addr + (dst-1)->f.i.immediate*4);
   
   jump_end_rel32();

   genupdate_count(dst->addr-4);
   mov_m32_imm32(&last_addr, dst->addr + 4);
   gencheck_interupt((unsigned int)(dst + 1));
   jmp(dst->addr + 4);
}

void genbeql(void)
{
#ifdef INTERPRET_BEQL
   gencallinterp((unsigned int)BEQL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BEQL, 1);
    return;
     }
   
   genbeq_test();
   free_all_registers();
   gentestl();
#endif
}

void gentestl_out(void)
{
   cmp_m32_imm32((unsigned int *)(&branch_taken), 0);
   je_near_rj(0);

   jump_start_rel32();

   gendelayslot();
   mov_m32_imm32(&last_addr, dst->addr + (dst-1)->f.i.immediate*4);
   gencheck_interupt_out(dst->addr + (dst-1)->f.i.immediate*4);
   mov_m32_imm32(&jump_to_address, dst->addr + (dst-1)->f.i.immediate*4);
   mov_m32_imm32((unsigned int*)(&PC), (unsigned int)(dst+1));
   mov_reg32_imm32(EAX, (unsigned int)jump_to_func);
   call_reg32(EAX);
   
   jump_end_rel32();

   genupdate_count(dst->addr-4);
   mov_m32_imm32(&last_addr, dst->addr + 4);
   gencheck_interupt((unsigned int)(dst + 1));
   jmp(dst->addr + 4);
}

void genbeql_out(void)
{
#ifdef INTERPRET_BEQL_OUT
   gencallinterp((unsigned int)BEQL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BEQL_OUT, 1);
    return;
     }
   
   genbeq_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbeql_idle(void)
{
#ifdef INTERPRET_BEQL_IDLE
   gencallinterp((unsigned int)BEQL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BEQL_IDLE, 1);
    return;
     }
   
   genbeq_test();
   gentest_idle();
   genbeql();
#endif
}

void genbnel(void)
{
#ifdef INTERPRET_BNEL
   gencallinterp((unsigned int)BNEL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BNEL, 1);
    return;
     }
   
   genbne_test();
   free_all_registers();
   gentestl();
#endif
}

void genbnel_out(void)
{
#ifdef INTERPRET_BNEL_OUT
   gencallinterp((unsigned int)BNEL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BNEL_OUT, 1);
    return;
     }
   
   genbne_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbnel_idle(void)
{
#ifdef INTERPRET_BNEL_IDLE
   gencallinterp((unsigned int)BNEL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BNEL_IDLE, 1);
    return;
     }
   
   genbne_test();
   gentest_idle();
   genbnel();
#endif
}

void genblezl(void)
{
#ifdef INTERPRET_BLEZL
   gencallinterp((unsigned int)BLEZL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BLEZL, 1);
    return;
     }
   
   genblez_test();
   free_all_registers();
   gentestl();
#endif
}

void genblezl_out(void)
{
#ifdef INTERPRET_BLEZL_OUT
   gencallinterp((unsigned int)BLEZL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BLEZL_OUT, 1);
    return;
     }
   
   genblez_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genblezl_idle(void)
{
#ifdef INTERPRET_BLEZL_IDLE
   gencallinterp((unsigned int)BLEZL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BLEZL_IDLE, 1);
    return;
     }
   
   genblez_test();
   gentest_idle();
   genblezl();
#endif
}

void genbgtzl(void)
{
#ifdef INTERPRET_BGTZL
   gencallinterp((unsigned int)BGTZL, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BGTZL, 1);
    return;
     }
   
   genbgtz_test();
   free_all_registers();
   gentestl();
#endif
}

void genbgtzl_out(void)
{
#ifdef INTERPRET_BGTZL_OUT
   gencallinterp((unsigned int)BGTZL_OUT, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BGTZL_OUT, 1);
    return;
     }
   
   genbgtz_test();
   free_all_registers();
   gentestl_out();
#endif
}

void genbgtzl_idle(void)
{
#ifdef INTERPRET_BGTZL_IDLE
   gencallinterp((unsigned int)BGTZL_IDLE, 1);
#else
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned int)BGTZL_IDLE, 1);
    return;
     }
   
   genbgtz_test();
   gentest_idle();
   genbgtzl();
#endif
}

void gendaddi(void)
{
#ifdef INTERPRET_DADDI
   gencallinterp((unsigned int)DADDI, 0);
#else
   int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
   int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
   int rt1 = allocate_64_register1_w((unsigned int *)dst->f.i.rt);
   int rt2 = allocate_64_register2_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_reg32(rt1, rs1);
   mov_reg32_reg32(rt2, rs2);
   add_reg32_imm32(rt1, dst->f.i.immediate);
   adc_reg32_imm32(rt2, (int)dst->f.i.immediate>>31);
#endif
}

void gendaddiu(void)
{
#ifdef INTERPRET_DADDIU
   gencallinterp((unsigned int)DADDIU, 0);
#else
   int rs1 = allocate_64_register1((unsigned int *)dst->f.i.rs);
   int rs2 = allocate_64_register2((unsigned int *)dst->f.i.rs);
   int rt1 = allocate_64_register1_w((unsigned int *)dst->f.i.rt);
   int rt2 = allocate_64_register2_w((unsigned int *)dst->f.i.rt);
   
   mov_reg32_reg32(rt1, rs1);
   mov_reg32_reg32(rt2, rs2);
   add_reg32_imm32(rt1, dst->f.i.immediate);
   adc_reg32_imm32(rt2, (int)dst->f.i.immediate>>31);
#endif
}

void genldl(void)
{
   gencallinterp((unsigned int)LDL, 0);
}

void genldr(void)
{
   gencallinterp((unsigned int)LDR, 0);
}

void genlb(void)
{
#ifdef INTERPRET_LB
   gencallinterp((unsigned int)LB, 0);
#else
   free_all_registers();
   simplify_access();
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmemb);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdramb);
     }
   je_rj(47);
   
   mov_m32_imm32((unsigned int *)&PC, (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_imm32((unsigned int *)(&rdword), (unsigned int)dst->f.i.rt); // 10
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmemb); // 7
   call_reg32(EBX); // 2
   movsx_reg32_m8(EAX, (unsigned char *)dst->f.i.rt); // 7
   jmp_imm_short(16); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   xor_reg8_imm8(BL, 3); // 3
   movsx_reg32_8preg32pimm32(EAX, EBX, (unsigned int)rdram); // 7
   
   set_register_state(EAX, (unsigned int*)dst->f.i.rt, 1);
#endif
}

void genlh(void)
{
#ifdef INTERPRET_LH
   gencallinterp((unsigned int)LH, 0);
#else
   free_all_registers();
   simplify_access();
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmemh);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdramh);
     }
   je_rj(47);
   
   mov_m32_imm32((unsigned int *)&PC, (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_imm32((unsigned int *)(&rdword), (unsigned int)dst->f.i.rt); // 10
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmemh); // 7
   call_reg32(EBX); // 2
   movsx_reg32_m16(EAX, (unsigned short *)dst->f.i.rt); // 7
   jmp_imm_short(16); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   xor_reg8_imm8(BL, 2); // 3
   movsx_reg32_16preg32pimm32(EAX, EBX, (unsigned int)rdram); // 7
   
   set_register_state(EAX, (unsigned int*)dst->f.i.rt, 1);
#endif
}

void genlwl(void)
{
   gencallinterp((unsigned int)LWL, 0);
}

void genlw(void)
{
#ifdef INTERPRET_LW
   gencallinterp((unsigned int)LW, 0);
#else
   free_all_registers();
   simplify_access();
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmem);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdram);
     }
   je_rj(45);
   
   mov_m32_imm32((unsigned int *)&PC, (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_imm32((unsigned int *)(&rdword), (unsigned int)dst->f.i.rt); // 10
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmem); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(dst->f.i.rt)); // 5
   jmp_imm_short(12); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_reg32_preg32pimm32(EAX, EBX, (unsigned int)rdram); // 6
   
   set_register_state(EAX, (unsigned int*)dst->f.i.rt, 1);
#endif
}

void genlbu(void)
{
#ifdef INTERPRET_LBU
   gencallinterp((unsigned int)LBU, 0);
#else
   free_all_registers();
   simplify_access();
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmemb);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdramb);
     }
   je_rj(46);
   
   mov_m32_imm32((unsigned int *)&PC, (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_imm32((unsigned int *)(&rdword), (unsigned int)dst->f.i.rt); // 10
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmemb); // 7
   call_reg32(EBX); // 2
   mov_reg32_m32(EAX, (unsigned int *)dst->f.i.rt); // 6
   jmp_imm_short(15); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   xor_reg8_imm8(BL, 3); // 3
   mov_reg32_preg32pimm32(EAX, EBX, (unsigned int)rdram); // 6
   
   and_eax_imm32(0xFF);
   
   set_register_state(EAX, (unsigned int*)dst->f.i.rt, 1);
#endif
}

void genlhu(void)
{
#ifdef INTERPRET_LHU
   gencallinterp((unsigned int)LHU, 0);
#else
   free_all_registers();
   simplify_access();
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmemh);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdramh);
     }
   je_rj(46);
   
   mov_m32_imm32((unsigned int *)&PC, (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_imm32((unsigned int *)(&rdword), (unsigned int)dst->f.i.rt); // 10
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmemh); // 7
   call_reg32(EBX); // 2
   mov_reg32_m32(EAX, (unsigned int *)dst->f.i.rt); // 6
   jmp_imm_short(15); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   xor_reg8_imm8(BL, 2); // 3
   mov_reg32_preg32pimm32(EAX, EBX, (unsigned int)rdram); // 6
   
   and_eax_imm32(0xFFFF);
   
   set_register_state(EAX, (unsigned int*)dst->f.i.rt, 1);
#endif
}

void genlwr(void)
{
   gencallinterp((unsigned int)LWR, 0);
}

void genlwu(void)
{
#ifdef INTERPRET_LWU
   gencallinterp((unsigned int)LWU, 0);
#else
   free_all_registers();
   simplify_access();
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmem);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdram);
     }
   je_rj(45);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_imm32((unsigned int *)(&rdword), (unsigned int)dst->f.i.rt); // 10
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmem); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(dst->f.i.rt)); // 5
   jmp_imm_short(12); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_reg32_preg32pimm32(EAX, EBX, (unsigned int)rdram); // 6
   
   xor_reg32_reg32(EBX, EBX);
   
   set_64_register_state(EAX, EBX, (unsigned int*)dst->f.i.rt, 1);
#endif
}

void gensb(void)
{
#ifdef INTERPRET_SB
   gencallinterp((unsigned int)SB, 0);
#else
   free_all_registers();
   simplify_access();
   mov_reg8_m8(CL, (unsigned char *)dst->f.i.rt);
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)writememb);
    cmp_reg32_imm32(EAX, (unsigned int)write_rdramb);
     }
   je_rj(41);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m8_reg8((unsigned char *)(&cpu_byte), CL); // 6
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)writememb); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(&address)); // 5
   jmp_imm_short(17); // 2
   
   mov_reg32_reg32(EAX, EBX); // 2
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   xor_reg8_imm8(BL, 3); // 3
   mov_preg32pimm32_reg8(EBX, (unsigned int)rdram, CL); // 6
   
   mov_reg32_reg32(EBX, EAX);
   shr_reg32_imm8(EBX, 12);
   cmp_preg32pimm32_imm8(EBX, (unsigned int)invalid_code, 0);
   jne_rj(54);
   mov_reg32_reg32(ECX, EBX); // 2
   shl_reg32_imm8(EBX, 2); // 3
   mov_reg32_preg32pimm32(EBX, EBX, (unsigned int)blocks); // 6
   mov_reg32_preg32pimm32(EBX, EBX, (int)&actual->block - (int)actual); // 6
   and_eax_imm32(0xFFF); // 5
   shr_reg32_imm8(EAX, 2); // 3
   mov_reg32_imm32(EDX, sizeof(precomp_instr)); // 5
   mul_reg32(EDX); // 2
   mov_reg32_preg32preg32pimm32(EAX, EAX, EBX, (int)&dst->ops - (int)dst); // 7
   cmp_reg32_imm32(EAX, (unsigned int)NOTCOMPILED); // 6
   je_rj(7); // 2
   mov_preg32pimm32_imm8(ECX, (unsigned int)invalid_code, 1); // 7
#endif
}

void gensh(void)
{
#ifdef INTERPRET_SH
   gencallinterp((unsigned int)SH, 0);
#else
   free_all_registers();
   simplify_access();
   mov_reg16_m16(CX, (unsigned short *)dst->f.i.rt);
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)writememh);
    cmp_reg32_imm32(EAX, (unsigned int)write_rdramh);
     }
   je_rj(42);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m16_reg16((unsigned short *)(&hword), CX); // 7
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)writememh); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(&address)); // 5
   jmp_imm_short(18); // 2
   
   mov_reg32_reg32(EAX, EBX); // 2
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   xor_reg8_imm8(BL, 2); // 3
   mov_preg32pimm32_reg16(EBX, (unsigned int)rdram, CX); // 7
   
   mov_reg32_reg32(EBX, EAX);
   shr_reg32_imm8(EBX, 12);
   cmp_preg32pimm32_imm8(EBX, (unsigned int)invalid_code, 0);
   jne_rj(54);
   mov_reg32_reg32(ECX, EBX); // 2
   shl_reg32_imm8(EBX, 2); // 3
   mov_reg32_preg32pimm32(EBX, EBX, (unsigned int)blocks); // 6
   mov_reg32_preg32pimm32(EBX, EBX, (int)&actual->block - (int)actual); // 6
   and_eax_imm32(0xFFF); // 5
   shr_reg32_imm8(EAX, 2); // 3
   mov_reg32_imm32(EDX, sizeof(precomp_instr)); // 5
   mul_reg32(EDX); // 2
   mov_reg32_preg32preg32pimm32(EAX, EAX, EBX, (int)&dst->ops - (int)dst); // 7
   cmp_reg32_imm32(EAX, (unsigned int)NOTCOMPILED); // 6
   je_rj(7); // 2
   mov_preg32pimm32_imm8(ECX, (unsigned int)invalid_code, 1); // 7
#endif
}

void genswl(void)
{
   gencallinterp((unsigned int)SWL, 0);
}

void gensw(void)
{
#ifdef INTERPRET_SW
   gencallinterp((unsigned int)SW, 0);
#else
   free_all_registers();
   simplify_access();
   mov_reg32_m32(ECX, (unsigned int *)dst->f.i.rt);
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)writemem);
    cmp_reg32_imm32(EAX, (unsigned int)write_rdram);
     }
   je_rj(41);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_reg32((unsigned int *)(&word), ECX); // 6
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)writemem); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(&address)); // 5
   jmp_imm_short(14); // 2
   
   mov_reg32_reg32(EAX, EBX); // 2
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_preg32pimm32_reg32(EBX, (unsigned int)rdram, ECX); // 6
   
   mov_reg32_reg32(EBX, EAX);
   shr_reg32_imm8(EBX, 12);
   cmp_preg32pimm32_imm8(EBX, (unsigned int)invalid_code, 0);
   jne_rj(54);
   mov_reg32_reg32(ECX, EBX); // 2
   shl_reg32_imm8(EBX, 2); // 3
   mov_reg32_preg32pimm32(EBX, EBX, (unsigned int)blocks); // 6
   mov_reg32_preg32pimm32(EBX, EBX, (int)&actual->block - (int)actual); // 6
   and_eax_imm32(0xFFF); // 5
   shr_reg32_imm8(EAX, 2); // 3
   mov_reg32_imm32(EDX, sizeof(precomp_instr)); // 5
   mul_reg32(EDX); // 2
   mov_reg32_preg32preg32pimm32(EAX, EAX, EBX, (int)&dst->ops - (int)dst); // 7
   cmp_reg32_imm32(EAX, (unsigned int)NOTCOMPILED); // 6
   je_rj(7); // 2
   mov_preg32pimm32_imm8(ECX, (unsigned int)invalid_code, 1); // 7
#endif
}

void gensdl(void)
{
   gencallinterp((unsigned int)SDL, 0);
}

void gensdr(void)
{
   gencallinterp((unsigned int)SDR, 0);
}

void genswr(void)
{
   gencallinterp((unsigned int)SWR, 0);
}

void gencheck_cop1_unusable(void)
{
   free_all_registers();
   simplify_access();
   test_m32_imm32((unsigned int*)&Status, 0x20000000);
   jne_rj(0);

   jump_start_rel8();
   
   gencallinterp((unsigned int)check_cop1_unusable, 0);
   
   jump_end_rel8();
}

void genlwc1(void)
{
#ifdef INTERPRET_LWC1
   gencallinterp((unsigned int)LWC1, 0);
#else
   gencheck_cop1_unusable();
   
   mov_eax_memoffs32((unsigned int *)(&reg[dst->f.lf.base]));
   add_eax_imm32((int)dst->f.lf.offset);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmem);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdram);
     }
   je_rj(42);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_reg32_m32(EDX, (unsigned int*)(&reg_cop1_simple[dst->f.lf.ft])); // 6
   mov_m32_reg32((unsigned int *)(&rdword), EDX); // 6
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmem); // 7
   call_reg32(EBX); // 2
   jmp_imm_short(20); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_reg32_preg32pimm32(EAX, EBX, (unsigned int)rdram); // 6
   mov_reg32_m32(EBX, (unsigned int*)(&reg_cop1_simple[dst->f.lf.ft])); // 6
   mov_preg32_reg32(EBX, EAX); // 2
#endif
}

void genldc1(void)
{
#ifdef INTERPRET_LDC1
   gencallinterp((unsigned int)LDC1, 0);
#else
   gencheck_cop1_unusable();
   
   mov_eax_memoffs32((unsigned int *)(&reg[dst->f.lf.base]));
   add_eax_imm32((int)dst->f.lf.offset);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmemd);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdramd);
     }
   je_rj(42);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_reg32_m32(EDX, (unsigned int*)(&reg_cop1_double[dst->f.lf.ft])); // 6
   mov_m32_reg32((unsigned int *)(&rdword), EDX); // 6
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmemd); // 7
   call_reg32(EBX); // 2
   jmp_imm_short(32); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_reg32_preg32pimm32(EAX, EBX, ((unsigned int)rdram)+4); // 6
   mov_reg32_preg32pimm32(ECX, EBX, ((unsigned int)rdram)); // 6
   mov_reg32_m32(EBX, (unsigned int*)(&reg_cop1_double[dst->f.lf.ft])); // 6
   mov_preg32_reg32(EBX, EAX); // 2
   mov_preg32pimm32_reg32(EBX, 4, ECX); // 6
#endif
}

void gencache(void)
{
}

void genld(void)
{
#ifdef INTERPRET_LD
   gencallinterp((unsigned int)LD, 0);
#else
   free_all_registers();
   simplify_access();
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)readmemd);
    cmp_reg32_imm32(EAX, (unsigned int)read_rdramd);
     }
   je_rj(51);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_imm32((unsigned int *)(&rdword), (unsigned int)dst->f.i.rt); // 10
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)readmemd); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(dst->f.i.rt)); // 5
   mov_reg32_m32(ECX, (unsigned int *)(dst->f.i.rt)+1); // 6
   jmp_imm_short(18); // 2
   
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_reg32_preg32pimm32(EAX, EBX, ((unsigned int)rdram)+4); // 6
   mov_reg32_preg32pimm32(ECX, EBX, ((unsigned int)rdram)); // 6
   
   set_64_register_state(EAX, ECX, (unsigned int*)dst->f.i.rt, 1);
#endif
}

void genswc1(void)
{
#ifdef INTERPRET_SWC1
   gencallinterp((unsigned int)SWC1, 0);
#else
   gencheck_cop1_unusable();
   
   mov_reg32_m32(EDX, (unsigned int*)(&reg_cop1_simple[dst->f.lf.ft]));
   mov_reg32_preg32(ECX, EDX);
   mov_eax_memoffs32((unsigned int *)(&reg[dst->f.lf.base]));
   add_eax_imm32((int)dst->f.lf.offset);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)writemem);
    cmp_reg32_imm32(EAX, (unsigned int)write_rdram);
     }
   je_rj(41);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_reg32((unsigned int *)(&word), ECX); // 6
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)writemem); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(&address)); // 5
   jmp_imm_short(14); // 2
   
   mov_reg32_reg32(EAX, EBX); // 2
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_preg32pimm32_reg32(EBX, (unsigned int)rdram, ECX); // 6
   
   mov_reg32_reg32(EBX, EAX);
   shr_reg32_imm8(EBX, 12);
   cmp_preg32pimm32_imm8(EBX, (unsigned int)invalid_code, 0);
   jne_rj(54);
   mov_reg32_reg32(ECX, EBX); // 2
   shl_reg32_imm8(EBX, 2); // 3
   mov_reg32_preg32pimm32(EBX, EBX, (unsigned int)blocks); // 6
   mov_reg32_preg32pimm32(EBX, EBX, (int)&actual->block - (int)actual); // 6
   and_eax_imm32(0xFFF); // 5
   shr_reg32_imm8(EAX, 2); // 3
   mov_reg32_imm32(EDX, sizeof(precomp_instr)); // 5
   mul_reg32(EDX); // 2
   mov_reg32_preg32preg32pimm32(EAX, EAX, EBX, (int)&dst->ops - (int)dst); // 7
   cmp_reg32_imm32(EAX, (unsigned int)NOTCOMPILED); // 6
   je_rj(7); // 2
   mov_preg32pimm32_imm8(ECX, (unsigned int)invalid_code, 1); // 7
#endif
}

void gensdc1(void)
{
#ifdef INTERPRET_SDC1
   gencallinterp((unsigned int)SDC1, 0);
#else
   gencheck_cop1_unusable();
   
   mov_reg32_m32(ESI, (unsigned int*)(&reg_cop1_double[dst->f.lf.ft]));
   mov_reg32_preg32(ECX, ESI);
   mov_reg32_preg32pimm32(EDX, ESI, 4);
   mov_eax_memoffs32((unsigned int *)(&reg[dst->f.lf.base]));
   add_eax_imm32((int)dst->f.lf.offset);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)writememd);
    cmp_reg32_imm32(EAX, (unsigned int)write_rdramd);
     }
   je_rj(47);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_reg32((unsigned int *)(&dword), ECX); // 6
   mov_m32_reg32((unsigned int *)(&dword)+1, EDX); // 6
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)writememd); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(&address)); // 5
   jmp_imm_short(20); // 2
   
   mov_reg32_reg32(EAX, EBX); // 2
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_preg32pimm32_reg32(EBX, ((unsigned int)rdram)+4, ECX); // 6
   mov_preg32pimm32_reg32(EBX, ((unsigned int)rdram)+0, EDX); // 6
   
   mov_reg32_reg32(EBX, EAX);
   shr_reg32_imm8(EBX, 12);
   cmp_preg32pimm32_imm8(EBX, (unsigned int)invalid_code, 0);
   jne_rj(54);
   mov_reg32_reg32(ECX, EBX); // 2
   shl_reg32_imm8(EBX, 2); // 3
   mov_reg32_preg32pimm32(EBX, EBX, (unsigned int)blocks); // 6
   mov_reg32_preg32pimm32(EBX, EBX, (int)&actual->block - (int)actual); // 6
   and_eax_imm32(0xFFF); // 5
   shr_reg32_imm8(EAX, 2); // 3
   mov_reg32_imm32(EDX, sizeof(precomp_instr)); // 5
   mul_reg32(EDX); // 2
   mov_reg32_preg32preg32pimm32(EAX, EAX, EBX, (int)&dst->ops - (int)dst); // 7
   cmp_reg32_imm32(EAX, (unsigned int)NOTCOMPILED); // 6
   je_rj(7); // 2
   mov_preg32pimm32_imm8(ECX, (unsigned int)invalid_code, 1); // 7
#endif
}

void gensd(void)
{
#ifdef INTERPRET_SD
   gencallinterp((unsigned int)SD, 0);
#else
   free_all_registers();
   simplify_access();
   
   mov_reg32_m32(ECX, (unsigned int *)dst->f.i.rt);
   mov_reg32_m32(EDX, ((unsigned int *)dst->f.i.rt)+1);
   mov_eax_memoffs32((unsigned int *)dst->f.i.rs);
   add_eax_imm32((int)dst->f.i.immediate);
   mov_reg32_reg32(EBX, EAX);
   if(fast_memory)
     {
    and_eax_imm32(0xDF800000);
    cmp_eax_imm32(0x80000000);
     }
   else
     {
    shr_reg32_imm8(EAX, 16);
    mov_reg32_preg32x4pimm32(EAX, EAX, (unsigned int)writememd);
    cmp_reg32_imm32(EAX, (unsigned int)write_rdramd);
     }
   je_rj(47);
   
   mov_m32_imm32((unsigned int *)(&PC), (unsigned int)(dst+1)); // 10
   mov_m32_reg32((unsigned int *)(&address), EBX); // 6
   mov_m32_reg32((unsigned int *)(&dword), ECX); // 6
   mov_m32_reg32((unsigned int *)(&dword)+1, EDX); // 6
   shr_reg32_imm8(EBX, 16); // 3
   mov_reg32_preg32x4pimm32(EBX, EBX, (unsigned int)writememd); // 7
   call_reg32(EBX); // 2
   mov_eax_memoffs32((unsigned int *)(&address)); // 5
   jmp_imm_short(20); // 2
   
   mov_reg32_reg32(EAX, EBX); // 2
   and_reg32_imm32(EBX, 0x7FFFFF); // 6
   mov_preg32pimm32_reg32(EBX, ((unsigned int)rdram)+4, ECX); // 6
   mov_preg32pimm32_reg32(EBX, ((unsigned int)rdram)+0, EDX); // 6
   
   mov_reg32_reg32(EBX, EAX);
   shr_reg32_imm8(EBX, 12);
   cmp_preg32pimm32_imm8(EBX, (unsigned int)invalid_code, 0);
   jne_rj(54);
   mov_reg32_reg32(ECX, EBX); // 2
   shl_reg32_imm8(EBX, 2); // 3
   mov_reg32_preg32pimm32(EBX, EBX, (unsigned int)blocks); // 6
   mov_reg32_preg32pimm32(EBX, EBX, (int)&actual->block - (int)actual); // 6
   and_eax_imm32(0xFFF); // 5
   shr_reg32_imm8(EAX, 2); // 3
   mov_reg32_imm32(EDX, sizeof(precomp_instr)); // 5
   mul_reg32(EDX); // 2
   mov_reg32_preg32preg32pimm32(EAX, EAX, EBX, (int)&dst->ops - (int)dst); // 7
   cmp_reg32_imm32(EAX, (unsigned int)NOTCOMPILED); // 6
   je_rj(7); // 2
   mov_preg32pimm32_imm8(ECX, (unsigned int)invalid_code, 1); // 7
#endif
}

void genll(void)
{
   gencallinterp((unsigned int)LL, 0);
}

void gensc(void)
{
   gencallinterp((unsigned int)SC, 0);
}

