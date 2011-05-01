/*
    Copyright (C) 1999, 2000, 2001, 2002, 2003  Charles MacDonald

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "shared.h"

namespace MDFN_IEN_MD
{

unsigned int m68k_read_bus_8(unsigned int address)
{
    uint16 temp = m68k_read_bus_16(address);
    return ((address & 1) ? (temp & 0xFF) : (temp >> 8));
}

unsigned int m68k_read_bus_16(unsigned int address)
{
    uint16 temp = 0x4e71;

    if(address >= 0xC00000)
    {
        return (temp);
    }
    else
    {
        return (temp & 0xFF00);
    }
}

void m68k_unused_w(unsigned int address, unsigned int value)
{
 printf("Unused %08X = %08X\n", address, value);
}

void m68k_unused_8_w(unsigned int address, unsigned int value)
{
 printf("Unused %08X = %02X (%08X)\n", address, value);
}

void m68k_unused_16_w(unsigned int address, unsigned int value)
{
 printf("Unused %08X = %04X (%08X)\n", address, value);
}

/*
    Functions to handle memory accesses which cause the Genesis to halt
    either temporarily (press RESET button to restart) or unrecoverably
    (cycle power to restart).
*/

void m68k_lockup_w_8(unsigned int address, unsigned int value)
{
    printf("Lockup %08X = %02X (%08X)\n", address, value, C68k_Get_PC(&Main68K));
}

void m68k_lockup_w_16(unsigned int address, unsigned int value)
{
    printf("Lockup %08X = %04X (%08X)\n", address, value, C68k_Get_PC(&Main68K));
}

unsigned int m68k_lockup_r_8(unsigned int address)
{
    printf("Lockup %08X.b (%08X)\n", address, C68k_Get_PC(&Main68K));
    return -1;
}

unsigned int m68k_lockup_r_16(unsigned int address)
{
    printf("Lockup %08X.w (%08X)\n", address, C68k_Get_PC(&Main68K));
    return -1;
}

/*--------------------------------------------------------------------------*/
/* 68000 memory handlers                                                    */
/*--------------------------------------------------------------------------*/

uint8 MD_ReadMemory8(uint32 address)
{
 address &= 0xFFFFFF;

 //printf("Read8: %08x\n", address);

    switch((address >> 21) & 7)
    {
        case 0: /* ROM */
        case 1:
        case 2: /* Unused */
        case 3:
	    return(MD_ExtRead8(address));

        case 7: /* RAM */
            return READ_BYTE_MSB(work_ram, address & 0xFFFF);

        case 5: /* Z80 & I/O */
            if(address <= 0xA0FFFF)
            {
                if(zbusack == 1)
                {
                    /* Z80 controls Z bus */
                    return (m68k_read_bus_8(address));
                }
                else
                {
                    /* Read data from Z bus */
                    switch(address & 0x6000)
                    {
                        case 0x0000: /* RAM */
                        case 0x2000:
                            return (zram[(address & 0x1FFF)]);
    
                        case 0x4000: /* YM2612 */
                            return (MDSound_ReadFM(address & 3));

                        case 0x6000: /* Unused */
                            switch(address & 0xFF00)
                            {
                                case 0x7F00: /* VDP */
                                    m68k_lockup_r_8(address);
        
                                default: /* Unused */
                                    return (0xFF);
                            }
                            break;
                    }
                }
            }
            else
            {
                switch((address >> 8) & 0xFF)
                {
                    case 0x00: /* I/O CHIP */
                        if(address <= 0xA1001F)
                        {
                            return (gen_io_r((address >> 1) & 0x0F));
                        }
                        else
                        {
                            return (m68k_read_bus_8(address));
                        }
                        break;

                    case 0x10: /* MEMORY MODE */
                        return (m68k_read_bus_8(address));

                    case 0x11: /* BUSACK */
                        if((address & 1) == 0)
                        {
                            return (gen_busack_r() | (m68k_read_bus_8(address) & 0xFE));
                        }
                        else
                        return (m68k_read_bus_8(address));

                    case 0x12: /* RESET */
                    case 0x13: /* TIME */
                    case 0x20: /* UNKNOWN */
                    case 0x30: /* UNKNOWN */
                        return (MD_ExtRead8(address));

                    default: /* Unused */
                        return (m68k_lockup_r_8(address));
                }
            }
            break;

        case 6: /* VDP */
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1F)
                {
                    case 0x00: /* DATA */
                    case 0x02:
                        return (MainVDP.vdp_data_r() >> 8);

                    case 0x01: /* DATA */
                    case 0x03:
                        return (MainVDP.vdp_data_r() & 0xFF);

                    case 0x04: /* CTRL */
                    case 0x06:
                        return ((m68k_read_bus_8(address) & 0xFC) | (MainVDP.vdp_ctrl_r() >> 8));

                    case 0x05: /* CTRL */
                    case 0x07:
                        return (MainVDP.vdp_ctrl_r() & 0xFF);

                    case 0x08: /* HVC */
                    case 0x0A:
                    case 0x0C:
                    case 0x0E:
                        return (MainVDP.vdp_hvc_r() >> 8);

                    case 0x09: /* HVC */
                    case 0x0B:
                    case 0x0D:
                    case 0x0F:
                        return (MainVDP.vdp_hvc_r() & 0xFF);

                    case 0x10: /* PSG */
                    case 0x11:
                    case 0x12:
                    case 0x13:
                    case 0x14:
                    case 0x15:
                    case 0x16:
                    case 0x17:
                        return (m68k_lockup_r_8(address));

                    case 0x18: /* Unused */
                    case 0x19:
                    case 0x1A:
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    case 0x1F:
                        return (m68k_read_bus_8(address));
                }
            }
            else
            {
                /* Unused */
                return (m68k_lockup_r_8(address));
            }
            break;

        case 4: /* Unused */
            return (m68k_lockup_r_8(address));
    }

    return -1;
}


