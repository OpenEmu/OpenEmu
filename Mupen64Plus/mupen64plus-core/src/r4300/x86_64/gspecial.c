/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gspecial.c                                              *
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
#include "r4300/exception.h"

#if !defined(offsetof)
#   define offsetof(TYPE,MEMBER) ((unsigned int) &((TYPE*)0)->MEMBER)
#endif

void gensll(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[55]);
#endif
#ifdef INTERPRET_SLL
   gencallinterp((unsigned long long)cached_interpreter_table.SLL, 0);
#else
   int rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   int rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);
   
   mov_reg32_reg32(rd, rt);
   shl_reg32_imm8(rd, dst->f.r.sa);
#endif
}

void gensrl(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[56]);
#endif
#ifdef INTERPRET_SRL
   gencallinterp((unsigned long long)cached_interpreter_table.SRL, 0);
#else
   int rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   int rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);
   
   mov_reg32_reg32(rd, rt);
   shr_reg32_imm8(rd, dst->f.r.sa);
#endif
}

void gensra(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[57]);
#endif
#ifdef INTERPRET_SRA
   gencallinterp((unsigned long long)cached_interpreter_table.SRA, 0);
#else
   int rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   int rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);
   
   mov_reg32_reg32(rd, rt);
   sar_reg32_imm8(rd, dst->f.r.sa);
#endif
}

void gensllv(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[58]);
#endif
#ifdef INTERPRET_SLLV
   gencallinterp((unsigned long long)cached_interpreter_table.SLLV, 0);
#else
   int rt, rd;
   allocate_register_32_manually(ECX, (unsigned int *)dst->f.r.rs);
   
   rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);
   
   if (rd != ECX)
     {
    mov_reg32_reg32(rd, rt);
    shl_reg32_cl(rd);
     }
   else
     {
    int temp = lru_register();
    free_register(temp);
    mov_reg32_reg32(temp, rt);
    shl_reg32_cl(temp);
    mov_reg32_reg32(rd, temp);
     }
#endif
}

void gensrlv(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[59]);
#endif
#ifdef INTERPRET_SRLV
   gencallinterp((unsigned long long)cached_interpreter_table.SRLV, 0);
#else
   int rt, rd;
   allocate_register_32_manually(ECX, (unsigned int *)dst->f.r.rs);
   
   rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);
   
   if (rd != ECX)
     {
    mov_reg32_reg32(rd, rt);
    shr_reg32_cl(rd);
     }
   else
     {
    int temp = lru_register();
    free_register(temp);
    mov_reg32_reg32(temp, rt);
    shr_reg32_cl(temp);
    mov_reg32_reg32(rd, temp);
     }
#endif
}

void gensrav(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[60]);
#endif
#ifdef INTERPRET_SRAV
   gencallinterp((unsigned long long)cached_interpreter_table.SRAV, 0);
#else
   int rt, rd;
   allocate_register_32_manually(ECX, (unsigned int *)dst->f.r.rs);
   
   rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);
   
   if (rd != ECX)
     {
    mov_reg32_reg32(rd, rt);
    sar_reg32_cl(rd);
     }
   else
     {
    int temp = lru_register();
    free_register(temp);
    mov_reg32_reg32(temp, rt);
    sar_reg32_cl(temp);
    mov_reg32_reg32(rd, temp);
     }
#endif
}

void genjr(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[61]);
#endif
#ifdef INTERPRET_JR
   gencallinterp((unsigned long long)cached_interpreter_table.JR, 1);
