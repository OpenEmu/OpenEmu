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

#include "r4300/recomph.h"
#include "api/callbacks.h"

#include <stdlib.h>

extern long long int reg[32];

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

extern int branch_taken;

void jump_start_rel8(void);
void jump_end_rel8(void);
void jump_start_rel32(void);
void jump_end_rel32(void);
void add_jump(unsigned int pc_addr, unsigned int mi_addr);

static inline void put8(unsigned char octet)
{
   (*inst_pointer)[code_length] = octet;
   code_length++;
   if (code_length == max_code_length)
   {
     *inst_pointer = (unsigned char *) realloc_exec(*inst_pointer, max_code_length, max_code_length+8192);
     max_code_length += 8192;
   }
}

static inline void put32(unsigned int dword)
{
   if ((code_length+4) >= max_code_length)
   {
     *inst_pointer = (unsigned char *) realloc_exec(*inst_pointer, max_code_length, max_code_length+8192);
     max_code_length += 8192;
   }
   *((unsigned int *)(&(*inst_pointer)[code_length])) = dword;
   code_length+=4;
}

static inline void mov_eax_memoffs32(unsigned int *memoffs32)
{
   put8(0xA1);
   put32((unsigned int)(memoffs32));
}

static inline void mov_memoffs32_eax(unsigned int *memoffs32)
{
   put8(0xA3);
   put32((unsigned int)(memoffs32));
}

static inline void mov_m8_reg8(unsigned char *m8, int reg8)
{
   put8(0x88);
   put8((reg8 << 3) | 5);
   put32((unsigned int)(m8));
}

static inline void mov_reg16_m16(int reg16, unsigned short *m16)
{
   put8(0x66);
   put8(0x8B);
   put8((reg16 << 3) | 5);
   put32((unsigned int)(m16));
}

static inline void mov_m16_reg16(unsigned short *m16, int reg16)
{
   put8(0x66);
   put8(0x89);
   put8((reg16 << 3) | 5);
   put32((unsigned int)(m16));
}

static inline void cmp_reg32_m32(int reg32, unsigned int *m32)
{
   put8(0x3B);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m32));
}

static inline void cmp_reg32_reg32(int reg1, int reg2)
{
   put8(0x39);
   put8((reg2 << 3) | reg1 | 0xC0);
}

static inline void cmp_reg32_imm8(int reg32, unsigned char imm8)
{
   put8(0x83);
   put8(0xF8 + reg32);
   put8(imm8);
}

static inline void cmp_preg32pimm32_imm8(int reg32, unsigned int imm32, unsigned char imm8)
{
   put8(0x80);
   put8(0xB8 + reg32);
   put32(imm32);
   put8(imm8);
}

static inline void cmp_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xF8 + reg32);
   put32(imm32);
}

static inline void test_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0xF7);
   put8(0xC0 + reg32);
   put32(imm32);
}

static inline void test_m32_imm32(unsigned int *m32, unsigned int imm32)
{
   put8(0xF7);
   put8(0x05);
   put32((unsigned int)m32);
   put32(imm32);
}

static inline void add_m32_reg32(unsigned int *m32, int reg32)
{
   put8(0x01);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m32));
}

static inline void sub_reg32_m32(int reg32, unsigned int *m32)
{
   put8(0x2B);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m32));
}

static inline void sub_reg32_reg32(int reg1, int reg2)
{
   put8(0x29);
   put8((reg2 << 3) | reg1 | 0xC0);
}

static inline void sbb_reg32_reg32(int reg1, int reg2)
{
   put8(0x19);
   put8((reg2 << 3) | reg1 | 0xC0);
}

static inline void sub_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xE8 + reg32);
   put32(imm32);
}

static inline void sub_eax_imm32(unsigned int imm32)
{
   put8(0x2D);
   put32(imm32);
}

static inline void jne_rj(unsigned char saut)
{
   put8(0x75);
   put8(saut);
}

