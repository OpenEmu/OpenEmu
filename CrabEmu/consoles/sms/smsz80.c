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

#include <stdio.h>
#include <stdlib.h>
#include "smsz80.h"
#include "CrabZ80.h"

CrabZ80_t *cpuz80 = NULL;

int sms_z80_init(void)  {
    cpuz80 = (CrabZ80_t *)malloc(sizeof(CrabZ80_t));

    if(cpuz80 == NULL)  {
#ifdef DEBUG
        fprintf(stderr, "Out of memory while initializing Z80\n");
#endif
        return -1;
    }

    CrabZ80_init(cpuz80);
    CrabZ80_reset(cpuz80);

    CrabZ80_set_portwrite(cpuz80, sms_port_write);
    CrabZ80_set_portread(cpuz80, sms_port_read);

    return 0;
}

int sms_z80_shutdown(void)  {
    free(cpuz80);

    return 0;
}

void sms_z80_reset(void)    {
    CrabZ80_reset(cpuz80);
}

void sms_z80_assert_irq(void)   {
    CrabZ80_assert_irq(cpuz80, 0xFFFFFFFF);
}

void sms_z80_clear_irq(void)    {
    CrabZ80_clear_irq(cpuz80);
}

void sms_z80_nmi(void)  {
    CrabZ80_pulse_nmi(cpuz80);
}

uint16 sms_z80_get_pc(void) {
    return cpuz80->pc.w;
}

uint32 sms_z80_get_cycles(void) {
    return cpuz80->cycles;
}

void sms_z80_set_mread(uint8 (*mread)(uint16))  {
    CrabZ80_set_memread(cpuz80, mread);
}

void sms_z80_set_readmap(uint8 *readmap[256]) {
    CrabZ80_set_readmap(cpuz80, readmap);
}

void sms_z80_set_mwrite(void (*mwrite)(uint16, uint8))  {
    CrabZ80_set_memwrite(cpuz80, mwrite);
}

void sms_z80_set_pread(uint8 (*pread)(uint16))  {
    CrabZ80_set_portread(cpuz80, pread);
}

void sms_z80_set_pwrite(void (*pwrite)(uint16, uint8))  {
    CrabZ80_set_portwrite(cpuz80, pwrite);
}

void sms_z80_set_mread16(uint16 (*mread)(uint16))   {
    CrabZ80_set_memread16(cpuz80, mread);
}

void sms_z80_set_mwrite16(void (*mwrite)(uint16, uint16))   {
    CrabZ80_set_memwrite16(cpuz80, mwrite);
}

uint32 sms_z80_run(uint32 cycles)   {
    return CrabZ80_execute(cpuz80, cycles);
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
