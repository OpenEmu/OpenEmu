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

#include <stdlib.h>
#include <stdio.h>

#include "assemble.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "r4300/recomph.h"
#include "r4300/recomp.h"
#include "r4300/r4300.h"

/* Placeholder for RIP-relative offsets is maxmimum 32-bit signed value.
 * So, if recompiled code is run without running passe2() first, it will
 * cause an exception.
*/
#define REL_PLACEHOLDER 0x7fffffff

typedef struct _jump_table
{
  unsigned int mi_addr;
  unsigned int pc_addr;
  unsigned int absolute64;
} jump_table;

static jump_table *jumps_table = NULL;
static int jumps_number = 0, max_jumps_number = 0;

typedef struct _riprelative_table
{
  unsigned int   pc_addr;     /* index in bytes from start of x86_64 code block to the displacement value to write */
  unsigned int   extra_bytes; /* number of remaining instruction bytes (immediate data) after 4-byte displacement */
  unsigned char *global_dst;  /* 64-bit pointer to the data object */
} riprelative_table;

static riprelative_table *riprel_table = NULL;
static int riprel_number = 0, max_riprel_number = 0;

/* Static Functions */

static void add_jump(unsigned int pc_addr, unsigned int mi_addr, unsigned int absolute64)
{
  if (jumps_number == max_jumps_number)
  {
    max_jumps_number += 512;
    jumps_table = realloc(jumps_table, max_jumps_number*sizeof(jump_table));
  }
  jumps_table[jumps_number].pc_addr = pc_addr;
  jumps_table[jumps_number].mi_addr = mi_addr;
  jumps_table[jumps_number].absolute64 = absolute64;
  jumps_number++;
}

/* Global Functions */

void init_assembler(void *block_jumps_table, int block_jumps_number, void *block_riprel_table, int block_riprel_number)
{
  if (block_jumps_table)
  {
    jumps_table = block_jumps_table;
    jumps_number = block_jumps_number;
    if (jumps_number <= 512)
      max_jumps_number = 512;
    else
      max_jumps_number = (jumps_number + 511) & 0xfffffe00;
  }
  else
  {
    jumps_table = malloc(512*sizeof(jump_table));
    jumps_number = 0;
    max_jumps_number = 512;
  }

  if (block_riprel_table)
  {
    riprel_table = block_riprel_table;
    riprel_number = block_riprel_number;
    if (riprel_number <= 512)
      max_riprel_number = 512;
    else
      max_riprel_number = (riprel_number + 511) & 0xfffffe00;
  }
  else
  {
    riprel_table = malloc(512 * sizeof(riprelative_table));
    riprel_number = 0;
    max_riprel_number = 512;
  }
}

void free_assembler(void **block_jumps_table, int *block_jumps_number, void **block_riprel_table, int *block_riprel_number)
{
  *block_jumps_table = jumps_table;
  *block_jumps_number = jumps_number;
  *block_riprel_table = riprel_table;
  *block_riprel_number = riprel_number;
}

void passe2(precomp_instr *dest, int start, int end, precomp_block *block)
{
  unsigned int i;

  build_wrappers(dest, start, end, block);

  /* First, fix up all the jumps.  This involves a table lookup to find the offset into the block of x86_64 code for
   * for start of a recompiled r4300i instruction corresponding to the given jump destination address in the N64
   * address space.  Next, the relative offset between this destination and the location of the jump instruction is
   * computed and stored in memory, so that the jump will branch to the right place in the recompiled code.
   */
  for (i = 0; i < jumps_number; i++)
  {
    precomp_instr *jump_instr = dest + ((jumps_table[i].mi_addr - dest[0].addr) / 4);
    unsigned int   jmp_offset_loc = jumps_table[i].pc_addr;
    unsigned char *addr_dest = NULL;
    /* calculate the destination address to jump to */
    if (jump_instr->reg_cache_infos.need_map)
    {
      addr_dest = jump_instr->reg_cache_infos.jump_wrapper;
    }
    else
    {
      addr_dest = block->code + jump_instr->local_addr;
    }
    /* write either a 32-bit IP-relative offset or a 64-bit absolute address */
    if (jumps_table[i].absolute64)
    {
      *((unsigned long long *) (block->code + jmp_offset_loc)) = (unsigned long long) addr_dest;
    }
    else
    {
      long jump_rel_offset = (long) (addr_dest - (block->code + jmp_offset_loc + 4));
      *((int *) (block->code + jmp_offset_loc)) = (int) jump_rel_offset;
      if (jump_rel_offset >= 0x7fffffffLL || jump_rel_offset < -0x80000000LL)
      {
        DebugMessage(M64MSG_ERROR, "assembler pass2 error: offset too big for relative jump from %lx to %lx", 
               (long) (block->code + jmp_offset_loc + 4), (long) addr_dest);
        asm(" int $3; ");
      }
    }
  }

  /* Next, fix up all of the RIP-relative memory accesses.  This is unique to the x86_64 architecture, because
   * the 32-bit absolute displacement addressing mode is not available (and there's no 64-bit absolute displacement
   * mode either).
   */
  for (i = 0; i < riprel_number; i++)
  {
    unsigned char *rel_offset_ptr = block->code + riprel_table[i].pc_addr;
    long rip_rel_offset = (long) (riprel_table[i].global_dst - (rel_offset_ptr + 4 + riprel_table[i].extra_bytes));
    if (rip_rel_offset >= 0x7fffffffLL || rip_rel_offset < -0x80000000LL)
    {
      DebugMessage(M64MSG_ERROR, "assembler pass2 error: offset too big between mem target: %lx and code position: %lx",
             (long) riprel_table[i].global_dst, (long) rel_offset_ptr);
      asm(" int $3; ");
    }
    *((int *) rel_offset_ptr) = (int) rip_rel_offset;
  }

}