static inline void je_rj(unsigned char saut)
{
   put8(0x74);
   put8(saut);
}

static inline void jb_rj(unsigned char saut)
{
   put8(0x72);
   put8(saut);
}

static inline void jbe_rj(unsigned char saut)
{
   put8(0x76);
   put8(saut);
}

static inline void ja_rj(unsigned char saut)
{
   put8(0x77);
   put8(saut);
}

static inline void jae_rj(unsigned char saut)
{
   put8(0x73);
   put8(saut);
}

static inline void jle_rj(unsigned char saut)
{
   put8(0x7E);
   put8(saut);
}

static inline void jge_rj(unsigned char saut)
{
   put8(0x7D);
   put8(saut);
}

static inline void jg_rj(unsigned char saut)
{
   put8(0x7F);
   put8(saut);
}

static inline void jl_rj(unsigned char saut)
{
   put8(0x7C);
   put8(saut);
}

static inline void jp_rj(unsigned char saut)
{
   put8(0x7A);
   put8(saut);
}

static inline void je_near_rj(unsigned int saut)
{
   put8(0x0F);
   put8(0x84);
   put32(saut);
}

static inline void mov_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0xB8+reg32);
   put32(imm32);
}

static inline void jmp_imm_short(char saut)
{
   put8(0xEB);
   put8(saut);
}

static inline void or_m32_imm32(unsigned int *m32, unsigned int imm32)
{
   put8(0x81);
   put8(0x0D);
   put32((unsigned int)(m32));
   put32(imm32);
}

static inline void or_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x09);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void and_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x21);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void and_m32_imm32(unsigned int *m32, unsigned int imm32)
{
   put8(0x81);
   put8(0x25);
   put32((unsigned int)(m32));
   put32(imm32);
}

static inline void xor_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x31);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void sub_m32_imm32(unsigned int *m32, unsigned int imm32)
{
   put8(0x81);
   put8(0x2D);
   put32((unsigned int)(m32));
   put32(imm32);
}

static inline void add_reg32_imm32(unsigned int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xC0+reg32);
   put32(imm32);
}

static inline void inc_m32(unsigned int *m32)
{
   put8(0xFF);
   put8(0x05);
   put32((unsigned int)(m32));
}

static inline void cmp_m32_imm32(unsigned int *m32, unsigned int imm32)
{
   put8(0x81);
   put8(0x3D);
   put32((unsigned int)(m32));
   put32(imm32);
}

static inline void cmp_eax_imm32(unsigned int imm32)
{
   put8(0x3D);
   put32(imm32);
}

static inline void mov_m32_imm32(unsigned int *m32, unsigned int imm32)
{
   put8(0xC7);
   put8(0x05);
   put32((unsigned int)(m32));
   put32(imm32);
}

static inline void jmp(unsigned int mi_addr)
{
   put8(0xE9);
   put32(0);
   add_jump(code_length-4, mi_addr);
}

static inline void cdq(void)
{
   put8(0x99);
}

static inline void mov_m32_reg32(unsigned int *m32, unsigned int reg32)
{
   put8(0x89);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m32));
}

static inline void call_reg32(unsigned int reg32)
{
   put8(0xFF);
   put8(0xD0+reg32);
}

static inline void shr_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
   put8(0xC1);
   put8(0xE8+reg32);
   put8(imm8);
}

static inline void shr_reg32_cl(unsigned int reg32)
{
   put8(0xD3);
   put8(0xE8+reg32);
}

static inline void sar_reg32_cl(unsigned int reg32)
{
   put8(0xD3);
   put8(0xF8+reg32);
}

static inline void shl_reg32_cl(unsigned int reg32)
{
   put8(0xD3);
   put8(0xE0+reg32);
}