#else
   static unsigned int precomp_instr_size = sizeof(precomp_instr);
   unsigned int diff = (unsigned int) offsetof(precomp_instr, local_addr);
   unsigned int diff_need = (unsigned int) offsetof(precomp_instr, reg_cache_infos.need_map);
   unsigned int diff_wrap = (unsigned int) offsetof(precomp_instr, reg_cache_infos.jump_wrapper);
   
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)cached_interpreter_table.JR, 1);
    return;
     }
   
   free_registers_move_start();

   mov_xreg32_m32rel(EAX, (unsigned int *)dst->f.i.rs);
   mov_m32rel_xreg32((unsigned int *)&local_rs, EAX);
   
   gendelayslot();
   
   mov_xreg32_m32rel(EAX, (unsigned int *)&local_rs);
   mov_m32rel_xreg32((unsigned int *)&last_addr, EAX);
   
   gencheck_interupt_reg();
   
   mov_xreg32_m32rel(EAX, (unsigned int *)&local_rs);
   mov_reg32_reg32(EBX, EAX);
   and_eax_imm32(0xFFFFF000);
   cmp_eax_imm32(dst_block->start & 0xFFFFF000);
   je_near_rj(0);

   jump_start_rel32();
   
   mov_m32rel_xreg32(&jump_to_address, EBX);
   mov_reg64_imm64(RAX, (unsigned long long) (dst+1));
   mov_m64rel_xreg64((unsigned long long *)(&PC), RAX);
   mov_reg64_imm64(RAX, (unsigned long long) jump_to_func);
   call_reg64(RAX);  /* will never return from call */

   jump_end_rel32();

   mov_reg64_imm64(RSI, (unsigned long long) dst_block->block);
   mov_reg32_reg32(EAX, EBX);
   sub_eax_imm32(dst_block->start);
   shr_reg32_imm8(EAX, 2);
   mul_m32rel((unsigned int *)(&precomp_instr_size));
   
   mov_reg32_preg64preg64pimm32(EBX, RAX, RSI, diff_need);
   cmp_reg32_imm32(EBX, 1);
   jne_rj(11);

   add_reg32_imm32(EAX, diff_wrap); // 6
   add_reg64_reg64(RAX, RSI); // 3
   jmp_reg64(RAX); // 2

   mov_reg32_preg64preg64pimm32(EBX, RAX, RSI, diff);
   mov_rax_memoffs64((unsigned long long *) &dst_block->code);
   add_reg64_reg64(RAX, RBX);
   jmp_reg64(RAX);
#endif
}

void genjalr(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[62]);
#endif
#ifdef INTERPRET_JALR
   gencallinterp((unsigned long long)cached_interpreter_table.JALR, 0);
#else
   static unsigned int precomp_instr_size = sizeof(precomp_instr);
   unsigned int diff = (unsigned int) offsetof(precomp_instr, local_addr);
   unsigned int diff_need = (unsigned int) offsetof(precomp_instr, reg_cache_infos.need_map);
   unsigned int diff_wrap = (unsigned int) offsetof(precomp_instr, reg_cache_infos.jump_wrapper);
   
   if (((dst->addr & 0xFFF) == 0xFFC && 
       (dst->addr < 0x80000000 || dst->addr >= 0xC0000000))||no_compiled_jump)
     {
    gencallinterp((unsigned long long)cached_interpreter_table.JALR, 1);
    return;
     }
   
   free_registers_move_start();

   mov_xreg32_m32rel(EAX, (unsigned int *)dst->f.r.rs);
   mov_m32rel_xreg32((unsigned int *)&local_rs, EAX);
   
   gendelayslot();
   
   mov_m32rel_imm32((unsigned int *)(dst-1)->f.r.rd, dst->addr+4);
   if ((dst->addr+4) & 0x80000000)
     mov_m32rel_imm32(((unsigned int *)(dst-1)->f.r.rd)+1, 0xFFFFFFFF);
   else
     mov_m32rel_imm32(((unsigned int *)(dst-1)->f.r.rd)+1, 0);
   
   mov_xreg32_m32rel(EAX, (unsigned int *)&local_rs);
   mov_m32rel_xreg32((unsigned int *)&last_addr, EAX);
   
   gencheck_interupt_reg();
   
   mov_xreg32_m32rel(EAX, (unsigned int *)&local_rs);
   mov_reg32_reg32(EBX, EAX);
   and_eax_imm32(0xFFFFF000);
   cmp_eax_imm32(dst_block->start & 0xFFFFF000);
   je_near_rj(0);

   jump_start_rel32();
   
   mov_m32rel_xreg32(&jump_to_address, EBX);
   mov_reg64_imm64(RAX, (unsigned long long) (dst+1));
   mov_m64rel_xreg64((unsigned long long *)(&PC), RAX);
   mov_reg64_imm64(RAX, (unsigned long long) jump_to_func);
   call_reg64(RAX);  /* will never return from call */

   jump_end_rel32();

   mov_reg64_imm64(RSI, (unsigned long long) dst_block->block);
   mov_reg32_reg32(EAX, EBX);
   sub_eax_imm32(dst_block->start);
   shr_reg32_imm8(EAX, 2);
   mul_m32rel((unsigned int *)(&precomp_instr_size));

   mov_reg32_preg64preg64pimm32(EBX, RAX, RSI, diff_need);
   cmp_reg32_imm32(EBX, 1);
   jne_rj(11);

   add_reg32_imm32(EAX, diff_wrap); // 6
   add_reg64_reg64(RAX, RSI); // 3
   jmp_reg64(RAX); // 2

   mov_reg32_preg64preg64pimm32(EBX, RAX, RSI, diff);
   mov_rax_memoffs64((unsigned long long *) &dst_block->code);
   add_reg64_reg64(RAX, RBX);
   jmp_reg64(RAX);