static void put8(unsigned char octet)
{
  (*inst_pointer)[code_length] = octet;
  code_length++;
  if (code_length == max_code_length)
  {
    *inst_pointer = realloc_exec(*inst_pointer, max_code_length, max_code_length+8192);
    max_code_length += 8192;
  }
}

static void put16(unsigned short word)
{
  if ((code_length + 2) >= max_code_length)
  {
    *inst_pointer = realloc_exec(*inst_pointer, max_code_length, max_code_length+8192);
    max_code_length += 8192;
  }
  *((unsigned short *) (*inst_pointer + code_length)) = word;
  code_length += 2;
}

static void put32(unsigned int dword)
{
  if ((code_length + 4) >= max_code_length)
  {
    *inst_pointer = realloc_exec(*inst_pointer, max_code_length, max_code_length+8192);
    max_code_length += 8192;
  }
  *((unsigned int *) (*inst_pointer + code_length)) = dword;
  code_length += 4;
}

static void put64(unsigned long long qword)
{
  if ((code_length + 8) >= max_code_length)
  {
    *inst_pointer = realloc_exec(*inst_pointer, max_code_length, max_code_length+8192);
    max_code_length += 8192;
  }
  *((unsigned long long *) (*inst_pointer + code_length)) = qword;
  code_length += 8;
}

static int rel_r15_offset(void *dest, const char *op_name)
{
    /* calculate the destination pointer's offset from the base of the r4300 registers */
    long long rel_offset = (long long) ((unsigned char *) dest - (unsigned char *) reg);

    if (llabs(rel_offset) > 0x7fffffff)
    {
        DebugMessage(M64MSG_ERROR, "Error: destination %lx more than 2GB away from r15 base %lx in %s()", (long) dest, (long) reg, op_name);
        asm(" int $3; ");
    }

    return (int) rel_offset;
}

void code_align16(void)
{
  int bytes = 0;

  if (((long) (*inst_pointer) & 15) != 0)
  {
    DebugMessage(M64MSG_ERROR, "Error: code block pointer is not 16-byte aligned!");
    asm(" int $3; ");
  }

  bytes = code_length & 15;
  if (bytes != 0) bytes = 16 - bytes;
  while (bytes > 0) /* this nop-generator was coded from information given in AMD64 optimization manual */
  {
    if (bytes == 4 || bytes > 9)
    { bytes -= 4; put32(0x90666666); }
    else if (bytes == 1)
    { bytes -= 1; put8(0x90); }
    else if (bytes == 2)
    { bytes -= 2; put16(0x9066); }
    else if (bytes == 3)
    { bytes -= 4; put16(0x6666); put8(0x90); }
    else if (bytes == 5)
    { bytes -= 5; put32(0x66906666); put8(0x90); }
    else if (bytes == 6)
    { bytes -= 6; put32(0x66906666); put16(0x9066); }
    else if (bytes == 7)
    { bytes -= 7; put32(0x90666666); put16(0x6666); put8(0x90); }
    else if (bytes == 8)
    { bytes -= 8; put32(0x90666666); put32(0x90666666); }
    else if (bytes == 9)
    { bytes -= 9; put32(0x66906666); put32(0x66669066); put8(0x90); }
  }

}

static unsigned int g_jump_start8 = 0;
static unsigned int g_jump_start32 = 0;

void jump_start_rel8(void)
{
  g_jump_start8 = code_length;
}

void jump_start_rel32(void)
{
  g_jump_start32 = code_length;
}

void jump_end_rel8(void)
{
  unsigned int jump_end = code_length;
  int jump_vec = jump_end - g_jump_start8;

  if (jump_vec > 127 || jump_vec < -128)
  {
    DebugMessage(M64MSG_ERROR, "Error: 8-bit relative jump too long! From %x to %x", g_jump_start8, jump_end);
    asm(" int $3; ");
  }

  code_length = g_jump_start8 - 1;
  put8(jump_vec);
  code_length = jump_end;
}

void jump_end_rel32(void)
{
  unsigned int jump_end = code_length;
  int jump_vec = jump_end - g_jump_start32;

  code_length = g_jump_start32 - 4;
  put32(jump_vec);
  code_length = jump_end;
}

void int_imm8(unsigned char imm8)
{
   put8(0xCD);
   put8(imm8);
}

void mov_eax_memoffs32(unsigned int *memoffs32)
{
   put8(0xA1);
   put64((unsigned long long) memoffs32);
}

void mov_memoffs32_eax(unsigned int *memoffs32)
{
   put8(0xA3);
   put64((unsigned long long) memoffs32);
}

void mov_rax_memoffs64(unsigned long long *memoffs64)
{
   put8(0x48);
   put8(0xA1);
   put64((unsigned long long) memoffs64);
}

void mov_memoffs64_rax(unsigned long long *memoffs64)
{
   put8(0x48);
   put8(0xA3);
   put64((unsigned long long) memoffs64);
}

