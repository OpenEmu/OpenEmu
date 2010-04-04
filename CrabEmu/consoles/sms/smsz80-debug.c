/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2008 Lawrence Sebald

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

/* This file contains the z80 core debugger that I wrote
   to compare how CrabZ80 worked versus the Z80 core from
   MAME. The code isn't pretty, but its here, in case
   anyone wants to compile their own binary with it.
   (These binaries cannot be released due to licensing
   restrictions on the MAME core that contradict the GPL).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smsz80.h"

#include "CrabZ80d.h"

#include "../../cpu/mamez80/z80.h"

typedef struct  {
    uint16 addr;
    uint8 val;
    uint8 write;
} debug_memory_ent;

CrabZ80_t *cpuz80;
extern Z80_Regs Z80;

debug_memory_ent crabmem[10];
debug_memory_ent mz80mem[10];

debug_memory_ent crabport[10];
debug_memory_ent mz80port[10];

debug_memory_ent portread[10];
debug_memory_ent memread[200];

int debug_crab_ents = 0;
int debug_mz80_ents = 0;
int debug_crab_ports = 0;
int debug_mz80_ports = 0;
int debug_port_reads1 = 0;
int debug_port_reads2 = 0;
int debug_mem_reads1 = 0;
int debug_mem_reads2 = 0;

uint8 (*mread8)(uint16) = NULL;
void (*mwrite8)(uint16, uint8) = NULL;

static void sms_debug_memwrite(uint16 addr, uint8 val)  {
    crabmem[debug_crab_ents].addr = addr;
    crabmem[debug_crab_ents].write = 1;
    crabmem[debug_crab_ents++].val = val;

    mwrite8(addr, val);
}

static uint8 sms_debug_portread(uint16 port)    {
    uint8 res = sms_port_read(port);

    portread[debug_port_reads1].addr = port;
    portread[debug_port_reads1].val = res;
    portread[debug_port_reads1++].write = 0;

    return res;
}

static void sms_debug_portwrite(uint16 port, uint8 data)   {
    crabport[debug_crab_ports].addr = port & 0xFF;
    crabport[debug_crab_ports].write = 1;
    crabport[debug_crab_ports++].val = data;

    sms_port_write(port, data);
}

static uint8 sms_debug_memread(uint16 addr) {
    uint8 res = mread8(addr);

    memread[debug_mem_reads1].addr = addr;
    memread[debug_mem_reads1].write = 0;
    memread[debug_mem_reads1++].val = res;

    return res;
}

/* Glue code for mz80 */
void program_write_byte_8(UINT32 addr, UINT8 val)   {
    mz80mem[debug_mz80_ents].addr = addr;
    mz80mem[debug_mz80_ents].write = 1;
    mz80mem[debug_mz80_ents++].val = val;
}

UINT8 program_read_byte_8(UINT32 addr)  {
    int i;

    for(i = 0; i < debug_mem_reads1; ++i)	{
        if(memread[i].addr == (addr & 0xFFFF))	
            return memread[i].val;
    }

    printf("reading other addr: %04x %04X\n", addr & 0xFFFF, Z80.pc.w.l);
    return mread8(addr);
}

#define UNUSED __attribute__((unused))

void io_write_byte_8(UINT32 port, UINT8 val)    {
    mz80port[debug_mz80_ports].addr = port & 0xFF;
    mz80port[debug_mz80_ports].write = 1;
    mz80port[debug_mz80_ports++].val = val;
}

UINT8 io_read_byte_8(UINT32 port)   {
    int i;

    for(i = 0; i < debug_port_reads1; ++i)  {
        if(portread[i].addr == (port & 0xFF))	
            return portread[i].val;
    }

    printf("reading other port: %04x %04X\n", port & 0xFF, Z80.pc.w.l);
    return sms_port_read(port & 0xFF);
}

