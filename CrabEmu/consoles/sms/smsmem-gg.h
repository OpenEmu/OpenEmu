/*
    This file is part of CrabEmu.

    Copyright (C) 2009 Lawrence Sebald

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

#ifndef SMSMEM_GG_H
#define SMSMEM_GG_H

CLINKAGE

extern void sms_mem_remap_page0_gg_bios(void);

extern void sms_gg_port_write(uint16 port, uint8 data);
extern uint8 sms_gg_port_read(uint16 port);

ENDCLINK

#endif /* !SMSMEM_GG_H */