void mov_m8rel_xreg8(unsigned char *m8, int xreg8)
{
   int offset = rel_r15_offset(m8, "mov_m8rel_xreg8");

   put8(0x41 | ((xreg8 & 8) >> 1));
   put8(0x88);
   put8(0x87 | ((xreg8 & 7) << 3));
   put32(offset);
}

void mov_xreg16_m16rel(int xreg16, unsigned short *m16)
{
   int offset = rel_r15_offset(m16, "mov_xreg16_m16rel");

   put8(0x66);
   put8(0x41 | ((xreg16 & 8) >> 1));
   put8(0x8B);
   put8(0x87 | ((xreg16 & 7) << 3));
   put32(offset);
}

void mov_m16rel_xreg16(unsigned short *m16, int xreg16)
{
   int offset = rel_r15_offset(m16, "mov_m16rel_xreg16");

   put8(0x66);
   put8(0x41 | ((xreg16 & 8) >> 1));
   put8(0x89);
   put8(0x87 | ((xreg16 & 7) << 3));
   put32(offset);
}

void cmp_xreg32_m32rel(int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "cmp_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x3B);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void cmp_xreg64_m64rel(int xreg64, unsigned long long *m64)
{
   int offset = rel_r15_offset(m64, "cmp_xreg64_m64rel");

   put8(0x49 | ((xreg64 & 8) >> 1));
   put8(0x3B);
   put8(0x87 | ((xreg64 & 7) << 3));
   put32(offset);
}

void cmp_reg32_reg32(int reg1, int reg2)
{
   put8(0x39);
   put8((reg2 << 3) | reg1 | 0xC0);
}

void cmp_reg64_reg64(int reg1, int reg2)
{
   put8(0x48);
   put8(0x39);
   put8((reg2 << 3) | reg1 | 0xC0);
}

void cmp_reg32_imm8(int reg32, unsigned char imm8)
{
   put8(0x83);
   put8(0xF8 + reg32);
   put8(imm8);
}

void cmp_reg64_imm8(int reg64, unsigned char imm8)
{
   put8(0x48);
   put8(0x83);
   put8(0xF8 + reg64);
   put8(imm8);
}

void cmp_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xF8 + reg32);
   put32(imm32);
}

void cmp_reg64_imm32(int reg64, unsigned int imm32)
{
   put8(0x48);
   put8(0x81);
   put8(0xF8 + reg64);
   put32(imm32);
}

void cmp_preg64pimm32_imm8(int reg64, unsigned int imm32, unsigned char imm8)
{
   put8(0x80);
   put8(0xB8 + reg64);
   put32(imm32);
   put8(imm8);
}

void cmp_preg64preg64_imm8(int reg1, int reg2, unsigned char imm8)
{
   put8(0x80);
   put8(0x3C);
   put8((reg1 << 3) | reg2);
   put8(imm8);
}

void sete_m8rel(unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "sete_m8rel");

   put8(0x41);
   put8(0x0F);
   put8(0x94);
   put8(0x87);
   put32(offset);
}

void setne_m8rel(unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "setne_m8rel");

   put8(0x41);
   put8(0x0F);
   put8(0x95);
   put8(0x87);
   put32(offset);
}

void setl_m8rel(unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "setl_m8rel");

   put8(0x41);
   put8(0x0F);
   put8(0x9C);
   put8(0x87);
   put32(offset);
}

void setle_m8rel(unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "setle_m8rel");

   put8(0x41);
   put8(0x0F);
   put8(0x9E);
   put8(0x87);
   put32(offset);
}

void setg_m8rel(unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "setg_m8rel");

   put8(0x41);
   put8(0x0F);
   put8(0x9F);
   put8(0x87);
   put32(offset);
}

void setge_m8rel(unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "setge_m8rel");

   put8(0x41);
   put8(0x0F);
   put8(0x9D);
   put8(0x87);
   put32(offset);
}

void setl_reg8(unsigned int reg8)
{
   put8(0x40);  /* we need an REX prefix to use the uniform byte registers */
   put8(0x0F);
   put8(0x9C);
   put8(0xC0 | reg8);
}

void setb_reg8(unsigned int reg8)
{
   put8(0x40);  /* we need an REX prefix to use the uniform byte registers */
   put8(0x0F);
   put8(0x92);
   put8(0xC0 | reg8);
}

void push_reg64(unsigned int reg64)
{
   put8(0x50 + reg64);
}

void pop_reg64(unsigned int reg64)
{
   put8(0x58 + reg64);
}

void test_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0xF7);
   put8(0xC0 + reg32);
   put32(imm32);
}

void test_m32rel_imm32(unsigned int *m32, unsigned int imm32)
{
   int offset = rel_r15_offset(m32, "test_m32rel_imm32");

   put8(0x41);
   put8(0xF7);
   put8(0x87);
   put32(offset);
   put32(imm32);
}

void cmp_al_imm8(unsigned char imm8)
{
   put8(0x3C);
   put8(imm8);
}

void add_m32rel_xreg32(unsigned int *m32, int xreg32)
{
   int offset = rel_r15_offset(m32, "add_m32rel_xreg32");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x01);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void sub_xreg32_m32rel(int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "sub_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x2B);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void sub_reg32_reg32(int reg1, int reg2)
{
   put8(0x29);
   put8((reg2 << 3) | reg1 | 0xC0);
}

void sub_reg64_reg64(int reg1, int reg2)
{
   put8(0x48);
   put8(0x29);
   put8((reg2 << 3) | reg1 | 0xC0);
}

void sbb_reg32_reg32(int reg1, int reg2)
{
   put8(0x19);
   put8((reg2 << 3) | reg1 | 0xC0);
}

void sub_reg64_imm32(int reg64, unsigned int imm32)
{
   put8(0x48);
   put8(0x81);
   put8(0xE8 + reg64);
   put32(imm32);
}

void sub_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xE8 + reg32);
   put32(imm32);
}

