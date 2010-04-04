/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2007, 2008, 2009 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CRABZ80_H
#define CRABZ80_H

#ifdef IN_CRABEMU
#include "CrabEmu.h"
#else

#ifdef __cplusplus
#define CLINKAGE extern "C" {
#define ENDCLINK }
#else
#define CLINKAGE
#define ENDCLINK
#endif /* __cplusplus */

#ifndef CRABEMU_TYPEDEFS
#define CRABEMU_TYPEDEFS

#ifdef _arch_dreamcast
#include <arch/types.h>
#else

#include <stdint.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef int8_t int8;
typedef int16_t int16;
typedef uint32_t int32;
#endif /* _arch_dreamcast */
#endif /* CRABEMU_TYPEDEFS */
#endif /* IN_CRABEMU */

CLINKAGE

typedef union   {
    struct  {
#ifdef __BIG_ENDIAN__
        uint8 h;
        uint8 l;
#else
        uint8 l;
        uint8 h;
#endif
    } b;
    uint8 bytes[2];
    uint16 w;
} CrabZ80_reg_t;

typedef union   {
    struct  {
#ifdef __BIG_ENDIAN__
        uint8 l;
        uint8 h;
#else
        uint8 h;
        uint8 l;
#endif
    } b;
    uint16 w;
} CrabZ80_regAF_t;

#ifdef __BIG_ENDIAN__
#define REG8(x) cpu->regs8[(x) & 0x07]
#else
#define REG8(x) cpu->regs8[((x) & 0x07) ^ 0x01]
#endif

#define REG16(x) cpu->regs16[(x) & 0x03]

typedef struct CrabZ80_struct   {
    union   {
        uint8 regs8[8];
        uint16 regs16[4];
        struct  {
            CrabZ80_reg_t bc;
            CrabZ80_reg_t de;
            CrabZ80_reg_t hl;
            CrabZ80_regAF_t af;
        };
    };
    CrabZ80_reg_t ix;
    CrabZ80_reg_t iy;
    CrabZ80_reg_t pc;
    CrabZ80_reg_t sp;
    CrabZ80_reg_t ir;
    CrabZ80_regAF_t afp;
    CrabZ80_reg_t bcp;
    CrabZ80_reg_t dep;
    CrabZ80_reg_t hlp;

    CrabZ80_reg_t *offset;

    uint8 internal_reg;
    uint8 iff1;
    uint8 iff2;
    uint8 im;

    uint8 halt;
    uint8 ei;
    uint8 irq_pending;
    uint8 r_top;

    uint32 irq_vector;
    uint32 cycles;
    uint32 cycles_in;

    uint8 (*pread)(uint16 port);
    uint8 (*mread)(uint16 addr);

    void (*pwrite)(uint16 port, uint8 data);
    void (*mwrite)(uint16 addr, uint8 data);

    uint16 (*mread16)(uint16 addr);
    void (*mwrite16)(uint16 addr, uint16 data);

    uint8 *readmap[256];
} CrabZ80_t;

/* Flag definitions */
#define CRABZ80_CF      0
#define CRABZ80_NF      1
#define CRABZ80_PF      2
#define CRABZ80_XF      3
#define CRABZ80_HF      4
#define CRABZ80_YF      5
#define CRABZ80_ZF      6
#define CRABZ80_SF      7

/* Flag setting macros */
#define CRABZ80_SET_FLAG(z80, n)    (z80)->af.b.l |= (1 << (n))
#define CRABZ80_CLEAR_FLAG(z80, n)  (z80)->af.b.l &= (~(1 << (n)))
#define CRABZ80_GET_FLAG(z80, n)    ((z80)->af.b.l >> (n)) & 1

/* Function definitions */
void CrabZ80_init(CrabZ80_t *cpu);
void CrabZ80_reset(CrabZ80_t *cpu);

void CrabZ80_pulse_nmi(CrabZ80_t *cpu);
void CrabZ80_assert_irq(CrabZ80_t *cpu, uint32 vector);
void CrabZ80_clear_irq(CrabZ80_t *cpu);

uint32 CrabZ80_execute(CrabZ80_t *cpu, uint32 cycles);

void CrabZ80_release_cycles();

void CrabZ80_set_portread(CrabZ80_t *cpu, uint8 (*pread)(uint16 port));
void CrabZ80_set_memread(CrabZ80_t *cpu, uint8 (*mread)(uint16 addr));
void CrabZ80_set_portwrite(CrabZ80_t *cpu,
                           void (*pwrite)(uint16 port, uint8 data));
void CrabZ80_set_memwrite(CrabZ80_t *cpu,
                          void (*mwrite)(uint16 addr, uint8 data));

void CrabZ80_set_memread16(CrabZ80_t *cpu, uint16 (*mread16)(uint16 addr));
void CrabZ80_set_memwrite16(CrabZ80_t *cpu,
                            void (*mwrite16)(uint16 addr, uint16 data));

void CrabZ80_set_readmap(CrabZ80_t *cpuz80, uint8 *readmap[256]);

ENDCLINK

#endif /* !CRABZ80_H */
