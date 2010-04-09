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

#include "smsmem.h"
#include "smsmem-gg.h"
#include "smsvdp.h"
#include "smsz80.h"
#include "sn76489.h"

extern uint8 sms_paging_regs[4];
extern uint8 *sms_rom_page0;
extern uint8 *sms_rom_page1;
extern uint8 *sms_rom_page2;
extern uint8 *sms_cart_rom;
extern uint32 sms_cart_len;
extern uint8 *gg_bios_rom;
extern uint32 gg_bios_len;
extern uint8 sms_memctl;
extern uint16 sms_ioctl_input_mask;
extern uint16 sms_ioctl_output_mask;
extern uint16 sms_ioctl_output_bits;
extern uint8 sms_gg_regs[7];
extern uint16 sms_pad;
extern sn76489_t psg;
extern uint8 *sms_read_map[256];
extern uint8 *sms_write_map[256];
extern uint8 sms_dummy_arear[256];
extern uint8 sms_dummy_areaw[256];

typedef void (*remap_page_func)();
remap_page_func sms_mem_remap_page[4];

void sms_mem_remap_page0_gg_bios(void)  {
    int i;

    if(sms_paging_regs[1] && sms_cart_len > 0x4000) {
        sms_rom_page0 = &sms_cart_rom[0x4000 *
            (sms_paging_regs[1] % (sms_cart_len / 0x4000))];
    }
    else    {
        sms_rom_page0 = sms_cart_rom;
    }

    i = 0x04;
    sms_read_map[0] = gg_bios_rom;
    sms_read_map[1] = gg_bios_rom + 0x100;
    sms_read_map[2] = gg_bios_rom + 0x200;
    sms_read_map[3] = gg_bios_rom + 0x300;
    sms_write_map[0] = sms_dummy_areaw;
    sms_write_map[1] = sms_dummy_areaw;
    sms_write_map[2] = sms_dummy_areaw;
    sms_write_map[3] = sms_dummy_areaw;

    for(; i < 0x40; ++i)    {
        sms_read_map[i] = sms_rom_page0 + (i << 8);
        sms_write_map[i] = sms_dummy_areaw;
    }

    sms_z80_set_readmap(sms_read_map);
}

static void sms_mem_gg_handle_memctl(uint8 data)    {
    if(sms_memctl == data)
        return;

    sms_mem_handle_memctl(data & SMS_MEMCTL_BIOS);

    if(!(data & SMS_MEMCTL_BIOS) && gg_bios_rom != NULL)    {
        sms_mem_remap_page[1] = &sms_mem_remap_page0_gg_bios;
        sms_mem_remap_page0_gg_bios();
    }

    sms_memctl = data;
}

void sms_gg_port_write(uint16 port, uint8 data) {
    port &= 0xFF;

    if(port < 0x07) {
        sms_gg_regs[port] = data;

        if(port == 0x06)    {
            sn76489_set_output_channels(&psg, data);
        }
    }
    else if(port < 0x40)    {
        if(port & 0x01) {
            /* I/O Control register */
            sms_mem_handle_ioctl(data);
        }
        else    {
            /* Memory Control register */
            sms_mem_gg_handle_memctl(data);
        }
    }
    else if(port < 0x80)    {
        /* SN76489 PSG */
        sn76489_write(&psg, data);
    }
    else if(port < 0xC0)    {
        if(port & 0x01) {
            /* VDP Control port */
            sms_vdp_ctl_write(data);
        }
        else    {
            /* VDP Data port */
            sms_vdp_data_write(data);
        }
    }
    else    {
    }
}

uint8 sms_gg_port_read(uint16 port) {
    port &= 0xFF;

    if(port < 0x07) {
        return sms_gg_regs[port];
    }
    else if(port < 0x40)    {
        return 0xFF;
    }
    else if(port < 0x80)    {
        if(port & 0x01) {
            return 0;
        }
        else    {
            return sms_vdp_vcnt_read();
        }
    }
    else if(port < 0xC0)    {
        if(port & 0x01) {
            return sms_vdp_status_read();
        }
        else    {
            return sms_vdp_data_read();
        }
    }
    else if(port == 0xC0 || port == 0xDC)   {
        /* I/O port A/B register */
        return ((sms_pad & sms_ioctl_input_mask) |
                (sms_ioctl_output_bits & sms_ioctl_output_mask)) & 0xFF;
    }
    else if(port == 0xC1 || 0xDD)   {
        /* I/O port B/misc register */
        return (((sms_pad & sms_ioctl_input_mask) |
                 (sms_ioctl_output_bits & sms_ioctl_output_mask)) >> 8) & 0xFF;
    }
    else    {
        return 0xFF;
    }
}