void sub_eax_imm32(unsigned int imm32)
{
   put8(0x2D);
   put32(imm32);
}

void jne_rj(unsigned char saut)
{
   put8(0x75);
   put8(saut);
}

void je_rj(unsigned char saut)
{
   put8(0x74);
   put8(saut);
}

void jb_rj(unsigned char saut)
{
   put8(0x72);
   put8(saut);
}

void jbe_rj(unsigned char saut)
{
   put8(0x76);
   put8(saut);
}

void ja_rj(unsigned char saut)
{
   put8(0x77);
   put8(saut);
}

void jae_rj(unsigned char saut)
{
   put8(0x73);
   put8(saut);
}

void jle_rj(unsigned char saut)
{
   put8(0x7E);
   put8(saut);
}

void jge_rj(unsigned char saut)
{
   put8(0x7D);
   put8(saut);
}

void jg_rj(unsigned char saut)
{
   put8(0x7F);
   put8(saut);
}

void jl_rj(unsigned char saut)
{
   put8(0x7C);
   put8(saut);
}

void jp_rj(unsigned char saut)
{
   put8(0x7A);
   put8(saut);
}

void je_near(unsigned int mi_addr)
{
   put8(0x0F);
   put8(0x84);
   put32(0);
   add_jump(code_length-4, mi_addr, 0);
}

void je_near_rj(unsigned int saut)
{
   put8(0x0F);
   put8(0x84);
   put32(saut);
}

void jl_near(unsigned int mi_addr)
{
   put8(0x0F);
   put8(0x8C);
   put32(0);
   add_jump(code_length-4, mi_addr, 0);
}

void jl_near_rj(unsigned int saut)
{
   put8(0x0F);
   put8(0x8C);
   put32(saut);
}

void jne_near(unsigned int mi_addr)
{
   put8(0x0F);
   put8(0x85);
   put32(0);
   add_jump(code_length-4, mi_addr, 0);
}

void jne_near_rj(unsigned int saut)
{
   put8(0x0F);
   put8(0x85);
   put32(saut);
}

void jge_near(unsigned int mi_addr)
{
   put8(0x0F);
   put8(0x8D);
   put32(0);
   add_jump(code_length-4, mi_addr, 0);
}

void jge_near_rj(unsigned int saut)
{
   put8(0x0F);
   put8(0x8D);
   put32(saut);
}

void jg_near(unsigned int mi_addr)
{
   put8(0x0F);
   put8(0x8F);
   put32(0);
   add_jump(code_length-4, mi_addr, 0);
}

void jle_near(unsigned int mi_addr)
{
   put8(0x0F);
   put8(0x8E);
   put32(0);
   add_jump(code_length-4, mi_addr, 0);
}

void jle_near_rj(unsigned int saut)
{
   put8(0x0F);
   put8(0x8E);
   put32(saut);
}

void mov_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0xB8+reg32);
   put32(imm32);
}

void mov_reg64_imm64(int reg64, unsigned long long imm64)
{
   put8(0x48);
   put8(0xB8+reg64);
   put64(imm64);
}

void jmp_imm(int saut)
{
   put8(0xE9);
   put32(saut);
}

void jmp_imm_short(char saut)
{
   put8(0xEB);
   put8(saut);
}

void or_m32rel_imm32(unsigned int *m32, unsigned int imm32)
{
   int offset = rel_r15_offset(m32, "or_m32rel_imm32");

   put8(0x41);
   put8(0x81);
   put8(0x8F);
   put32(offset);
   put32(imm32);
}