#endif
}

void gensyscall(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[63]);
#endif
#ifdef INTERPRET_SYSCALL
   gencallinterp((unsigned long long)cached_interpreter_table.SYSCALL, 0);
#else
   free_registers_move_start();

   mov_m32rel_imm32(&Cause, 8 << 2);
   gencallinterp((unsigned long long)exception_general, 0);
#endif
}

void gensync(void)
{
}

void genmfhi(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[64]);
#endif
#ifdef INTERPRET_MFHI
   gencallinterp((unsigned long long)cached_interpreter_table.MFHI, 0);
#else
   int rd = allocate_register_64_w((unsigned long long *) dst->f.r.rd);
   int _hi = allocate_register_64((unsigned long long *) &hi);
   
   mov_reg64_reg64(rd, _hi);
#endif
}

void genmthi(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[65]);
#endif
#ifdef INTERPRET_MTHI
   gencallinterp((unsigned long long)cached_interpreter_table.MTHI, 0);
#else
   int _hi = allocate_register_64_w((unsigned long long *) &hi);
   int rs = allocate_register_64((unsigned long long *) dst->f.r.rs);

   mov_reg64_reg64(_hi, rs);
#endif
}

void genmflo(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[66]);
#endif
#ifdef INTERPRET_MFLO
   gencallinterp((unsigned long long)cached_interpreter_table.MFLO, 0);
#else
   int rd = allocate_register_64_w((unsigned long long *) dst->f.r.rd);
   int _lo = allocate_register_64((unsigned long long *) &lo);
   
   mov_reg64_reg64(rd, _lo);
#endif
}

void genmtlo(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[67]);
#endif
#ifdef INTERPRET_MTLO
   gencallinterp((unsigned long long)cached_interpreter_table.MTLO, 0);
#else
   int _lo = allocate_register_64_w((unsigned long long *)&lo);
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);

   mov_reg64_reg64(_lo, rs);
#endif
}

void gendsllv(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[68]);
#endif
#ifdef INTERPRET_DSLLV
   gencallinterp((unsigned long long)cached_interpreter_table.DSLLV, 0);
#else
   int rt, rd;
   allocate_register_32_manually(ECX, (unsigned int *)dst->f.r.rs);
   
   rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   if (rd != ECX)
     {
    mov_reg64_reg64(rd, rt);
    shl_reg64_cl(rd);
     }
   else
     {
    int temp;
    temp = lru_register();
    free_register(temp);
    
    mov_reg64_reg64(temp, rt);
    shl_reg64_cl(temp);
    mov_reg64_reg64(rd, temp);
     }
#endif
}

void gendsrlv(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[69]);
#endif
#ifdef INTERPRET_DSRLV
   gencallinterp((unsigned long long)cached_interpreter_table.DSRLV, 0);
#else
   int rt, rd;
   allocate_register_32_manually(ECX, (unsigned int *)dst->f.r.rs);
   
   rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   if (rd != ECX)
     {
    mov_reg64_reg64(rd, rt);
    shr_reg64_cl(rd);
     }
   else
     {
    int temp;
    temp = lru_register();
    free_register(temp);
    
    mov_reg64_reg64(temp, rt);
    shr_reg64_cl(temp);
    mov_reg64_reg64(rd, temp);
     }
#endif
}

void gendsrav(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[70]);
#endif
#ifdef INTERPRET_DSRAV
   gencallinterp((unsigned long long)cached_interpreter_table.DSRAV, 0);