static int _compare_registers() {
    int f = 0;
    if(cpuz80->af.b.h != Z80.af.b.h)    {
        printf("A does not match, %x v %x\n", cpuz80->af.b.h, Z80.af.b.h);
        f++;
    }
    if(cpuz80->af.b.l != Z80.af.b.l)    {
        printf("F does not match, %02X v %02X\n", cpuz80->af.b.l, Z80.af.b.l);
        f++;
    }
    if(cpuz80->bc.b.h != Z80.bc.b.h)    {
        printf("B does not match, %x v %x\n", cpuz80->bc.b.h, Z80.bc.b.h);
        f++;
    }
    if(cpuz80->bc.b.l != Z80.bc.b.l)    {
        printf("C does not match, %x v %x\n", cpuz80->bc.b.l, Z80.bc.b.l);
        f++;
    }
    if(cpuz80->de.b.h != Z80.de.b.h)    {
        printf("D does not match, %x v %x\n", cpuz80->de.b.h, Z80.de.b.h);
        f++;
    }
    if(cpuz80->de.b.l != Z80.de.b.l)    {
        printf("E does not match, %x v %x\n", cpuz80->de.b.l, Z80.de.b.l);
        f++;
    }
    if(cpuz80->hl.b.h != Z80.hl.b.h)    {
        printf("H does not match, %x v %x\n", cpuz80->hl.b.h, Z80.hl.b.h);
        f++;
    }
    if(cpuz80->hl.b.l != Z80.hl.b.l)    {
        printf("L does not match, %x v %x\n", cpuz80->hl.b.l, Z80.hl.b.l);
        f++;
    }
    if(cpuz80->afp.b.h != Z80.af2.b.h)  {
        printf("Ap does not match, %x v %x\n", cpuz80->afp.b.h, Z80.af2.b.h);
        f++;
    }
    if(cpuz80->afp.b.l != Z80.af2.b.l)  {
        printf("Fp does not match, %x v %x\n", cpuz80->afp.b.l, Z80.af2.b.l);
        f++;
    }
    if(cpuz80->bcp.b.h != Z80.bc2.b.h)  {
        printf("Bp does not match, %x v %x\n", cpuz80->bcp.b.h, Z80.bc2.b.h);
        f++;
    }
    if(cpuz80->bcp.b.l != Z80.bc2.b.l)  {
        printf("Cp does not match, %x v %x\n", cpuz80->bcp.b.l, Z80.bc2.b.l);
        f++;
    }
    if(cpuz80->dep.b.h != Z80.de2.b.h)  {
        printf("Dp does not match, %x v %x\n", cpuz80->dep.b.h, Z80.de2.b.h);
        f++;
    }
    if(cpuz80->dep.b.l != Z80.de2.b.l)  {
        printf("Ep does not match, %x v %x\n", cpuz80->dep.b.l, Z80.de2.b.l);
        f++;
    }
    if(cpuz80->hlp.b.h != Z80.hl2.b.h)  {
        printf("Hp does not match, %x v %x\n", cpuz80->hlp.b.h, Z80.hl2.b.h);
        f++;
    }
    if(cpuz80->hlp.b.l != Z80.hl2.b.l)  {
        printf("Lp does not match, %x v %x\n", cpuz80->hlp.b.l, Z80.hl2.b.l);
        f++;
    }
    if(cpuz80->pc.w != Z80.pc.w.l)  {
        printf("PC does not match, %x v %x\n", cpuz80->pc.w, Z80.pc.w.l);
        f++;
    }
    if(cpuz80->ix.w != Z80.ix.w.l)  {
        printf("IX does not match %x v %x\n", cpuz80->ix.w, Z80.iy.w.l);
        f++;
    }
    if(cpuz80->iy.w != Z80.iy.w.l)  {
        printf("IY does not match %x v %x\n", cpuz80->iy.w, Z80.ix.w.l);
        f++;
    }
    if(cpuz80->sp.w != Z80.sp.w.l)  {
        printf("SP does not match %x v %x\n", cpuz80->sp.w, Z80.sp.w.l);
        f++;
    }
    if(cpuz80->ir.b.h != Z80.i) {
        printf("I does not match %x v %x\n", cpuz80->ir.b.h, Z80.i);
        f++;
    }
    if(cpuz80->ir.b.l != Z80.r) {
        printf("R does not match %x v %x\n", cpuz80->ir.b.l, Z80.r);
        f++;
    }

    return f;
}