void or_m32rel_xreg32(unsigned int *m32, unsigned int xreg32)
{
   int offset = rel_r15_offset(m32, "or_m32rel_xreg32");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x09);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void or_xreg32_m32rel(unsigned int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "or_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x0B);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void or_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x09);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void or_reg64_reg64(unsigned int reg1, unsigned int reg2)
{
   put8(0x48);
   put8(0x09);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void and_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x21);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void and_reg64_reg64(unsigned int reg1, unsigned int reg2)
{
   put8(0x48);
   put8(0x21);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void and_m32rel_imm32(unsigned int *m32, unsigned int imm32)
{
   int offset = rel_r15_offset(m32, "and_m32rel_imm32");

   put8(0x41);
   put8(0x81);
   put8(0xA7);
   put32(offset);
   put32(imm32);
}

void and_xreg32_m32rel(unsigned int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "and_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x23);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void xor_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x31);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void xor_reg64_reg64(unsigned int reg1, unsigned int reg2)
{
   put8(0x48);
   put8(0x31);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void xor_xreg32_m32rel(unsigned int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "xor_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x33);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void add_m32rel_imm32(unsigned int *m32, unsigned int imm32)
{
   int offset = rel_r15_offset(m32, "add_m32rel_imm32");

   put8(0x41);
   put8(0x81);
   put8(0x87);
   put32(offset);
   put32(imm32);
}

void add_m32rel_imm8(unsigned int *m32, unsigned char imm8)
{
   int offset = rel_r15_offset(m32, "add_m32rel_imm8");

   put8(0x41);
   put8(0x83);
   put8(0x87);
   put32(offset);
   put8(imm8);
}

void sub_m32rel_imm32(unsigned int *m32, unsigned int imm32)
{
   int offset = rel_r15_offset(m32, "sub_m32rel_imm32");

   put8(0x41);
   put8(0x81);
   put8(0xAF);
   put32(offset);
   put32(imm32);
}

void sub_m64rel_imm32(unsigned long long *m64, unsigned int imm32)
{
   int offset = rel_r15_offset(m64, "sub_m64rel_imm32");

   put8(0x49);
   put8(0x81);
   put8(0xAF);
   put32(offset);
   put32(imm32);
}

void add_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
   put8(0x83);
   put8(0xC0+reg32);
   put8(imm8);
}

void add_reg64_imm32(unsigned int reg64, unsigned int imm32)
{
   put8(0x48);
   put8(0x81);
   put8(0xC0+reg64);
   put32(imm32);
}

void add_reg32_imm32(unsigned int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xC0+reg32);
   put32(imm32);
}

void inc_m32rel(unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "inc_m32rel");

   put8(0x41);
   put8(0xFF);
   put8(0x87);
   put32(offset);
}

void cmp_m32rel_imm32(unsigned int *m32, unsigned int imm32)
{
   int offset = rel_r15_offset(m32, "cmp_m32rel_imm32");

   put8(0x41);
   put8(0x81);
   put8(0xBF);
   put32(offset);
   put32(imm32);
}

void cmp_m32rel_imm8(unsigned int *m32, unsigned char imm8)
{
   int offset = rel_r15_offset(m32, "cmp_m32rel_imm8");

   put8(0x41);
   put8(0x83);
   put8(0xBF);
   put32(offset);
   put8(imm8);
}

void cmp_m8rel_imm8(unsigned char *m8, unsigned char imm8)
{
   int offset = rel_r15_offset(m8, "cmp_m8rel_imm8");

   put8(0x41);
   put8(0x80);
   put8(0xBF);
   put32(offset);
   put8(imm8);
}

void cmp_eax_imm32(unsigned int imm32)
{
   put8(0x3D);
   put32(imm32);
}

void mov_m32rel_imm32(unsigned int *m32, unsigned int imm32)
{
   int offset = rel_r15_offset(m32, "mov_m32rel_imm32");

   put8(0x41);
   put8(0xC7);
   put8(0x87);
   put32(offset);
   put32(imm32);
}

void jmp(unsigned int mi_addr)
{
   put8(0xFF);
   put8(0x25);
   put32(0);
   put64(0);
   add_jump(code_length-8, mi_addr, 1);
}

void cdq(void)
{
   put8(0x99);
}

void cwde(void)
{
   put8(0x98);
}

void cbw(void)
{
   put8(0x66);
   put8(0x98);
}

void ret(void)
{
   put8(0xC3);
}

void call_reg64(unsigned int reg64)
{
   put8(0xFF);
   put8(0xD0+reg64);
}

void call_m64rel(unsigned long long *m64)
{
   int offset = rel_r15_offset(m64, "call_m64rel");

   put8(0x41);
   put8(0xFF);
   put8(0x97);
   put32(offset);
}

void shr_reg64_imm8(unsigned int reg64, unsigned char imm8)
{
   put8(0x48);
   put8(0xC1);
   put8(0xE8+reg64);
   put8(imm8);
}

void shr_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
   put8(0xC1);
   put8(0xE8+reg32);
   put8(imm8);
}

void shr_reg32_cl(unsigned int reg32)
{
   put8(0xD3);
   put8(0xE8+reg32);
}

void shr_reg64_cl(unsigned int reg64)
{
   put8(0x48);
   put8(0xD3);
   put8(0xE8+reg64);
}

void sar_reg32_cl(unsigned int reg32)
{
   put8(0xD3);
   put8(0xF8+reg32);
}

void sar_reg64_cl(unsigned int reg64)
{
   put8(0x48);
   put8(0xD3);
   put8(0xF8+reg64);
}

void shl_reg32_cl(unsigned int reg32)
{
   put8(0xD3);
   put8(0xE0+reg32);
}

void shl_reg64_cl(unsigned int reg64)
{
   put8(0x48);
   put8(0xD3);
   put8(0xE0+reg64);
}

void shld_reg32_reg32_cl(unsigned int reg1, unsigned int reg2)
{
   put8(0x0F);
   put8(0xA5);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void shld_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8)
{
   put8(0x0F);
   put8(0xA4);
   put8(0xC0 | (reg2 << 3) | reg1);
   put8(imm8);
}

void shrd_reg32_reg32_cl(unsigned int reg1, unsigned int reg2)
{
   put8(0x0F);
   put8(0xAD);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void sar_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
   put8(0xC1);
   put8(0xF8+reg32);
   put8(imm8);
}

void sar_reg64_imm8(unsigned int reg64, unsigned char imm8)
{
   put8(0x48);
   put8(0xC1);
   put8(0xF8+reg64);
   put8(imm8);
}

void shrd_reg32_reg32_imm8(unsigned int reg1, unsigned int reg2, unsigned char imm8)
{
   put8(0x0F);
   put8(0xAC);
   put8(0xC0 | (reg2 << 3) | reg1);
   put8(imm8);
}

void mul_m32rel(unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "mul_m32rel");

   put8(0x41);
   put8(0xF7);
   put8(0xA7);
   put32(offset);
}