#else
   int rt, rd;
   allocate_register_32_manually(ECX, (unsigned int *)dst->f.r.rs);
   
   rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   if (rd != ECX)
     {
    mov_reg64_reg64(rd, rt);
    sar_reg64_cl(rd);
     }
   else
     {
    int temp;
    temp = lru_register();
    free_register(temp);
    
    mov_reg64_reg64(temp, rt);
    sar_reg64_cl(temp);
    mov_reg64_reg64(rd, temp);
     }
#endif
}

void genmult(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[71]);
#endif
#ifdef INTERPRET_MULT
   gencallinterp((unsigned long long)cached_interpreter_table.MULT, 0);
#else
   int rs, rt;
   allocate_register_32_manually_w(EAX, (unsigned int *)&lo); /* these must be done first so they are not assigned by allocate_register() */
   allocate_register_32_manually_w(EDX, (unsigned int *)&hi);
   rs = allocate_register_32((unsigned int*)dst->f.r.rs);
   rt = allocate_register_32((unsigned int*)dst->f.r.rt);
   mov_reg32_reg32(EAX, rs);
   imul_reg32(rt);
#endif
}

void genmultu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[72]);
#endif
#ifdef INTERPRET_MULTU
   gencallinterp((unsigned long long)cached_interpreter_table.MULTU, 0);
#else
   int rs, rt;
   allocate_register_32_manually_w(EAX, (unsigned int *)&lo);
   allocate_register_32_manually_w(EDX, (unsigned int *)&hi);
   rs = allocate_register_32((unsigned int*)dst->f.r.rs);
   rt = allocate_register_32((unsigned int*)dst->f.r.rt);
   mov_reg32_reg32(EAX, rs);
   mul_reg32(rt);
#endif
}

void gendiv(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[73]);
#endif
#ifdef INTERPRET_DIV
   gencallinterp((unsigned long long)cached_interpreter_table.DIV, 0);
#else
   int rs, rt;
   allocate_register_32_manually_w(EAX, (unsigned int *)&lo);
   allocate_register_32_manually_w(EDX, (unsigned int *)&hi);
   rs = allocate_register_32((unsigned int*)dst->f.r.rs);
   rt = allocate_register_32((unsigned int*)dst->f.r.rt);
   cmp_reg32_imm32(rt, 0);
   je_rj((rs == EAX ? 0 : 2) + 1 + 2);
   mov_reg32_reg32(EAX, rs); // 0 or 2
   cdq(); // 1
   idiv_reg32(rt); // 2
#endif
}

void gendivu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[74]);
#endif
#ifdef INTERPRET_DIVU
   gencallinterp((unsigned long long)cached_interpreter_table.DIVU, 0);
#else
   int rs, rt;
   allocate_register_32_manually_w(EAX, (unsigned int *)&lo);
   allocate_register_32_manually_w(EDX, (unsigned int *)&hi);
   rs = allocate_register_32((unsigned int*)dst->f.r.rs);
   rt = allocate_register_32((unsigned int*)dst->f.r.rt);
   cmp_reg32_imm32(rt, 0);
   je_rj((rs == EAX ? 0 : 2) + 2 + 2);
   mov_reg32_reg32(EAX, rs); // 0 or 2
   xor_reg32_reg32(EDX, EDX); // 2
   div_reg32(rt); // 2
#endif
}

void gendmult(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[75]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.DMULT, 0);
}

void gendmultu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[76]);
#endif
#ifdef INTERPRET_DMULTU
   gencallinterp((unsigned long long)cached_interpreter_table.DMULTU, 0);
#else
   free_registers_move_start();
   
   mov_xreg64_m64rel(RAX, (unsigned long long *) dst->f.r.rs);
   mov_xreg64_m64rel(RDX, (unsigned long long *) dst->f.r.rt);
   mul_reg64(RDX);
   mov_m64rel_xreg64((unsigned long long *) &lo, RAX);
   mov_m64rel_xreg64((unsigned long long *) &hi, RDX);
#endif
}

void genddiv(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[77]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.DDIV, 0);
}

void genddivu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[78]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.DDIVU, 0);
}

void genadd(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[79]);
#endif
#ifdef INTERPRET_ADD
   gencallinterp((unsigned long long)cached_interpreter_table.ADD, 0);
#else
   int rs = allocate_register_32((unsigned int *)dst->f.r.rs);
   int rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   int rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);

   if (rs == rd)
     add_reg32_reg32(rd, rt);
   else if (rt == rd)
     add_reg32_reg32(rd, rs);
   else
     {
    mov_reg32_reg32(rd, rs);
    add_reg32_reg32(rd, rt);
     }
