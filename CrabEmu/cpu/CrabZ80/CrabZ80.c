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

#include "CrabZ80.h"
#include "CrabZ80_tables.h"
#include "CrabZ80_macros.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static CrabZ80_t *cpu = NULL;

#ifndef CRABZ80_NO_READMAP_FALLBACK

#define FETCH_BYTE(name)    \
    uint8 name = cpu->readmap[cpu->pc.w >> 8] ? \
        cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF] : \
        cpu->mread(cpu->pc.w); \
    ++cpu->pc.w;

#define FETCH_ARG8(name) \
    name = cpu->readmap[cpu->pc.w >> 8] ? \
        cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF] : \
        cpu->mread(cpu->pc.w);  \
    ++cpu->pc.w;

#define FETCH_WORD(name)    \
    uint16 name = cpu->readmap[cpu->pc.w >> 8] ? \
        cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF] : \
        cpu->mread(cpu->pc.w) | \
        ((cpu->readmap[(cpu->pc.w + 1) >> 8] ? \
          cpu->readmap[(cpu->pc.w + 1) >> 8][(cpu->pc.w + 1) & 0xFF] : \
          cpu->mread(cpu->pc.w + 1)) << 8); \
    cpu->pc.w += 2;

#define FETCH_ARG16(name)    \
    name = cpu->readmap[cpu->pc.w >> 8] ? \
        cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF] : \
        cpu->mread(cpu->pc.w) | \
        ((cpu->readmap[(cpu->pc.w + 1) >> 8] ? \
          cpu->readmap[(cpu->pc.w + 1) >> 8][(cpu->pc.w + 1) & 0xFF] : \
          cpu->mread(cpu->pc.w + 1)) << 8); \
    cpu->pc.w += 2;

#else

#define FETCH_BYTE(name)    \
    uint8 name = cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF]; \
    ++cpu->pc.w;

#define FETCH_ARG8(name)    \
    name = cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF]; \
    ++cpu->pc.w;

#define FETCH_WORD(name)    \
    uint16 name = cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF] | \
        (cpu->readmap[(cpu->pc.w + 1) >> 8][(cpu->pc.w + 1) & 0xFF] << 8); \
    cpu->pc.w += 2;

#define FETCH_ARG16(name)    \
    name = cpu->readmap[cpu->pc.w >> 8][cpu->pc.w & 0xFF] | \
        (cpu->readmap[(cpu->pc.w + 1) >> 8][(cpu->pc.w + 1) & 0xFF] << 8); \
    cpu->pc.w += 2;

#endif

static uint8 CrabZ80_dummy_read(uint16 addr __attribute__((unused)))    {
    return 0;
}

static void CrabZ80_dummy_write(uint16 addr __attribute__((unused)), 
                                uint8 data __attribute__((unused))) {
}

static uint16 CrabZ80_default_mread16(uint16 addr)  {
    if(!cpu)    {
#ifdef CRABZ80_DEBUG
        fprintf(stderr, "CrabZ80_default_mread16: No CPU structure set!");
#endif
        assert(0);
    }

    return cpu->mread(addr) | (cpu->mread((addr + 1) & 0xFFFF) << 8);
}

static void CrabZ80_default_mwrite16(uint16 addr, uint16 data)  {
    if(!cpu)    {
#ifdef CRABZ80_DEBUG
        fprintf(stderr, "CrabZ80_default_mwrite16: No CPU structure set!");
#endif
        assert(0);
    }

    cpu->mwrite((addr + 1) & 0xFFFF, data >> 8);
    cpu->mwrite(addr, data & 0xFF);
}

void CrabZ80_set_portread(CrabZ80_t *cpuz80, uint8 (*pread)(uint16 port))   {
    if(pread == NULL)
        cpuz80->pread = CrabZ80_dummy_read;
    else
        cpuz80->pread = pread;
}

void CrabZ80_set_memread(CrabZ80_t *cpuz80, uint8 (*mread)(uint16 addr))    {
    if(mread == NULL)
        cpuz80->mread = CrabZ80_dummy_read;
    else
        cpuz80->mread = mread;
}

void CrabZ80_set_portwrite(CrabZ80_t *cpuz80,
                           void (*pwrite)(uint16 port, uint8 data)) {
    if(pwrite == NULL)
        cpuz80->pwrite = CrabZ80_dummy_write;
    else
        cpuz80->pwrite = pwrite;
}

void CrabZ80_set_memwrite(CrabZ80_t *cpuz80,
                          void (*mwrite)(uint16 addr, uint8 data))  {
    if(mwrite == NULL)
        cpuz80->mwrite = CrabZ80_dummy_write;
    else
        cpuz80->mwrite = mwrite;
}

void CrabZ80_set_memread16(CrabZ80_t *cpuz80,
                           uint16 (*mread16)(uint16 addr))  {
    if(mread16 == NULL)
        cpuz80->mread16 = CrabZ80_default_mread16;
    else
        cpuz80->mread16 = mread16;
}

void CrabZ80_set_memwrite16(CrabZ80_t *cpuz80,
                            void (*mwrite16)(uint16 addr, uint16 data)) {
    if(mwrite16 == NULL)
        cpuz80->mwrite16 = CrabZ80_default_mwrite16;
    else
        cpuz80->mwrite16 = mwrite16;
}

void CrabZ80_set_readmap(CrabZ80_t *cpuz80, uint8 *readmap[256])    {
    memcpy(cpuz80->readmap, readmap, 256 * sizeof(uint8 *));
}