void imul_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xE8+reg32);
}

void mul_reg64(unsigned int reg64)
{
   put8(0x48);
   put8(0xF7);
   put8(0xE0+reg64);
}

void mul_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xE0+reg32);
}

void idiv_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xF8+reg32);
}

void div_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xF0+reg32);
}

void idiv_m32rel(unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "idiv_m32rel");

   put8(0x41);
   put8(0xF7);
   put8(0xBF);
   put32(offset);
}

void div_m32rel(unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "div_m32rel");

   put8(0x41);
   put8(0xF7);
   put8(0xB7);
   put32(offset);
}

void add_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x01);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void add_reg64_reg64(unsigned int reg1, unsigned int reg2)
{
   put8(0x48);
   put8(0x01);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void add_xreg32_m32rel(unsigned int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "add_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x03);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void add_xreg64_m64rel(unsigned int xreg64, unsigned long long *m64)
{
   int offset = rel_r15_offset(m64, "add_xreg64_m64rel");

   put8(0x49 | ((xreg64 & 8) >> 1));
   put8(0x03);
   put8(0x87 | ((xreg64 & 7) << 3));
   put32(offset);
}

void adc_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   put8(0x11);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void adc_xreg32_m32rel(unsigned int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "adc_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x13);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void adc_reg32_imm32(unsigned int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xD0 + reg32);
   put32(imm32);
}

void jmp_reg64(unsigned int reg64)
{
   put8(0xFF);
   put8(0xE0 + reg64);
}

void jmp_m64rel(unsigned long long *m64)
{
   int offset = rel_r15_offset(m64, "jmp_m64rel");

   put8(0x41);
   put8(0xFF);
   put8(0xA7);
   put32(offset);
}

void mov_reg32_preg64(unsigned int reg1, unsigned int reg2)
{
   put8(0x8B);
   put8((reg1 << 3) | reg2);
}

void mov_preg64_reg32(int reg1, int reg2)
{
   put8(0x89);
   put8((reg2 << 3) | reg1);
}

void mov_preg64_reg64(int reg1, int reg2)
{
   put8(0x48);
   put8(0x89);
   put8((reg2 << 3) | reg1);
}

void mov_reg64_preg64(int reg1, int reg2)
{
   put8(0x48);
   put8(0x8B);
   put8((reg1 << 3) | reg2);
}

void mov_reg32_preg64preg64pimm32(int reg1, int reg2, int reg3, unsigned int imm32)
{
   put8(0x8B);
   put8((reg1 << 3) | 0x84);
   put8(reg2 | (reg3 << 3));
   put32(imm32);
}

void mov_preg64preg64pimm32_reg32(int reg1, int reg2, unsigned int imm32, int reg3)
{
   put8(0x89);
   put8((reg3 << 3) | 0x84);
   put8(reg1 | (reg2 << 3));
   put32(imm32);
}

void mov_reg64_preg64preg64pimm32(int reg1, int reg2, int reg3, unsigned int imm32)
{
   put8(0x48);
   put8(0x8B);
   put8((reg1 << 3) | 0x84);
   put8(reg2 | (reg3 << 3));
   put32(imm32);
}

void mov_reg32_preg64preg64(int reg1, int reg2, int reg3)
{
   put8(0x8B);
   put8((reg1 << 3) | 0x04);
   put8((reg2 << 3) | reg3);
}

void mov_reg64_preg64preg64(int reg1, int reg2, int reg3)
{
   put8(0x48);
   put8(0x8B);
   put8((reg1 << 3) | 0x04);
   put8(reg2 | (reg3 << 3));
}

void mov_reg32_preg64pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x8B);
   put8(0x80 | (reg1 << 3) | reg2);
   put32(imm32);
}

void mov_reg64_preg64pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x48);
   put8(0x8B);
   put8(0x80 | (reg1 << 3) | reg2);
   put32(imm32);
}

void mov_reg64_preg64pimm8(int reg1, int reg2, unsigned int imm8)
{
   put8(0x48);
   put8(0x8B);
   put8(0x40 | (reg1 << 3) | reg2);
   put8(imm8);
}

void mov_reg32_preg64x4pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x8B);
   put8((reg1 << 3) | 4);
   put8(0x80 | (reg2 << 3) | 5);
   put32(imm32);
}

void mov_reg64_preg64x8preg64(int reg1, int reg2, int reg3)
{
   put8(0x48);
   put8(0x8B);
   put8((reg1 << 3) | 4);
   put8(0xC0 | (reg2 << 3) | reg3);
}

void mov_preg64preg64_reg8(int reg1, int reg2, int reg8)
{
   put8(0x88);
   put8(0x04 | (reg8 << 3));
   put8((reg1 << 3) | reg2);
}

void mov_preg64preg64_imm8(int reg1, int reg2, unsigned char imm8)
{
   put8(0xC6);
   put8(0x04);
   put8((reg1 << 3) | reg2);
   put8(imm8);
}

void mov_preg64pimm32_imm8(int reg64, unsigned int imm32, unsigned char imm8)
{
   put8(0xC6);
   put8(0x80 + reg64);
   put32(imm32);
   put8(imm8);
}

void mov_preg64pimm32_reg16(int reg64, unsigned int imm32, int reg16)
{
   put8(0x66);
   put8(0x89);
   put8(0x80 | reg64 | (reg16 << 3));
   put32(imm32);
}