static int _compare_memory()    {
    int i;
    int f = 0;

    for(i = 0; i < debug_crab_ents; ++i)    {
        if(crabmem[i].addr != mz80mem[i].addr)  {
            printf("Mismatched addresses (write=%d)! %02x v %02x\n", crabmem[i].write, crabmem[i].addr, mz80mem[i].addr);
            f++;
        }
        else if(crabmem[i].val != mz80mem[i].val)   {
            printf("mismatched bytes (write=%d)! %02x v %02x\n", crabmem[i].write, crabmem[i].val, mz80mem[i].val);
            f++;
        }
    }

    return f;
}

static int _compare_ports() {
    int i;
    int f = 0;

    for(i = 0; i < debug_crab_ports; ++i)   {
        if(crabport[i].addr != mz80port[i].addr)    {
            printf("Mismatched ports (write=%d)! %02x v %02x\n", crabport[i].write, crabport[i].addr, mz80port[i].addr);
            f++;
        }
        else if(crabport[i].val != mz80port[i].val) {
            printf("mismatched pbytes (write=%d)! %02x v %02x\n", crabport[i].write, crabport[i].val, mz80port[i].val);
            f++;
        }
    }

    return f;
}

int sms_z80_init(void)  {
    cpuz80 = (CrabZ80_t *)malloc(sizeof(CrabZ80_t));

    if(cpuz80 == NULL)  {
        fprintf(stderr, "Out of memory while initializing Z80 in debug mode\n");
        return -1;
    }

    /* Initialize CrabZ80 */
    CrabZ80_init(cpuz80);
    CrabZ80_reset(cpuz80);

    CrabZ80_set_memwrite(cpuz80, sms_debug_memwrite);
    CrabZ80_set_memread(cpuz80, sms_debug_memread);
    CrabZ80_set_portwrite(cpuz80, sms_debug_portwrite);
    CrabZ80_set_portread(cpuz80, sms_debug_portread);

    z80_init();
    z80_reset();

    return 0;
}

int sms_z80_shutdown(void)  {
    free(cpuz80);
    z80_exit();

    return 0;
}

void sms_z80_reset(void)    {
    CrabZ80_reset(cpuz80);
    z80_reset();
}

#define INPUT_LINE_NMI 127

void sms_z80_assert_irq(void)   {
    CrabZ80_assert_irq(cpuz80, 0xFFFFFFFF);
    z80_set_irq_line(1, 1);
}

void sms_z80_clear_irq(void)    {
    CrabZ80_clear_irq(cpuz80);
    z80_set_irq_line(1, 0);
}

void sms_z80_nmi(void)  {
    CrabZ80_pulse_nmi(cpuz80);
    z80_set_irq_line(INPUT_LINE_NMI, 1);
}

uint32 sms_z80_run(uint32 cycles)   {
    uint32 cyclesdone = 0;
    uint32 opcode;
    uint16 pc;
    int tmp;
    static char last_str[256];
    char str[256];
    static uint16 last_pc = 0;
    uint32 c1, c2;

    while(cyclesdone < cycles)  {
        debug_crab_ents = debug_mz80_ents = 0;
        debug_port_reads1 = debug_port_reads2 = 0;
        debug_mem_reads1 = debug_mem_reads2 = 0;
        debug_crab_ports = debug_mz80_ports = 0;

        pc = cpuz80->pc.w;

        opcode = mread8(cpuz80->pc.w);

        if(opcode == 0xED || opcode == 0xCB || opcode == 0xDD || opcode == 0xFD)    {
            opcode |= (mread8(cpuz80->pc.w + 1) << 8);
        }
        if((opcode & 0xFF00) == 0xCB00) {
            opcode |= (mread8(cpuz80->pc.w + 2) << 16) | (mread8(cpuz80->pc.w + 3) << 24);
        }

        c1 = CrabZ80_execute(cpuz80, 1);
        c2 = z80_execute(1);
        cyclesdone += c1;

        tmp = _compare_registers() + _compare_memory() + _compare_ports();
        CrabZ80_disassemble(str, cpuz80, pc);

        if(tmp) {
            printf("Cycles done: %d %d\n", (int)c1, (int)c2);
            printf("%d errors at: PC = 0x%04X, old_pc = 0x%04X Opcode = 0x%02X",
                   tmp, pc, last_pc, opcode & 0xFF);
            switch(opcode & 0xFF)   {
                case 0xED:
                case 0xCB:
                    printf("%02X (%s -- %s)\n", (opcode & 0xFF00) >> 8, str,
                           last_str);
                    break;
                case 0xDD:
                case 0xFD:
                    if((opcode & 0xFF00) == 0xCB00)
                        printf("%02X%02X%02X (%s -- %s)\n",
                               (opcode & 0xFF00) >> 8,
                               (opcode & 0xFF0000) >> 16,
                               (opcode & 0xFF000000) >> 24, str, last_str);
                    else
                        printf("%02X (%s -- %s)\n", (opcode & 0xFF00) >> 8, str,
                               last_str);
                    break;
                default:
                    printf("(%s -- %s)\n", str, last_str);
            }
        }

        strcpy(last_str, str);
        last_pc = pc;
    }

    return cyclesdone;
}

