/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - assemble.c                                              *
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

#ifndef __ASSEMBLE_H__
#define __ASSEMBLE_H__

#define RAX 0
#define RCX 1
#define RDX 2
#define RBX 3
#define RSP 4
#define RBP 5
#define RSI 6
#define RDI 7

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
   unsigned char jump_wrapper[84];
   int need_cop1_check;
} reg_cache_struct;

extern int branch_taken;
extern int dynarec_stack_initialized;

void jump_start_rel8(void);
void jump_end_rel8(void);
void jump_start_rel32(void);
void jump_end_rel32(void);
void code_align16(void);

void mov_eax_memoffs32(unsigned int *memoffs32);
void mov_memoffs32_eax(unsigned int *memoffs32);
void mov_reg32_reg32(unsigned int reg1, unsigned int reg2);

void mov_xreg32_m32rel(unsigned int xreg32, unsigned int *m32);
void mov_m32rel_xreg32(unsigned int *m32, unsigned int xreg32);
void mov_xreg64_m64rel(unsigned int xreg64, unsigned long long* m64);
void mov_m64rel_xreg64(unsigned long long *m64, unsigned int xreg64);

void mov_rax_memoffs64(unsigned long long *memoffs64);
void mov_memoffs64_rax(unsigned long long *memoffs64);
void mov_reg64_reg64(unsigned int reg1, unsigned int reg2);

void sete_m8rel(unsigned char *m8);
void setne_m8rel(unsigned char *m8);
void setl_m8rel(unsigned char *m8);
void setle_m8rel(unsigned char *m8);
void setg_m8rel(unsigned char *m8);
void setge_m8rel(unsigned char *m8);
void setl_reg8(unsigned int reg8);
void setb_reg8(unsigned int reg8);