uint16 MD_ReadMemory16(uint32 address)
{
 if(address & 1)
 {
  // TODO: Generate 68K exception(and remove address &= ~1)
  printf("16-bit unaligned read: %08x\n", address);
  address &= ~1;
 }
 address &= 0xFFFFFF;

 //printf("Read16: %08x\n", address);

    switch((address >> 21) & 7)
    {

        case 0: /* ROM */
        case 1:
        case 2:
        case 3:
            return(MD_ExtRead16(address));

        case 7: /* RAM */
            return READ_WORD_MSB(work_ram, address & 0xFFFF);

        case 5: /* Z80 & I/O */
            if(address <= 0xA0FFFF)
            {
                if(zbusack == 1)
                {
                    return (m68k_read_bus_16(address));
                }
                else
                {
                    uint8 temp;

                    switch(address & 0x6000)
                    {
                        case 0x0000: /* RAM */
                        case 0x2000:
                            temp = zram[address & 0x1FFF];
                            return (temp << 8 | temp);
    
                        case 0x4000: /* YM2612 */
                            temp = MDSound_ReadFM(address & 3);
                            return (temp << 8 | temp);

                        case 0x6000:
                            switch(address & 0xFF00)
                            {
                                case 0x7F00: /* VDP */
                                    m68k_lockup_r_16(address);
        
                                default: /* Unused */
                                    return (0xFFFF);
                            }
                            break;
                    }
                }
            }
            else
            {
                if(address <= 0xA1001F)
                {
                    uint8 temp = gen_io_r((address >> 1) & 0x0F);
                    return (temp << 8 | temp);
                }
                else
                {
                    switch((address >> 8) & 0xFF)
                    {
                        case 0x10: /* MEMORY MODE */
                            return (m68k_read_bus_16(address));

                        case 0x11: /* BUSACK */
                            return ((m68k_read_bus_16(address) & 0xFEFF) | (gen_busack_r() << 8));

                        case 0x12: /* RESET */
                        case 0x13: /* TIME */
                        case 0x20: /* UNKNOWN */
                        case 0x30: /* UNKNOWN */
	                        return(MD_ExtRead16(address));

                        default: /* Unused */
                            return (m68k_lockup_r_16(address));
                    }
                }
            }
            break;

        case 6:
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1F)
                {
                    case 0x00: /* DATA */
                    case 0x02:
                        return (MainVDP.vdp_data_r());

                    case 0x04: /* CTRL */                  
                    case 0x06:                             
                        return (MainVDP.vdp_ctrl_r() | (m68k_read_bus_16(address) & 0xFC00));

                    case 0x08: /* HVC */
                    case 0x0A:
                    case 0x0C:
                    case 0x0E:
                        return (MainVDP.vdp_hvc_r());

                    case 0x10: /* PSG */
                    case 0x12:
                    case 0x14:
                    case 0x16:
                        return (m68k_lockup_r_16(address));

                    case 0x18: /* Unused */
                    case 0x1A:
                    case 0x1C:
                    case 0x1E:
                        return (m68k_read_bus_16(address));
                }
            }
            else
            {
                return (m68k_lockup_r_16(address));
            }
            break;

        case 4:
            return (m68k_lockup_r_16(address));
    }

    return (0xA5A5);
}


