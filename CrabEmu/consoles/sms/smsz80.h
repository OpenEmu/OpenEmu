/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2008, 2009 Lawrence Sebald

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

#ifndef SMSZ80_H
#define SMSZ80_H

#include <stdio.h>
#include "CrabEmu.h"
#include "smsmem.h"

CLINKAGE

extern void sms_z80_reset(void);
extern uint32 sms_z80_run(uint32 cycles);
extern void sms_z80_assert_irq(void);
extern void sms_z80_clear_irq(void);
extern void sms_z80_nmi(void);
extern uint16 sms_z80_get_pc(void);
extern uint32 sms_z80_get_cycles(void);

extern void sms_z80_set_mread(uint8 (*mread)(uint16));
extern void sms_z80_set_mwrite(void (*mwrite)(uint16, uint8));
extern void sms_z80_set_pread(uint8 (*pread)(uint16));
extern void sms_z80_set_pwrite(void (*pwrite)(uint16, uint8));
extern void sms_z80_set_mread16(uint16 (*mread)(uint16));
extern void sms_z80_set_mwrite16(void (*mwrite)(uint16, uint16));
extern void sms_z80_set_readmap(uint8 *readmap[256]);

extern int sms_z80_init(void);
extern int sms_z80_shutdown(void);

extern void sms_z80_write_context(FILE *fp);
extern void sms_z80_read_context(FILE *fp);

ENDCLINK

#endif /* !SMSZ80_H */