#endif
}

void genaddu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[80]);
#endif
#ifdef INTERPRET_ADDU
   gencallinterp((unsigned long long)cached_interpreter_table.ADDU, 0);
#else
   int rs = allocate_register_32((unsigned int *)dst->f.r.rs);
   int rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   int rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);

   if (rs == rd)
     add_reg32_reg32(rd, rt);
   else if (rt == rd)
     add_reg32_reg32(rd, rs);
   else
     {
    mov_reg32_reg32(rd, rs);
    add_reg32_reg32(rd, rt);
     }
#endif
}

void gensub(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[81]);
#endif
#ifdef INTERPRET_SUB
   gencallinterp((unsigned long long)cached_interpreter_table.SUB, 0);
#else
   int rs = allocate_register_32((unsigned int *)dst->f.r.rs);
   int rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   int rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);

   if (rs == rd)
     sub_reg32_reg32(rd, rt);
   else if (rt == rd)
   {
     neg_reg32(rd);
     add_reg32_reg32(rd, rs);
   }
   else
   {
    mov_reg32_reg32(rd, rs);
    sub_reg32_reg32(rd, rt);
   }
#endif
}

void gensubu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[82]);
#endif
#ifdef INTERPRET_SUBU
   gencallinterp((unsigned long long)cached_interpreter_table.SUBU, 0);
#else
   int rs = allocate_register_32((unsigned int *)dst->f.r.rs);
   int rt = allocate_register_32((unsigned int *)dst->f.r.rt);
   int rd = allocate_register_32_w((unsigned int *)dst->f.r.rd);

   if (rs == rd)
     sub_reg32_reg32(rd, rt);
   else if (rt == rd)
   {
     neg_reg32(rd);
     add_reg32_reg32(rd, rs);
   }
   else
     {
    mov_reg32_reg32(rd, rs);
    sub_reg32_reg32(rd, rt);
     }
#endif
}

void genand(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[83]);
#endif
#ifdef INTERPRET_AND
   gencallinterp((unsigned long long)cached_interpreter_table.AND, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   if (rs == rd)
     and_reg64_reg64(rd, rt);
   else if (rt == rd)
     and_reg64_reg64(rd, rs);
   else
     {
    mov_reg64_reg64(rd, rs);
    and_reg64_reg64(rd, rt);
     }
#endif
}

void genor(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[84]);
#endif
#ifdef INTERPRET_OR
   gencallinterp((unsigned long long)cached_interpreter_table.OR, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   if (rs == rd)
     or_reg64_reg64(rd, rt);
   else if (rt == rd)
     or_reg64_reg64(rd, rs);
   else
     {
    mov_reg64_reg64(rd, rs);
    or_reg64_reg64(rd, rt);
     }
#endif
}

void genxor(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[85]);
#endif
#ifdef INTERPRET_XOR
   gencallinterp((unsigned long long)cached_interpreter_table.XOR, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   if (rs == rd)
     xor_reg64_reg64(rd, rt);
   else if (rt == rd)
     xor_reg64_reg64(rd, rs);
   else
     {
    mov_reg64_reg64(rd, rs);
    xor_reg64_reg64(rd, rt);
     }
#endif
}

void gennor(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[86]);
#endif
#ifdef INTERPRET_NOR
   gencallinterp((unsigned long long)cached_interpreter_table.NOR, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   if (rs == rd)
   {
     or_reg64_reg64(rd, rt);
     not_reg64(rd);
   }
   else if (rt == rd)
   {
     or_reg64_reg64(rd, rs);
     not_reg64(rd);
   }
   else
   {
     mov_reg64_reg64(rd, rs);
     or_reg64_reg64(rd, rt);
     not_reg64(rd);
   }
#endif
}

void genslt(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[87]);
#endif
#ifdef INTERPRET_SLT
   gencallinterp((unsigned long long)cached_interpreter_table.SLT, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   cmp_reg64_reg64(rs, rt);
   setl_reg8(rd);
   and_reg64_imm8(rd, 1);
#endif
}