static inline void shld_reg32_reg32_cl(unsigned int reg1, unsigned int reg2)
{
   put8(0x0F);
   put8(0xA5);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void shld_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8)
{
   put8(0x0F);
   put8(0xA4);
   put8(0xC0 | (reg2 << 3) | reg1);
   put8(imm8);
}

static inline void shrd_reg32_reg32_cl(unsigned int reg1, unsigned int reg2)
{
   put8(0x0F);
   put8(0xAD);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void sar_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
   put8(0xC1);
   put8(0xF8+reg32);
   put8(imm8);
}

static inline void shrd_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8)
{
   put8(0x0F);
   put8(0xAC);
   put8(0xC0 | (reg2 << 3) | reg1);
   put8(imm8);
}

static inline void mul_m32(unsigned int *m32)
{
   put8(0xF7);
   put8(0x25);
   put32((unsigned int)(m32));
}

static inline void imul_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xE8+reg32);
}

static inline void mul_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xE0+reg32);
}

static inline void idiv_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xF8+reg32);
}

static inline void div_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xF0+reg32);
}

static inline void add_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x01);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void adc_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x11);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void add_reg32_m32(unsigned int reg32, unsigned int *m32)
{
   put8(0x03);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m32));
}

static inline void adc_reg32_imm32(unsigned int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xD0 + reg32);
   put32(imm32);
}

static inline void jmp_reg32(unsigned int reg32)
{
   put8(0xFF);
   put8(0xE0 + reg32);
}

static inline void mov_reg32_preg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x8B);
   put8((reg1 << 3) | reg2);
}

static inline void mov_preg32_reg32(int reg1, int reg2)
{
   put8(0x89);
   put8((reg2 << 3) | reg1);
}

static inline void mov_reg32_preg32preg32pimm32(int reg1, int reg2, int reg3, unsigned int imm32)
{
   put8(0x8B);
   put8((reg1 << 3) | 0x84);
   put8(reg2 | (reg3 << 3));
   put32(imm32);
}

static inline void mov_reg32_preg32pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x8B);
   put8(0x80 | (reg1 << 3) | reg2);
   put32(imm32);
}

static inline void mov_reg32_preg32x4pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x8B);
   put8((reg1 << 3) | 4);
   put8(0x80 | (reg2 << 3) | 5);
   put32(imm32);
}

static inline void mov_preg32pimm32_reg8(int reg32, unsigned int imm32, int reg8)
{
   put8(0x88);
   put8(0x80 | reg32 | (reg8 << 3));
   put32(imm32);
}

static inline void mov_preg32pimm32_imm8(int reg32, unsigned int imm32, unsigned char imm8)
{
   put8(0xC6);
   put8(0x80 + reg32);
   put32(imm32);
   put8(imm8);
}

static inline void mov_preg32pimm32_reg16(int reg32, unsigned int imm32, int reg16)
{
   put8(0x66);
   put8(0x89);
   put8(0x80 | reg32 | (reg16 << 3));
   put32(imm32);
}

static inline void mov_preg32pimm32_reg32(int reg1, unsigned int imm32, int reg2)
{
   put8(0x89);
   put8(0x80 | reg1 | (reg2 << 3));
   put32(imm32);
}

static inline void add_eax_imm32(unsigned int imm32)
{
   put8(0x05);
   put32(imm32);
}

static inline void shl_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
   put8(0xC1);
   put8(0xE0 + reg32);
   put8(imm8);
}

static inline void mov_reg32_m32(unsigned int reg32, unsigned int* m32)
{
   put8(0x8B);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m32));
}

static inline void mov_reg8_m8(int reg8, unsigned char *m8)
{
   put8(0x8A);
   put8((reg8 << 3) | 5);
   put32((unsigned int)(m8));
}

static inline void and_eax_imm32(unsigned int imm32)
{
   put8(0x25);
   put32(imm32);
}

static inline void and_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xE0 + reg32);
   put32(imm32);
}

static inline void or_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xC8 + reg32);
   put32(imm32);
}