void int_imm8(unsigned char imm8);
void push_reg64(unsigned int reg64);
void pop_reg64(unsigned int reg64);
void cmp_xreg32_m32rel(int xreg32, unsigned int *m32);
void cmp_xreg64_m64rel(int xreg64, unsigned long long *m64);
void jne_rj(unsigned char saut);
void mov_reg32_imm32(int reg32, unsigned int imm32);
void mov_reg64_imm64(int reg64, unsigned long long imm64);
void jmp_imm(int saut);
void or_m32rel_imm32(unsigned int *m32, unsigned int imm32);
void add_m32rel_imm32(unsigned int *m32, unsigned int imm32);
void add_reg32_imm8(unsigned int reg32, unsigned char imm8);
void inc_m32rel(unsigned int *m32);
void cmp_m32rel_imm32(unsigned int *m32, unsigned int imm32);
void mov_m32rel_imm32(unsigned int *m32, unsigned int imm32);
void je_rj(unsigned char saut);
void jmp(unsigned int mi_addr);
void cdq(void);
void je_near(unsigned int mi_addr);
void jne_near(unsigned int mi_addr);
void jge_near(unsigned int mi_addr);
void jle_rj(unsigned char saut);
void jge_rj(unsigned char saut);
void ret(void);
void jle_near(unsigned int mi_addr);
void call_reg64(unsigned int reg64);
void jne_near_rj(unsigned int saut);
void jl_rj(unsigned char saut);
void sub_xreg32_m32rel(int xreg32, unsigned int *m32);
void mul_m32rel(unsigned int *m32);
void add_reg32_reg32(unsigned int reg1, unsigned int reg2);
void add_reg64_reg64(unsigned int reg1, unsigned int reg2);
void add_xreg32_m32rel(unsigned int xreg32, unsigned int *m32);
void add_xreg64_m64rel(unsigned int xreg64, unsigned long long *m64);
void add_reg32_imm32(unsigned int reg32, unsigned int imm32);
void add_reg64_imm32(unsigned int reg64, unsigned int imm32);
void jmp_reg64(unsigned int reg64);
void sub_reg32_imm32(int reg32, unsigned int imm32);
void sub_reg64_imm32(int reg64, unsigned int imm32);
void mov_reg32_preg64(unsigned int reg1, unsigned int reg2);
void sub_eax_imm32(unsigned int imm32);
void jl_near(unsigned int mi_addr);
void add_eax_imm32(unsigned int imm32);
void and_eax_imm32(unsigned int imm32);
void nop(void);
void cwde(void);
void jb_rj(unsigned char saut);
void ja_rj(unsigned char saut);
void jg_rj(unsigned char saut);
void and_ax_imm16(unsigned short imm16);
void or_ax_imm16(unsigned short imm16);
void xor_ax_imm16(unsigned short imm16);
void shrd_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8);
void or_eax_imm32(unsigned int imm32);
void or_m32rel_xreg32(unsigned int *m32, unsigned int xreg32);
void or_reg32_reg32(unsigned int reg1, unsigned int reg2);
void or_reg64_reg64(unsigned int reg1, unsigned int reg2);
void and_reg32_reg32(unsigned int reg1, unsigned int reg2);
void and_reg64_reg64(unsigned int reg1, unsigned int reg2);
void xor_reg32_reg32(unsigned int reg1, unsigned int reg2);
void xor_reg64_reg64(unsigned int reg1, unsigned int reg2);
void idiv_m32rel(unsigned int *m32);
void shl_reg32_imm8(unsigned int reg32, unsigned char imm8);
void shl_reg64_imm8(unsigned int reg64, unsigned char imm8);
void shl_reg32_cl(unsigned int reg32);
void shl_reg64_cl(unsigned int reg64);
void shr_reg32_imm8(unsigned int reg32, unsigned char imm8);
void shr_reg64_imm8(unsigned int reg64, unsigned char imm8);
void shr_reg32_cl(unsigned int reg32);
void shr_reg64_cl(unsigned int reg64);
void sar_reg32_imm8(unsigned int reg32, unsigned char imm8);
void sar_reg64_imm8(unsigned int reg64, unsigned char imm8);
void sar_reg32_cl(unsigned int reg32);
void sar_reg64_cl(unsigned int reg64);
void cmp_eax_imm32(unsigned int imm32);
void jg_near(unsigned int mi_addr);
void add_m32rel_xreg32(unsigned int *m32, int xreg32);
void je_near_rj(unsigned int saut);
void jge_near_rj(unsigned int saut);
void jl_near_rj(unsigned int saut);
void jle_near_rj(unsigned int saut);
void call_m64rel(unsigned long long *m64);
void and_xreg32_m32rel(unsigned int xreg32, unsigned int *m32);
void or_xreg32_m32rel(unsigned int xreg32, unsigned int *m32);
void not_reg32(unsigned int reg32);
void not_reg64(unsigned int reg64);
void neg_reg32(unsigned int reg32);
void neg_reg64(unsigned int reg64);
void xor_xreg32_m32rel(unsigned int xreg32, unsigned int *m32);
void sar_reg32_cl(unsigned int reg32);
void jmp_imm_short(char saut);
void jmp_m64rel(unsigned long long *m64);
void mov_reg32_preg64preg64pimm32(int reg1, int reg2, int reg3, unsigned int imm32);
void mov_preg64preg64pimm32_reg32(int reg1, int reg2, unsigned int imm32, int reg3);
void mov_reg64_preg64preg64pimm32(int reg1, int reg2, int reg3, unsigned int imm32);
void mov_reg32_preg64preg64(int reg1, int reg2, int reg3);
void mov_reg64_preg64preg64(int reg1, int reg2, int reg3);
void mov_reg32_preg64pimm32(int reg1, int reg2, unsigned int imm32);
void mov_reg64_preg64pimm32(int reg1, int reg2, unsigned int imm32);
void mov_reg64_preg64pimm8(int reg1, int reg2, unsigned int imm8);
void cmp_m32rel_imm8(unsigned int *m32, unsigned char imm8);
void mov_reg32_preg64x4pimm32(int reg1, int reg2, unsigned int imm32);
void mov_reg64_preg64x8preg64(int reg1, int reg2, int reg3);
void movsx_xreg32_m8rel(int xreg32, unsigned char *m8);
void sub_reg32_reg32(int reg1, int reg2);
void sub_reg64_reg64(int reg1, int reg2);
void cbw(void);
void or_reg32_imm32(int reg32, unsigned int imm32);
void or_reg64_imm32(int reg64, unsigned int imm32);
void and_reg32_imm32(int reg32, unsigned int imm32);
void and_reg64_imm32(int reg64, unsigned int imm32);
void and_reg32_imm8(int reg32, unsigned char imm8);
void and_reg64_imm8(int reg64, unsigned char imm8);
void movsx_reg32_reg16(int reg32, int reg16);
void movsx_xreg32_m16rel(int xreg32, unsigned short *m16);
void movsxd_reg64_reg32(int reg64, int reg32);
void movsxd_xreg64_m32rel(int xreg64, unsigned int *m32);
void cmp_reg32_imm8(int reg32, unsigned char imm8);
void cmp_reg64_imm8(int reg64, unsigned char imm8);
void cmp_reg32_imm32(int reg32, unsigned int imm32);
void cmp_reg64_imm32(int reg64, unsigned int imm32);
void add_m32rel_imm8(unsigned int *m32, unsigned char imm8);
void mov_xreg8_m8rel(int xreg8, unsigned char *m8);
void mov_preg64pimm32_reg16(int reg64, unsigned int imm32, int reg16);
void mov_preg64preg64_reg16(int reg1, int reg2, int reg16);
void mov_preg64preg64_reg32(int reg1, int reg2, int reg32);
void mov_preg64pimm32_reg32(int reg1, unsigned int imm32, int reg2);
void mov_preg64pimm8_reg64(int reg1, unsigned int imm8, int reg2);
void fld_preg64_dword(int reg64);
void fdiv_preg64_dword(int reg64);
void fstp_preg64_dword(int reg64);
void mov_preg64_reg32(int reg1, int reg2);
void mov_preg64_reg64(int reg1, int reg2);
void mov_reg64_preg64(int reg1, int reg2);
void fchs(void);
void fstp_preg64_qword(int reg64);
void fadd_preg64_dword(int reg64);
void fmul_preg64_dword(int reg64);
void fcomp_preg64_dword(int reg64);
void and_m32rel_imm32(unsigned int *m32, unsigned int imm32);
void fistp_m32rel(unsigned int *m32);
void fistp_m64rel(unsigned long long *m64);
void div_m32rel(unsigned int *m32);
void mov_xreg16_m16rel(int xreg16, unsigned short *m16);
void mov_m16rel_xreg16(unsigned short *m16, int xreg16);
void fld_preg64_qword(int reg64);
void fadd_preg64_qword(int reg64);
void fdiv_preg64_qword(int reg64);
void fsub_preg64_dword(int reg64);
void xor_reg32_imm32(int reg32, unsigned int imm32);
void xor_reg64_imm32(int reg64, unsigned int imm32);
void xor_al_imm8(unsigned char imm8);
void mov_preg64preg64_reg8(int reg1, int reg2, int reg8);
void mov_preg64preg64_imm8(int reg1, int reg2, unsigned char imm8);
void xor_reg8_imm8(int reg8, unsigned char imm8);
void cmp_m8rel_imm8(unsigned char *m8, unsigned char imm8);
void fsub_preg64_qword(int reg64);
void fmul_preg64_qword(int reg64);
void adc_xreg32_m32rel(unsigned int xreg32, unsigned int *m32);
void sub_m32rel_imm32(unsigned int *m32, unsigned int imm32);
void sub_m64rel_imm32(unsigned long long *m64, unsigned int imm32);
void jbe_rj(unsigned char saut);
void cmp_reg32_reg32(int reg1, int reg2);
void cmp_reg64_reg64(int reg1, int reg2);
void adc_reg32_imm32(unsigned int reg32, unsigned int imm32);
void and_al_imm8(unsigned char imm8);
void cmp_al_imm8(unsigned char imm8);
void movsx_reg32_8preg64pimm32(int reg1, int reg2, unsigned int imm32);
void movsx_reg32_8preg64preg64(int reg1, int reg2, int reg3);
void movsx_reg32_16preg64pimm32(int reg1, int reg2, unsigned int imm32);
void movsx_reg32_16preg64preg64(int reg1, int reg2, int reg3);
void mov_preg64pimm32_imm8(int reg64, unsigned int imm32, unsigned char imm8);
void mov_m8rel_xreg8(unsigned char *m8, int xreg8);
void shld_reg32_reg32_cl(unsigned int reg1, unsigned int reg2);
void test_reg32_imm32(int reg32, unsigned int imm32);
void shrd_reg32_reg32_cl(unsigned int reg1, unsigned int reg2);
void imul_reg32(unsigned int reg32);
void mul_reg32(unsigned int reg32);
void mul_reg64(unsigned int reg64);
void idiv_reg32(unsigned int reg32);
void div_reg32(unsigned int reg32);
void adc_reg32_reg32(unsigned int reg1, unsigned int reg2);
void sbb_reg32_reg32(int reg1, int reg2);
void shld_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8);
void cmp_preg64pimm32_imm8(int reg64, unsigned int imm32, unsigned char imm8);
void cmp_preg64preg64_imm8(int reg1, int reg2, unsigned char imm8);
void test_m32rel_imm32(unsigned int *m32, unsigned int imm32);
void fldcw_m16rel(unsigned short* m16);
void fsqrt(void);
void fabs_(void);
void fistp_preg64_qword(int reg64);
void fistp_preg64_dword(int reg64);
void fcomip_fpreg(int fpreg);
void fucomip_fpreg(int fpreg);
void ffree_fpreg(int fpreg);
void jp_rj(unsigned char saut);
void jae_rj(unsigned char saut);
void fild_preg64_qword(int reg64);
void fild_preg64_dword(int reg64);

#endif /* __ASSEMBLE_H__ */