void MD_WriteMemory8(uint32 address, uint8 value)
{
 address &= 0xFFFFFF;

 //printf("Write8: %08x %02x\n", address & 0xFFFFFF, value);

    switch((address >> 21) & 7)
    {
        case 7:
            WRITE_BYTE_MSB(work_ram, address & 0xFFFF, value);
            return;

        case 6:
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1F)
                {
                    case 0x00: /* DATA */
                    case 0x01:
                    case 0x02:
                    case 0x03:
                        MainVDP.vdp_data_w(value << 8 | value);
                        return;

                    case 0x04: /* CTRL */
                    case 0x05:
                    case 0x06:
                    case 0x07:
                        MainVDP.vdp_ctrl_w(value << 8 | value);
                        return;

                    case 0x08: /* HVC */
                    case 0x09:
                    case 0x0A:
                    case 0x0B:
                    case 0x0C:
                    case 0x0D:
                    case 0x0E:
                    case 0x0F:
                        m68k_lockup_w_8(address, value);
                        return;

                    case 0x10: /* PSG */
                    case 0x12:
                    case 0x14:
                    case 0x16:
                        m68k_unused_8_w(address, value);
                        return;

                    case 0x11: /* PSG */
                    case 0x13:
                    case 0x15:
                    case 0x17:
                        psg_write(value);
                        return;

                    case 0x18: /* Unused */
                    case 0x19:
                    case 0x1A:
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    case 0x1F:
                        m68k_unused_8_w(address, value);
                        return;
                }
            }
            else
            {
                m68k_lockup_w_8(address, value);
                return;
            }

        case 5:
            if(address <= 0xA0FFFF)
            {
                if(zbusack == 1)
                {
                    m68k_unused_8_w(address, value);
                    return;
                }
                else
                {
                    switch(address & 0x6000)
                    {
                        case 0x0000:
                        case 0x2000:
                            zram[(address & 0x1FFF)] = value;
                            return;
    
                        case 0x4000:
                            fm_write(address & 3, value);
                            return;

                        case 0x6000:
                            switch(address & 0xFF00)
                            {
                                case 0x6000: /* BANK */
                                    gen_bank_w(value & 1);
                                    return;

                                case 0x7F00: /* VDP */
                                    m68k_lockup_w_8(address, value);
                                    return;

                                default: /* Unused */
                                    m68k_unused_8_w(address, value);
                                    return;
                            }
                            break;
                    }
                }
            }
            else
            {
                if(address <= 0xA1001F)
                {
                    /* I/O chip only gets /LWR */
                    if(address & 1)
                        gen_io_w((address >> 1) & 0x0F, value);
                    return;
                }
                else
                {
                    /* Bus control chip registers */
                    switch((address >> 8) & 0xFF)
                    {
                        case 0x10: /* MEMORY MODE */
                            m68k_unused_8_w(address, value);
                            return;

                        case 0x11: /* BUSREQ */
                            if((address & 1) == 0)
                            {
                                gen_busreq_w(value & 1);
                            }
                            else
                            {
                                m68k_unused_8_w(address, value);
                            }
                            return;

                        case 0x12: /* RESET */
                            gen_reset_w(value & 1);
                            return;

                        case 0x13: /* TIME */
                        case 0x20: /* UNKNOWN */
                        case 0x30: /* UNKNOWN */
			    MD_ExtWrite8(address, value);
                            return;

                        default: /* Unused */
                            m68k_lockup_w_8(address, value);
                            return;
                    }
                }
            }
            break;


        case 0: /* ROM */
        case 1: /* ROM */
        case 2: /* Unused */
        case 3:
	    MD_ExtWrite8(address, value);
            //m68k_unused_8_w(address, value);
            return;

        case 4: /* Unused */
            m68k_lockup_w_8(address, value);
            return;
    }

}