void mov_preg64preg64_reg16(int reg1, int reg2, int reg16)
{
   put8(0x66);
   put8(0x89);
   put8(0x04 | (reg16 << 3));
   put8((reg1 << 3) | reg2);
}

void mov_preg64preg64_reg32(int reg1, int reg2, int reg32)
{
   put8(0x89);
   put8(0x04 | (reg32 << 3));
   put8((reg1 << 3) | reg2);
}

void mov_preg64pimm32_reg32(int reg1, unsigned int imm32, int reg2)
{
   put8(0x89);
   put8(0x80 | reg1 | (reg2 << 3));
   put32(imm32);
}

void mov_preg64pimm8_reg64(int reg1, unsigned int imm8, int reg2)
{
   put8(0x48);
   put8(0x89);
   put8(0x40 | (reg2 << 3) | reg1);
   put8(imm8);
}

void add_eax_imm32(unsigned int imm32)
{
   put8(0x05);
   put32(imm32);
}

void shl_reg32_imm8(unsigned int reg32, unsigned char imm8)
{
   put8(0xC1);
   put8(0xE0 + reg32);
   put8(imm8);
}

void shl_reg64_imm8(unsigned int reg64, unsigned char imm8)
{
   put8(0x48);
   put8(0xC1);
   put8(0xE0 + reg64);
   put8(imm8);
}

void mov_reg32_reg32(unsigned int reg1, unsigned int reg2)
{
   if (reg1 == reg2) return;
   put8(0x89);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void mov_reg64_reg64(unsigned int reg1, unsigned int reg2)
{
   if (reg1 == reg2) return;
   put8(0x48);
   put8(0x89);
   put8(0xC0 | (reg2 << 3) | reg1);
}

void mov_xreg32_m32rel(unsigned int xreg32, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "mov_xreg32_m32rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x8B);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void mov_m32rel_xreg32(unsigned int *m32, unsigned int xreg32)
{
   int offset = rel_r15_offset(m32, "mov_m32rel_xreg32");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x89);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void mov_xreg64_m64rel(unsigned int xreg64, unsigned long long* m64)
{
   int offset = rel_r15_offset(m64, "mov_xreg64_m64rel");

   put8(0x49 | ((xreg64 & 8) >> 1));
   put8(0x8B);
   put8(0x87 | ((xreg64 & 7) << 3));
   put32(offset);
}

void mov_m64rel_xreg64(unsigned long long *m64, unsigned int xreg64)
{
   int offset = rel_r15_offset(m64, "mov_m64rel_xreg64");

   put8(0x49 | ((xreg64 & 8) >> 1));
   put8(0x89);
   put8(0x87 | ((xreg64 & 7) << 3));
   put32(offset);
}

void mov_xreg8_m8rel(int xreg8, unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "mov_xreg8_m8rel");

   put8(0x41 | ((xreg8 & 8) >> 1));
   put8(0x8A);
   put8(0x87 | ((xreg8 & 7) << 3));
   put32(offset);
}

void and_eax_imm32(unsigned int imm32)
{
   put8(0x25);
   put32(imm32);
}

void or_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xC8 + reg32);
   put32(imm32);
}

void or_reg64_imm32(int reg64, unsigned int imm32)
{
   put8(0x48);
   put8(0x81);
   put8(0xC8 + reg64);
   put32(imm32);
}

void and_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xE0 + reg32);
   put32(imm32);
}

void and_reg64_imm32(int reg64, unsigned int imm32)
{
   put8(0x48);
   put8(0x81);
   put8(0xE0 + reg64);
   put32(imm32);
}

void and_reg32_imm8(int reg32, unsigned char imm8)
{
   put8(0x83);
   put8(0xE0 + reg32);
   put8(imm8);
}

void and_reg64_imm8(int reg64, unsigned char imm8)
{
   put8(0x48);
   put8(0x83);
   put8(0xE0 + reg64);
   put8(imm8);
}

void and_ax_imm16(unsigned short imm16)
{
   put8(0x66);
   put8(0x25);
   put16(imm16);
}

void and_al_imm8(unsigned char imm8)
{
   put8(0x24);
   put8(imm8);
}

void or_ax_imm16(unsigned short imm16)
{
   put8(0x66);
   put8(0x0D);
   put16(imm16);
}

void or_eax_imm32(unsigned int imm32)
{
   put8(0x0D);
   put32(imm32);
}

void xor_ax_imm16(unsigned short imm16)
{
   put8(0x66);
   put8(0x35);
   put16(imm16);
}

void xor_al_imm8(unsigned char imm8)
{
   put8(0x34);
   put8(imm8);
}

void xor_reg64_imm32(int reg64, unsigned int imm32)
{
   put8(0x48);
   put8(0x81);
   put8(0xF0 + reg64);
   put32(imm32);
}

void xor_reg32_imm32(int reg32, unsigned int imm32)
{
   put8(0x81);
   put8(0xF0 + reg32);
   put32(imm32);
}

void xor_reg8_imm8(int reg8, unsigned char imm8)
{
   put8(0x40);  /* we need an REX prefix to use the uniform byte registers */
   put8(0x80);
   put8(0xF0 + reg8);
   put8(imm8);
}

void nop(void)
{
   put8(0x90);
}

void not_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xD0 + reg32);
}

void not_reg64(unsigned int reg64)
{
   put8(0x48);
   put8(0xF7);
   put8(0xD0 + reg64);
}

void neg_reg32(unsigned int reg32)
{
   put8(0xF7);
   put8(0xD8 + reg32);
}