static inline void xor_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xF0 + reg32);
   put32(imm32);
}

static inline void xor_reg8_imm8(int reg8, unsigned char imm8)
{
   put8(0x80);
   put8(0xF0 + reg8);
   put8(imm8);
}

static inline void mov_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   if (reg1 == reg2) return;
   put8(0x89);
   put8(0xC0 | (reg2 << 3) | reg1);
}

static inline void not_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xD0 + reg32);
}

static inline void movsx_reg32_m8(int reg32, unsigned char *m8)
{
   put8(0x0F);
   put8(0xBE);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m8));
}

static inline void movsx_reg32_8preg32pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x0F);
   put8(0xBE);
   put8((reg1 << 3) | reg2 | 0x80);
   put32(imm32);
}

static inline void movsx_reg32_16preg32pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x0F);
   put8(0xBF);
   put8((reg1 << 3) | reg2 | 0x80);
   put32(imm32);
}

static inline void movsx_reg32_m16(int reg32, unsigned short *m16)
{
   put8(0x0F);
   put8(0xBF);
   put8((reg32 << 3) | 5);
   put32((unsigned int)(m16));
}

static inline void fldcw_m16(unsigned short *m16)
{
   put8(0xD9);
   put8(0x2D);
   put32((unsigned int)(m16));
}

static inline void fld_preg32_dword(int reg32)
{
   put8(0xD9);
   put8(reg32);
}

static inline void fdiv_preg32_dword(int reg32)
{
   put8(0xD8);
   put8(0x30 + reg32);
}

static inline void fstp_preg32_dword(int reg32)
{
   put8(0xD9);
   put8(0x18 + reg32);
}

static inline void fchs(void)
{
   put8(0xD9);
   put8(0xE0);
}

static inline void fstp_preg32_qword(int reg32)
{
   put8(0xDD);
   put8(0x18 + reg32);
}

static inline void fadd_preg32_dword(int reg32)
{
   put8(0xD8);
   put8(reg32);
}

static inline void fsub_preg32_dword(int reg32)
{
   put8(0xD8);
   put8(0x20 + reg32);
}

static inline void fmul_preg32_dword(int reg32)
{
   put8(0xD8);
   put8(0x08 + reg32);
}

static inline void fistp_preg32_dword(int reg32)
{
   put8(0xDB);
   put8(0x18 + reg32);
}

static inline void fistp_preg32_qword(int reg32)
{
   put8(0xDF);
   put8(0x38 + reg32);
}

static inline void fld_preg32_qword(int reg32)
{
   put8(0xDD);
   put8(reg32);
}

static inline void fild_preg32_qword(int reg32)
{
   put8(0xDF);
   put8(0x28+reg32);
}

static inline void fild_preg32_dword(int reg32)
{
   put8(0xDB);
   put8(reg32);
}

static inline void fadd_preg32_qword(int reg32)
{
   put8(0xDC);
   put8(reg32);
}

static inline void fdiv_preg32_qword(int reg32)
{
   put8(0xDC);
   put8(0x30 + reg32);
}

static inline void fsub_preg32_qword(int reg32)
{
   put8(0xDC);
   put8(0x20 + reg32);
}

static inline void fmul_preg32_qword(int reg32)
{
   put8(0xDC);
   put8(0x08 + reg32);
}

static inline void fsqrt(void)
{
   put8(0xD9);
   put8(0xFA);
}

static inline void fabs_(void)
{
   put8(0xD9);
   put8(0xE1);
}

static inline void fcomip_fpreg(int fpreg)
{
   put8(0xDF);
   put8(0xF0 + fpreg);
}

static inline void fucomip_fpreg(int fpreg)
{
   put8(0xDF);
   put8(0xE8 + fpreg);
}

static inline void ffree_fpreg(int fpreg)
{
   put8(0xDD);
   put8(0xC0 + fpreg);
}

#endif // ASSEMBLE_H

