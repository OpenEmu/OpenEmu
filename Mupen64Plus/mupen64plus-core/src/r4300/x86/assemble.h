/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - assemble.h                                              *
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

#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7

#define AX 0
#define CX 1
#define DX 2
#define BX 3
#define SP 4
#define BP 5
#define SI 6
#define DI 7

#define AL 0
#define CL 1
#define DL 2
#define BL 3
#define AH 4
#define CH 5
#define DH 6
#define BH 7

typedef struct _reg_cache_struct
{
   int need_map;
   void *needed_registers[8];
   unsigned char jump_wrapper[62];
   int need_cop1_check;
} reg_cache_struct;

extern int branch_taken;

void jump_start_rel8(void);
void jump_end_rel8(void);
void jump_start_rel32(void);
void jump_end_rel32(void);

void push_reg32(int reg32);
void mov_eax_memoffs32(unsigned int *memoffs32);
void cmp_reg32_m32(int reg32, unsigned int *m32);
void jne_rj(unsigned char saut);
void mov_reg32_imm32(int reg32, unsigned int imm32);
void jmp_imm(int saut);
void dec_reg32(int reg32);
void mov_memoffs32_eax(unsigned int *memoffs32);
void or_m32_imm32(unsigned int *m32, unsigned int imm32);
void pop_reg32(int reg32);
void add_m32_imm32(unsigned int *m32, unsigned int imm32);
void inc_reg32(int reg32);
void push_imm32(unsigned int imm32);
void add_reg32_imm8(unsigned int reg32, unsigned char imm8);
void inc_m32(unsigned int *m32);
void cmp_m32_imm32(unsigned int *m32, unsigned int imm32);
void mov_m32_imm32(unsigned int *m32, unsigned int imm32);
void je_rj(unsigned char saut);
void jmp(unsigned int mi_addr);
void cdq(void);
void mov_m32_reg32(unsigned int *m32, unsigned int reg32);
void je_near(unsigned int mi_addr);
void jne_near(unsigned int mi_addr);
void jge_near(unsigned int mi_addr);
void jle_rj(unsigned char saut);
void jge_rj(unsigned char saut);
void ret(void);
void jle_near(unsigned int mi_addr);
void call_reg32(unsigned int reg32);
void jne_near_rj(unsigned int saut);
void jl_rj(unsigned char saut);
void sub_reg32_m32(int reg32, unsigned int *m32);
void shr_reg32_imm8(unsigned int reg32, unsigned char imm8);
void mul_m32(unsigned int *m32);
void add_reg32_reg32(unsigned int reg1, unsigned int reg2);
void add_reg32_m32(unsigned int reg32, unsigned int *m32);
void jmp_reg32(unsigned int reg32);
void sub_reg32_imm32(int reg32, unsigned int imm32);
void mov_reg32_preg32(unsigned int reg1, unsigned int reg2);
void sub_eax_imm32(unsigned int imm32);
void add_reg32_imm32(unsigned int reg32, unsigned int imm32);
void jl_near(unsigned int mi_addr);
void add_eax_imm32(unsigned int imm32);
void shl_reg32_imm8(unsigned int reg32, unsigned char imm8);
void mov_reg32_m32(unsigned int reg32, unsigned int *m32);
void and_eax_imm32(unsigned int imm32);
void mov_al_memoffs8(unsigned char *memoffs8);
void mov_memoffs8_al(unsigned char *memoffs8);
void nop(void);
void mov_ax_memoffs16(unsigned short *memoffs16);
void mov_memoffs16_ax(unsigned short *memoffs16);
void cwde(void);
void jb_rj(unsigned char saut);
void ja_rj(unsigned char saut);
void jg_rj(unsigned char saut);
void and_ax_imm16(unsigned short imm16);
void or_ax_imm16(unsigned short imm16);
void xor_ax_imm16(unsigned short imm16);
void shrd_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8);
void or_eax_imm32(unsigned int imm32);
void or_m32_reg32(unsigned int *m32, unsigned int reg32);
void or_reg32_reg32(unsigned int reg1, unsigned int reg2);
void sar_reg32_imm8(unsigned int reg32, unsigned char imm8);
void and_reg32_reg32(unsigned int reg1, unsigned int reg2);
void xor_reg32_reg32(unsigned int reg1, unsigned int reg2);
void imul_m32(unsigned int *m32);
void mov_reg32_reg32(unsigned int reg1, unsigned int reg2);
void idiv_m32(unsigned int *m32);
void shr_reg32_cl(unsigned int reg32);
void shl_reg32_cl(unsigned int reg32);
void cmp_eax_imm32(unsigned int imm32);
void jg_near(unsigned int mi_addr);
void add_m32_reg32(unsigned int *m32, int reg32);
void je_near_rj(unsigned int saut);
void jge_near_rj(unsigned int saut);
void jl_near_rj(unsigned int saut);
void jle_near_rj(unsigned int saut);
void call_m32(unsigned int *m32);
void and_reg32_m32(unsigned int reg32, unsigned int *m32);
void or_reg32_m32(unsigned int reg32, unsigned int *m32);
void not_reg32(unsigned int reg32);
void xor_reg32_m32(unsigned int reg32, unsigned int *m32);
void sar_reg32_cl(unsigned int reg32);
void jmp_imm_short(char saut);
void jmp_m32(unsigned int *m32);
void mov_reg32_preg32preg32pimm32(int reg1, int reg2, int reg3, unsigned int imm32);
void mov_reg32_preg32pimm32(int reg1, int reg2, unsigned int imm32);
void cmp_m32_imm8(unsigned int *m32, unsigned char imm8);
void mov_reg32_preg32x4preg32(int reg1, int reg2, int reg3);
void mov_reg32_preg32x4preg32pimm32(int reg1, int reg2, int reg3, unsigned int imm32);
void mov_reg32_preg32x4pimm32(int reg1, int reg2, unsigned int imm32);
void and_reg32_imm32(int reg32, unsigned int imm32);
void movsx_reg32_m8(int reg32, unsigned char *m8);
void sub_reg32_reg32(int reg1, int reg2);
void cbw(void);
void movsx_reg32_reg8(int reg32, int reg8);
void and_reg32_imm8(int reg32, unsigned char imm8);
void movsx_reg32_reg16(int reg32, int reg8);
void movsx_reg32_m16(int reg32, unsigned short *m16);
void cmp_reg32_imm8(int reg32, unsigned char imm8);
void add_m32_imm8(unsigned int *m32, unsigned char imm8);
void mov_reg8_m8(int reg8, unsigned char *m8);
void mov_preg32preg32pimm32_reg8(int reg1, int reg2, unsigned int imm32, int reg8);
void mov_preg32pimm32_reg16(int reg32, unsigned int imm32, int reg16);
void cmp_reg32_imm32(int reg32, unsigned int imm32);
void mov_preg32pimm32_reg32(int reg1, unsigned int imm32, int reg2);
void fld_preg32_dword(int reg32);
void fdiv_preg32_dword(int reg32);
void fstp_preg32_dword(int reg32);
void mov_preg32_reg32(int reg1, int reg2);
void fchs(void);
void fstp_preg32_qword(int reg32);
void fadd_preg32_dword(int reg32);
void fmul_preg32_dword(int reg32);
void fcomp_preg32_dword(int reg32);
void and_m32_imm32(unsigned int *m32, unsigned int imm32);
void fistp_m32(unsigned int *m32);
void fistp_m64(unsigned long long *m64);
void div_m32(unsigned int *m32);
void mov_m8_imm8(unsigned char *m8, unsigned char imm8);
void mov_reg16_m16(int reg16, unsigned short *m16);
void mov_m16_reg16(unsigned short *m16, int reg16);
void fld_preg32_qword(int reg32);
void fadd_preg32_qword(int reg32);
void fdiv_preg32_qword(int reg32);
void fsub_preg32_dword(int reg32);
void xor_reg32_imm32(int reg32, unsigned int imm32);
void xor_al_imm8(unsigned char imm8);
void mov_preg32pimm32_reg8(int reg32, unsigned int imm32, int reg8);
void xor_reg8_imm8(int reg8, unsigned char imm8);
void cmp_m8_imm8(unsigned char *m8, unsigned char imm8);
void fsub_preg32_qword(int reg32);
void fmul_preg32_qword(int reg32);
void adc_reg32_m32(unsigned int reg32, unsigned int *m32);
void sub_m32_imm32(unsigned int *m32, unsigned int imm32);
void jbe_rj(unsigned char saut);
void cmp_reg32_reg32(int reg1, int reg2);
void or_reg32_imm32(int reg32, unsigned int imm32);
void adc_reg32_imm32(unsigned int reg32, unsigned int imm32);
void and_al_imm8(unsigned char imm8);
void cmp_al_imm8(unsigned char imm8);
void movsx_reg32_8preg32pimm32(int reg1, int reg2, unsigned int imm32);
void movsx_reg32_16preg32pimm32(int reg1, int reg2, unsigned int imm32);
void mov_preg32pimm32_imm8(int reg32, unsigned int imm32, unsigned char imm8);
void mov_m8_reg8(unsigned char *m8, int reg8);
void shld_reg32_reg32_cl(unsigned int reg1, unsigned int reg2);
void test_reg32_imm32(int reg32, unsigned int imm32);
void shrd_reg32_reg32_cl(unsigned int reg1, unsigned int reg2);
void imul_reg32(unsigned int reg32);
void mul_reg32(unsigned int reg32);
void idiv_reg32(unsigned int reg32);
void div_reg32(unsigned int reg32);
void adc_reg32_reg32(unsigned int reg1, unsigned int reg2);
void sbb_reg32_reg32(int reg1, int reg2);
void shld_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8);
void cmp_preg32pimm32_imm8(int reg32, unsigned int imm32, unsigned char imm8);
void test_m32_imm32(unsigned int *m32, unsigned int imm32);
void fldcw_m16(unsigned short* m16);
void fsqrt(void);
void fabs_(void);
void fistp_preg32_qword(int reg32);
void fistp_preg32_dword(int reg32);
void fcomip_fpreg(int fpreg);
void fucomip_fpreg(int fpreg);
void ffree_fpreg(int fpreg);
void jp_rj(unsigned char saut);
void jae_rj(unsigned char saut);
void fild_preg32_qword(int reg32);
void fild_preg32_dword(int reg32);

#endif // ASSEMBLE_H