void MD_WriteMemory16(uint32 address, uint16 value) 
{
 if(address & 1)
 {
  // TODO: Generate 68K exception(and remove address &= ~1)
  printf("16-bit unaligned write: %08x %04x\n", address, value);
  address &= ~1;
 }
 address &= 0xFFFFFF;

 //printf("Write16: %08x %04x\n", address & 0xFFFFFF, value);

    switch((address >> 21) & 7)
    {
        case 0: /* ROM */
        case 1: /* ROM */
        case 2: /* Unused */
        case 3:
            MD_ExtWrite16(address, value);
            return;

        case 4: /* Unused */
            m68k_lockup_w_16(address, value);
            return;

        case 5: /* Z80 area, I/O chip, miscellaneous. */
            if(address <= 0xA0FFFF)
            {
                /* Writes are ignored when the Z80 hogs the Z-bus */
                if(zbusack == 1) {
                    m68k_unused_8_w(address, value);
                    return;
                }

                /* Write into Z80 address space */
                switch(address & 0x6000)
                {
                    case 0x0000: /* Work RAM */
                    case 0x2000: /* Work RAM */
                        zram[(address & 0x1FFF)] = (value >> 8) & 0xFF;
                        return;
    
                    case 0x4000: /* YM2612 */
                        fm_write(address & 3, (value >> 8) & 0xFF);
                        return;

                    case 0x6000: /* Bank register and VDP */
                        switch(address & 0x7F00)
                        {
                            case 0x6000: /* Bank register */
                                gen_bank_w((value >> 8) & 1);
                                return;

                             case 0x7F00: /* VDP registers */
                                m68k_lockup_w_16(address, value);
                                return;

                             default: /* Unused */
                                m68k_unused_8_w(address, value);
                                return;
                        }
                        break;
                }
            }
            else
            {
                /* I/O chip */
                if(address <= 0xA1001F)
                {
                    gen_io_w((address >> 1) & 0x0F, value & 0x00FF);
                    return;
                }
                else
                {
                    /* Bus control chip registers */
                    switch((address >> 8) & 0xFF)
                    {
                        case 0x10: /* MEMORY MODE */
                            m68k_unused_16_w(address, value);
                            return;

                        case 0x11: /* BUSREQ */
                            gen_busreq_w((value >> 8) & 1);
                            return;

                        case 0x12: /* RESET */
                            gen_reset_w((value >> 8) & 1);
                            return;

                        case 0x13: /* TIME */
                        case 0x20: /* UNKNOWN */
                        case 0x30: /* UNKNOWN */
                            MD_ExtWrite16(address, value);
                            return;

                        default: /* Unused */
                            m68k_lockup_w_16(address, value);
                            return;
                    }
                }
            }
            break;

        case 6: /* VDP */
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1C)
                {
                    case 0x00: /* DATA */
                        MainVDP.vdp_data_w(value);
                        return;

                    case 0x04: /* CTRL */
                        MainVDP.vdp_ctrl_w(value);
                        return;

                    case 0x08: /* HV counter */
                    case 0x0C: /* HV counter */
                        m68k_lockup_w_16(address, value);
                        return;

                    case 0x10: /* PSG */
                    case 0x14: /* PSG */
                        psg_write(value & 0xFF);
                        return;

                    case 0x18: /* Unused */
                    case 0x1C: /* Unused */
                        m68k_unused_8_w(address, value);
                        return;
                }
            }
            else
            {
                /* Invalid address */
                m68k_lockup_w_16(address, value);
            }
            break;

        case 7: /* Work RAM */
            WRITE_WORD_MSB(work_ram, address & 0xFFFF, value);
            return;
    }
}

}
