/*
    membnk.c --
    Memory handlers Z80 access to the banked V-bus address space.
*/

#include "shared.h"
#include "cart/cart.h"

namespace MDFN_IEN_MD
{

void z80_write_banked_memory(unsigned int address, unsigned int data)
{
    //printf("Z80 Banked Write: %02x, %08x, %02x\n", address >> 21, address, data);

    switch((address >> 21) & 7)
    {
        case 0: /* Cartridge ROM */
        case 1:
	    MD_ExtWrite8(address, data);
            return;

        case 2: /* Unused */
        case 3:
            MD_ExtWrite8(address, data);
            return;

        case 4: /* Unused (lockup) */
            z80bank_lockup_w(address, data);
            return;

        case 5: /* Z80, I/O chip, etc. */
            if(address <= 0xA0FFFF)
            {
                z80bank_lockup_w(address, data);
                return;
            }
            else
            {
                switch((address >> 8) & 0xFF)
                {
                    case 0x00: /* I/O chip */
                        if(address <= 0xA1001F)
                            gen_io_w((address >> 1) & 0x0F, data);
                        else
                            z80bank_unused_w(address, data);
                        return;

                    case 0x10: /* DRAM refresh */
                        z80bank_unused_w(address, data);
                        return;

                    case 0x11: /* /BUSREQ */
                        if(address & 1)
                            z80bank_unused_w(address, data);
                        else
                            gen_busreq_w(data & 1);
                        return;

                    case 0x12: /* /RESET (w) */
                        if(address & 1)
                            z80bank_unused_w(address, data);
                        else
                            gen_reset_w(data & 1);
                        return;

                    case 0x13: /* /TIME region */
                    case 0x20: /* ? */
                    case 0x30: /* ? */
                        MD_ExtWrite8(address, data);
                        return;

                    default: /* Invalid */
                        z80bank_lockup_w(address, data);
                        return;
                }
            }
            return;

        case 6: /* VDP */
            z80bank_vdp_w(address, data);
            return;

        case 7: /* Work RAM */
            WRITE_BYTE_MSB(work_ram, address & 0xFFFF, data);
            return;
    }
}


int z80_read_banked_memory(unsigned int address)
{
    //printf("Z80 Banked Read: %02x, %08x\n", address >> 21, address);

    switch((address >> 21) & 7)
    {
        case 0: /* Cartridge ROM */
        case 1:
	    return(MD_ExtRead8(address));

        case 2: /* Unused */
        case 3:
	    return(MD_ExtRead8(address));

        case 4: /* Unused (lockup) */
            return z80bank_lockup_r(address);

        case 5: /* Z80, I/O chip, etc.*/
            if(address <= 0xA0FFFF)
            {
                return z80bank_lockup_r(address);
            }
            else
            {
                switch((address >> 8) & 0xFF)
                {        
                    case 0x00: /* I/O chip */
                        if(address <= 0xA1001F)
                            return gen_io_r((address >> 1) & 0x0F);
                        else
                            return z80bank_unused_r(address);
                        break;

                    case 0x10: /* Unused */
                        return z80bank_unused_r(address);

                    case 0x11: /* /BUSACK from Z80 */
                        /* The Z80 can't read this bit (it would be halted
                           when the bit was zero) so we always return '1'. */
                        return 0xFF;

                    case 0x12: /* Unused */
                    case 0x13: /* /TIME region */
                    case 0x20: /* Unused */
                    case 0x30: /* Unused */
			return(MD_ExtRead8(address));

                    default: /* Lockup */
                        return z80bank_lockup_r(address);
                }
            }
            break;

        case 6: /* VDP */
            return z80bank_vdp_r(address);

        case 7: /* Work RAM - can't be read on some Genesis models (!) */
            return 0xFF;
    }

    return (-1);
}


void z80bank_vdp_w(int address, int data)
{
    if((address & 0xE700E0) == 0xC00000)
    {
        switch(address & 0x1F)
        {
            case 0x00: /* Data port */
            case 0x01:
            case 0x02:
            case 0x03:
                MainVDP.vdp_data_w(data << 8 | data);
                return;

            case 0x04: /* Control port */
            case 0x05:
            case 0x06:
            case 0x07:
                MainVDP.vdp_ctrl_w(data << 8 | data);
                return;

            case 0x08: /* Lockup (HVC) */
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
            case 0x0E:
            case 0x0F:
                z80bank_lockup_w(address, data);
                return;

            case 0x10: /* Unused */
            case 0x12:
            case 0x14:
            case 0x16:
                z80bank_unused_w(address, data);
                return;

            case 0x11: /* PSG */
            case 0x13:
            case 0x15:
            case 0x17:
                psg_write(data);
                return;

            case 0x18: /* Unused */
            case 0x19:
            case 0x1A:
            case 0x1B:
                z80bank_unused_w(address, data);
                return;

            case 0x1C: /* Test register */
            case 0x1D:
            case 0x1E:
            case 0x1F:
                MainVDP.vdp_test_w(data << 8 | data);
                return;
        }
    }
    else
    {
        /* Invalid VDP address */
        z80bank_lockup_w(address, data);
        return;
    }
}


int z80bank_vdp_r(int address)
{
    if((address & 0xE700E0) == 0xC00000)
    {
        switch(address & 0x1F)
        {
            case 0x00: /* Data */
            case 0x02:
                return (MainVDP.vdp_data_r() >> 8) & 0xFF;

            case 0x01: /* Data */
            case 0x03:
                return MainVDP.vdp_data_r() & 0xFF;

            case 0x04: /* Control */
            case 0x06:
                return (0xFC | (MainVDP.vdp_ctrl_r() >> 8)) & 0xFF;

            case 0x05: /* Control */
            case 0x07:
                return MainVDP.vdp_ctrl_r() & 0xFF;

            case 0x08: /* HVC */
            case 0x0A:
            case 0x0C:
            case 0x0E:
                return (MainVDP.vdp_hvc_r() >> 8) & 0xFF;

            case 0x09: /* HVC */
            case 0x0B:
            case 0x0D:
            case 0x0F:
                return MainVDP.vdp_hvc_r() & 0xFF;

            case 0x10: /* Lockup */
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
                return z80bank_lockup_r(address);

            case 0x18: /* Unused */
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
            case 0x1F:
                return (z80bank_unused_r(address) | 0xFF);
        }
    }
    else
    {
        /* Invalid VDP address */
        return z80bank_lockup_r(address);
    }

    return 0xFF;
}




/*
    Handlers for access to unused addresses and those which make the
    machine lock up.
*/
void z80bank_unused_w(int address, int data)
{
    error("Z80 bank unused write %06X = %02X (%04X)\n", address, data, z80_get_reg(Z80_PC));
}

int z80bank_unused_r(int address)
{
    error("Z80 bank unused read %06X (%04X)\n", address, z80_get_reg(Z80_PC));
    return (address & 1) ? 0x00 : 0xFF;
}

void z80bank_lockup_w(int address, int data)
{
    printf("Z80 bank lockup write %06X = %02X (%04X)\n", address, data, z80_getpc()); //_reg(Z80_PC));
    gen_running = 0;
    // FIXME/TODO
    //z80_end_timeslice();
}

int z80bank_lockup_r(int address)
{
    printf("Z80 bank lockup read %06X (%04X)\n", address, z80_getpc()); //reg(Z80_PC));
    gen_running = 0;
    //z80_end_timeslice();
    // FIXME/TODO
    return 0xFF;
}

}