void gensltu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[88]);
#endif
#ifdef INTERPRET_SLTU
   gencallinterp((unsigned long long)cached_interpreter_table.SLTU, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   cmp_reg64_reg64(rs, rt);
   setb_reg8(rd);
   and_reg64_imm8(rd, 1);
#endif
}

void gendadd(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[89]);
#endif
#ifdef INTERPRET_DADD
   gencallinterp((unsigned long long)cached_interpreter_table.DADD, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   if (rs == rd)
     add_reg64_reg64(rd, rt);
   else if (rt == rd)
     add_reg64_reg64(rd, rs);
   else
     {
    mov_reg64_reg64(rd, rs);
    add_reg64_reg64(rd, rt);
     }
#endif
}

void gendaddu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[90]);
#endif
#ifdef INTERPRET_DADDU
   gencallinterp((unsigned long long)cached_interpreter_table.DADDU, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   if (rs == rd)
     add_reg64_reg64(rd, rt);
   else if (rt == rd)
     add_reg64_reg64(rd, rs);
   else
     {
    mov_reg64_reg64(rd, rs);
    add_reg64_reg64(rd, rt);
     }
#endif
}

void gendsub(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[91]);
#endif
#ifdef INTERPRET_DSUB
   gencallinterp((unsigned long long)cached_interpreter_table.DSUB, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   if (rs == rd)
     sub_reg64_reg64(rd, rt);
   else if (rt == rd)
   {
     neg_reg64(rd);
     add_reg64_reg64(rd, rs);
   }
   else
   {
     mov_reg64_reg64(rd, rs);
     sub_reg64_reg64(rd, rt);
   }
#endif
}

void gendsubu(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[92]);
#endif
#ifdef INTERPRET_DSUBU
   gencallinterp((unsigned long long)cached_interpreter_table.DSUBU, 0);
#else
   int rs = allocate_register_64((unsigned long long *)dst->f.r.rs);
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   if (rs == rd)
     sub_reg64_reg64(rd, rt);
   else if (rt == rd)
   {
     neg_reg64(rd);
     add_reg64_reg64(rd, rs);
   }
   else
   {
     mov_reg64_reg64(rd, rs);
     sub_reg64_reg64(rd, rt);
   }
#endif
}

void genteq(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[96]);
#endif
   gencallinterp((unsigned long long)cached_interpreter_table.TEQ, 0);
}

void gendsll(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[93]);
#endif
#ifdef INTERPRET_DSLL
   gencallinterp((unsigned long long)cached_interpreter_table.DSLL, 0);
#else
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   mov_reg64_reg64(rd, rt);
   shl_reg64_imm8(rd, dst->f.r.sa);
#endif
}

void gendsrl(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[94]);
#endif
#ifdef INTERPRET_DSRL
   gencallinterp((unsigned long long)cached_interpreter_table.DSRL, 0);
#else
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   mov_reg64_reg64(rd, rt);
   shr_reg64_imm8(rd, dst->f.r.sa);
#endif
}

void gendsra(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[95]);
#endif
#ifdef INTERPRET_DSRA
   gencallinterp((unsigned long long)cached_interpreter_table.DSRA, 0);
#else
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   mov_reg64_reg64(rd, rt);
   sar_reg64_imm8(rd, dst->f.r.sa);
#endif
}

void gendsll32(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[97]);
#endif
#ifdef INTERPRET_DSLL32
   gencallinterp((unsigned long long)cached_interpreter_table.DSLL32, 0);
#else
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);

   mov_reg64_reg64(rd, rt);
   shl_reg64_imm8(rd, dst->f.r.sa + 32);
#endif
}

void gendsrl32(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[98]);
#endif
#ifdef INTERPRET_DSRL32
   gencallinterp((unsigned long long)cached_interpreter_table.DSRL32, 0);
#else
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   mov_reg64_reg64(rd, rt);
   shr_reg64_imm8(rd, dst->f.r.sa + 32);
#endif
}

void gendsra32(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[99]);
#endif
#ifdef INTERPRET_DSRA32
   gencallinterp((unsigned long long)cached_interpreter_table.DSRA32, 0);
#else
   int rt = allocate_register_64((unsigned long long *)dst->f.r.rt);
   int rd = allocate_register_64_w((unsigned long long *)dst->f.r.rd);
   
   mov_reg64_reg64(rd, rt);
   sar_reg64_imm8(rd, dst->f.r.sa + 32);
#endif
}