void CrabZ80_init(CrabZ80_t *cpuz80)    {
    cpuz80->pread = CrabZ80_dummy_read;
    cpuz80->mread = CrabZ80_dummy_read;
    cpuz80->pwrite = CrabZ80_dummy_write;
    cpuz80->mwrite = CrabZ80_dummy_write;
    cpuz80->mread16 = CrabZ80_default_mread16;
    cpuz80->mwrite16 = CrabZ80_default_mwrite16;

    memset(cpuz80->readmap, 0, 256 * sizeof(uint8 *));
}

void CrabZ80_reset(CrabZ80_t *cpuz80)   {
    cpuz80->pc.w = 0x0000;
    cpuz80->iff1 = 0;
    cpuz80->iff2 = 0;
    cpuz80->im = 0;
    cpuz80->halt = 0;
    cpuz80->r_top = 0;
    cpuz80->af.w = 0x4000;
    cpuz80->bc.w = 0x0000;
    cpuz80->de.w = 0x0000;
    cpuz80->hl.w = 0x0000;
    cpuz80->ix.w = 0xFFFF;
    cpuz80->iy.w = 0xFFFF;
    cpuz80->sp.w = 0x0000;
    cpuz80->ir.w = 0x0000;
    cpuz80->afp.w = 0x0000;
    cpuz80->bcp.w = 0x0000;
    cpuz80->dep.w = 0x0000;
    cpuz80->hlp.w = 0x0000;
    cpuz80->cycles = 0;
    cpuz80->cycles_in = 0;
    cpuz80->ei = 0;
    cpuz80->irq_pending = 0;
}

static uint32 CrabZ80_take_nmi(CrabZ80_t *cpuz80)   {
    if(cpuz80->halt)    {
        cpuz80->pc.w++;
        cpuz80->halt = 0;
    }

#ifndef CRABZ80_MAMEZ80_COMPAT
    ++cpuz80->ir.b.l;
#endif

    cpuz80->iff1 = 0;
    cpuz80->sp.w -= 2;
    cpuz80->mwrite16(cpuz80->sp.w, cpuz80->pc.w);
    cpuz80->pc.w = 0x0066;

    cpuz80->irq_pending &= 1;

    return 11;
}

static uint32 CrabZ80_take_irq(CrabZ80_t *cpuz80)   {
    if(cpuz80->halt)    {
        cpuz80->pc.w++;
        cpuz80->halt = 0;
    }

#ifndef CRABZ80_MAMEZ80_COMPAT
    ++cpuz80->ir.b.l;
#endif

    cpuz80->iff1 = cpuz80->iff2 = 0;

    switch(cpuz80->im)  {
        /* This case 0 is a hack specific to the Sega Master System 2/Game Gear
           technically, we should have some sort of function to give us the
           opcode to use in mode 0, but its not implemented yet. If someone
           really wants to use CrabZ80 for something that needs mode 0,
           let me know, and I'll implement it properly. Until then,
           or when I decide to do it, it'll stay this way. */
        case 0:
        case 1:
            cpuz80->sp.w -= 2;
            cpuz80->mwrite16(cpu->sp.w, cpu->pc.w);
            cpuz80->pc.w = 0x0038;
            return 13;

        case 2:
        {
            uint16 tmp = (cpuz80->ir.b.h << 8) + (cpuz80->irq_vector & 0xFF);
            cpuz80->sp.w -= 2;
            cpuz80->mwrite16(cpu->sp.w, cpu->pc.w);
            cpuz80->pc.w = cpu->mread16(tmp);
            return 19;
        }

        default:
#ifdef CRABZ80_DEBUG
            fprintf(stderr, "Invalid interrupt mode %d, report this error\n",
                    cpuz80->im);
#endif
            assert(0);
    }
}

void CrabZ80_pulse_nmi(CrabZ80_t *cpuz80)   {
    cpuz80->irq_pending |= 2;
}

void CrabZ80_assert_irq(CrabZ80_t *cpuz80, uint32 vector)   {
    cpuz80->irq_pending |= 1;
    cpuz80->irq_vector = vector;
}

void CrabZ80_clear_irq(CrabZ80_t *cpuz80)   {
    cpuz80->irq_pending &= 2;
}

void CrabZ80_release_cycles(CrabZ80_t *cpuz80)  {
    cpuz80->cycles_in = 0;
}

uint32 CrabZ80_execute(CrabZ80_t *cpuin, uint32 cycles) {
    register uint32 cycles_done = 0;
    CrabZ80_t *oldcpu = NULL;
    uint32 oldcyclesin = 0, oldcycles = 0;

    if(cpu) {
        oldcyclesin = cpu->cycles_in;
        oldcycles = cpu->cycles;
        oldcpu = cpu;
    }

    cpu = cpuin;

    cycles_done = 0;
    cpuin->cycles_in = cycles;
    cpuin->cycles = 0;

    while(cycles_done < cpuin->cycles_in)   {
        if(cpuin->irq_pending & 2)  {
            cycles_done += CrabZ80_take_nmi(cpuin);
        }
        else if(cpuin->irq_pending && !cpuin->ei && cpuin->iff1)    {
            cycles_done += CrabZ80_take_irq(cpuin);
        }

        cpuin->ei = 0;
        {
            FETCH_BYTE(inst);
            ++cpuin->ir.b.l;
#define INSIDE_CRABZ80_EXECUTE
#include "CrabZ80ops.h"
#undef INSIDE_CRABZ80_EXECUTE
        }

out:
        cpuin->cycles = cycles_done;
    }

    cpu = oldcpu;

    if(cpu) {
        cpu->cycles = oldcycles;
        cpu->cycles_in = oldcyclesin;
    }

    return cycles_done;
}