void neg_reg64(unsigned int reg64)
{
   put8(0x48);
   put8(0xF7);
   put8(0xD8 + reg64);
}

void movsx_xreg32_m8rel(int xreg32, unsigned char *m8)
{
   int offset = rel_r15_offset(m8, "movsx_xreg32_m8rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x0F);
   put8(0xBE);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void movsx_reg32_8preg64pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x0F);
   put8(0xBE);
   put8((reg1 << 3) | reg2 | 0x80);
   put32(imm32);
}

void movsx_reg32_8preg64preg64(int reg1, int reg2, int reg3)
{
   put8(0x0F);
   put8(0xBE);
   put8((reg1 << 3) | 0x04);
   put8((reg2 << 3) | reg3);
}

void movsx_reg32_16preg64pimm32(int reg1, int reg2, unsigned int imm32)
{
   put8(0x0F);
   put8(0xBF);
   put8((reg1 << 3) | reg2 | 0x80);
   put32(imm32);
}

void movsx_reg32_16preg64preg64(int reg1, int reg2, int reg3)
{
   put8(0x0F);
   put8(0xBF);
   put8((reg1 << 3) | 0x04);
   put8((reg2 << 3) | reg3);
}

void movsx_reg32_reg16(int reg32, int reg16)
{
   put8(0x0F);
   put8(0xBF);
   put8((reg32 << 3) | reg16 | 0xC0);
}

void movsx_xreg32_m16rel(int xreg32, unsigned short *m16)
{
   int offset = rel_r15_offset(m16, "movsx_xreg32_m16rel");

   put8(0x41 | ((xreg32 & 8) >> 1));
   put8(0x0F);
   put8(0xBF);
   put8(0x87 | ((xreg32 & 7) << 3));
   put32(offset);
}

void movsxd_reg64_reg32(int reg64, int reg32)
{
   put8(0x48);
   put8(0x63);
   put8((reg64 << 3) | reg32 | 0xC0);
}

void movsxd_xreg64_m32rel(int xreg64, unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "movsxd_xreg64_m32rel");

   put8(0x49 | ((xreg64 & 8) >> 1));
   put8(0x63);
   put8(0x87 | ((xreg64 & 7) << 3));
   put32(offset);
}

void fldcw_m16rel(unsigned short *m16)
{
   int offset = rel_r15_offset(m16, "fldcw_m16rel");

   put8(0x41);
   put8(0xD9);
   put8(0xAF);
   put32(offset);
}

void fld_preg64_dword(int reg64)
{
   put8(0xD9);
   put8(reg64);
}

void fdiv_preg64_dword(int reg64)
{
   put8(0xD8);
   put8(0x30 + reg64);
}

void fstp_preg64_dword(int reg64)
{
   put8(0xD9);
   put8(0x18 + reg64);
}

void fchs(void)
{
   put8(0xD9);
   put8(0xE0);
}

void fstp_preg64_qword(int reg64)
{
   put8(0xDD);
   put8(0x18 + reg64);
}

void fadd_preg64_dword(int reg64)
{
   put8(0xD8);
   put8(reg64);
}

void fsub_preg64_dword(int reg64)
{
   put8(0xD8);
   put8(0x20 + reg64);
}

void fmul_preg64_dword(int reg64)
{
   put8(0xD8);
   put8(0x08 + reg64);
}

void fcomp_preg64_dword(int reg64)
{
   put8(0xD8);
   put8(0x18 + reg64);
}

void fistp_preg64_dword(int reg64)
{
   put8(0xDB);
   put8(0x18 + reg64);
}

void fistp_m32rel(unsigned int *m32)
{
   int offset = rel_r15_offset(m32, "fistp_m32rel");

   put8(0x41);
   put8(0xDB);
   put8(0x9F);
   put32(offset);
}

void fistp_preg64_qword(int reg64)
{
   put8(0xDF);
   put8(0x38 + reg64);
}

void fistp_m64rel(unsigned long long *m64)
{
   int offset = rel_r15_offset(m64, "fistp_m64rel");

   put8(0x41);
   put8(0xDF);
   put8(0xBF);
   put32(offset);
}

void fld_preg64_qword(int reg64)
{
   put8(0xDD);
   put8(reg64);
}

void fild_preg64_qword(int reg64)
{
   put8(0xDF);
   put8(0x28+reg64);
}

void fild_preg64_dword(int reg64)
{
   put8(0xDB);
   put8(reg64);
}

void fadd_preg64_qword(int reg64)
{
   put8(0xDC);
   put8(reg64);
}

void fdiv_preg64_qword(int reg64)
{
   put8(0xDC);
   put8(0x30 + reg64);
}

void fsub_preg64_qword(int reg64)
{
   put8(0xDC);
   put8(0x20 + reg64);
}

void fmul_preg64_qword(int reg64)
{
   put8(0xDC);
   put8(0x08 + reg64);
}

void fsqrt(void)
{
   put8(0xD9);
   put8(0xFA);
}

void fabs_(void)
{
   put8(0xD9);
   put8(0xE1);
}

void fcomip_fpreg(int fpreg)
{
   put8(0xDF);
   put8(0xF0 + fpreg);
}

void fucomip_fpreg(int fpreg)
{
   put8(0xDF);
   put8(0xE8 + fpreg);
}

void ffree_fpreg(int fpreg)
{
   put8(0xDD);
   put8(0xC0 + fpreg);
}