uint16 sms_z80_get_pc(void) {
    return cpuz80->pc.w;
}

void sms_z80_set_mread(uint8 (*mread)(uint16))  {
    mread8 = mread;
}

void sms_z80_set_mwrite(void (*mwrite)(uint16, uint8))  {
    mwrite8 = mwrite;
}

void sms_z80_set_mread16(uint16 (*mread)(uint16) UNUSED)    {
    /* Disabled for debugging */
}

void sms_z80_set_mwrite16(void (*mwrite)(uint16, uint16) UNUSED)    {
    /* Disabled for debugging */
}

void sms_z80_set_readmap(uint8 *readmap[256] UNUSED)    {
    /* Disabled for debugging */
}

#define WRITE_REG(reg) { \
    fwrite(&cpuz80->reg.b.l, 1, 1, fp); \
    fwrite(&cpuz80->reg.b.h, 1, 1, fp); \
}

#define READ_REG(reg)   { \
    fread(&cpuz80->reg.b.l, 1, 1, fp); \
    fread(&cpuz80->reg.b.h, 1, 1, fp); \
}

void sms_z80_write_context(FILE *fp)    {
    if(cpuz80 == NULL)
        return;

    WRITE_REG(af);
    WRITE_REG(bc);
    WRITE_REG(de);
    WRITE_REG(hl);
    WRITE_REG(ix);
    WRITE_REG(iy);
    WRITE_REG(pc);
    WRITE_REG(sp);
    WRITE_REG(ir);
    WRITE_REG(afp);
    WRITE_REG(bcp);
    WRITE_REG(dep);
    WRITE_REG(hlp);

    fwrite(&cpuz80->internal_reg, 1, 1, fp);
    fwrite(&cpuz80->iff1, 1, 1, fp);
    fwrite(&cpuz80->iff2, 1, 1, fp);
    fwrite(&cpuz80->im, 1, 1, fp);
    fwrite(&cpuz80->halt, 1, 1, fp);
    fwrite(&cpuz80->ei, 1, 1, fp);
    fwrite(&cpuz80->r_top, 1, 1, fp);
}

void sms_z80_read_context(FILE *fp) {
    if(cpuz80 == NULL)
        return;

    READ_REG(af);
    READ_REG(bc);
    READ_REG(de);
    READ_REG(hl);
    READ_REG(ix);
    READ_REG(iy);
    READ_REG(pc);
    READ_REG(sp);
    READ_REG(ir);
    READ_REG(afp);
    READ_REG(bcp);
    READ_REG(dep);
    READ_REG(hlp);

    fread(&cpuz80->internal_reg, 1, 1, fp);
    fread(&cpuz80->iff1, 1, 1, fp);
    fread(&cpuz80->iff2, 1, 1, fp);
    fread(&cpuz80->im, 1, 1, fp);
    fread(&cpuz80->halt, 1, 1, fp);
    fread(&cpuz80->ei, 1, 1, fp);
    fread(&cpuz80->r_top, 1, 1, fp);
}
