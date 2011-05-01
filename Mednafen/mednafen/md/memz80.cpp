/*
    memz80.c --
    Memory handlers for Z80 memory and port access, and the Z80 to
    VDP interface.
*/

#define LOG_PORT 0      /* 1= Log Z80 I/O port accesses */

#include "shared.h"

namespace MDFN_IEN_MD
{


uint8 MD_Z80_ReadByte(uint16 address)
{
    switch((address >> 13) & 7)
    {
        case 0: /* Work RAM */
        case 1:
            return zram[address & 0x1FFF];

        case 2: /* YM2612 */
            return MDSound_ReadFM(address & 3);

        case 3: /* VDP */
	    //    printf("Z80 Read: %08x, %04x, %08x\n", zbank, address, zbank | address);
            if((address & 0xFF00) == 0x7F00)
                return z80_vdp_r(address);
            return 0xFF;

        default: /* V-bus bank */
            return z80_read_banked_memory(zbank | (address & 0x7FFF));
    }

    return 0xFF;
}


void MD_Z80_WriteByte(uint16 address, uint8 data)
{
    switch((address >> 13) & 7)
    {
        case 0: /* Work RAM */
        case 1: 
            zram[address & 0x1FFF] = data;
            return;

        case 2: /* YM2612 */
            fm_write(address & 3, data);
            return;

        case 3: /* Bank register and VDP */
	    //printf("Z80 Write: %08x, %04x, %08x\n", zbank, address, zbank | address);
            switch(address & 0xFF00)
            {
                case 0x6000:
                    gen_bank_w(data & 1);
                    return;

                case 0x7F00:
                    z80_vdp_w(address, data);
                    return;

                default:
                    z80_unused_w(address, data);
                    return;
            }
            return;

        default: /* V-bus bank */
            z80_write_banked_memory(zbank | (address & 0x7FFF), data);
            return;
    }
}


int z80_vdp_r(int address)
{
    switch(address & 0xFF)
    {
        case 0x00: /* VDP data port */
        case 0x02:
            return (MainVDP.vdp_data_r() >> 8) & 0xFF;
                        
        case 0x01: /* VDP data port */ 
        case 0x03:
            return (MainVDP.vdp_data_r() & 0xFF);

        case 0x04: /* VDP control port */
        case 0x06:
            return (0xFF | ((MainVDP.vdp_ctrl_r() >> 8) & 3));
                        
        case 0x05: /* VDP control port */
        case 0x07:
            return (MainVDP.vdp_ctrl_r() & 0xFF);

        case 0x08: /* HV counter */
        case 0x0A:
        case 0x0C:
        case 0x0E:
            return (MainVDP.vdp_hvc_r() >> 8) & 0xFF;

        case 0x09: /* HV counter */
        case 0x0B:
        case 0x0D:
        case 0x0F:
            return (MainVDP.vdp_hvc_r() & 0xFF);

        case 0x10: /* Unused (PSG) */
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            return z80_lockup_r(address);

        case 0x18: /* Unused */
        case 0x19:
        case 0x1A:
        case 0x1B:
            return z80_unused_r(address);

        case 0x1C: /* Unused (test register) */
        case 0x1D:
        case 0x1E:
        case 0x1F:
            return z80_unused_r(address);

        default: /* Invalid VDP addresses */
            return z80_lockup_r(address);
    }

    return 0xFF;
}


void z80_vdp_w(int address, int data)
{
    switch(address & 0xFF)
    {
        case 0x00: /* VDP data port */
        case 0x01: 
        case 0x02:
        case 0x03:
            MainVDP.vdp_data_w(data << 8 | data);
            return;

        case 0x04: /* VDP control port */
        case 0x05:
        case 0x06:
        case 0x07:
            MainVDP.vdp_ctrl_w(data << 8 | data);
            return;

        case 0x08: /* Unused (HV counter) */
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            z80_lockup_w(address, data);
            return;

        case 0x11: /* PSG */
        case 0x13:
        case 0x15:
        case 0x17:
            psg_write(data);
            return;

        case 0x10: /* Unused */
        case 0x12:
        case 0x14:
        case 0x16:
            z80_unused_w(address, data);

        case 0x18: /* Unused */
        case 0x19:
        case 0x1A:
        case 0x1B:
            z80_unused_w(address, data);
            return;

        case 0x1C: /* Test register */
        case 0x1D: 
        case 0x1E:
        case 0x1F:
            MainVDP.vdp_test_w(data << 8 | data);
            return;

        default: /* Invalid VDP addresses */
            z80_lockup_w(address, data);
            return;
    }
}


/*
    Port handlers. Ports are unused when not in Mark III compatability mode.

    Games that access ports anyway:
    - Thunder Force IV reads port $BF in its interrupt handler.
*/

uint8 MD_Z80_ReadPort(uint16 address)
{
#if LOG_PORT
    error("Z80 read port %04X (%04X)\n", port, z80_get_reg(Z80_PC));
#endif    
    return 0xFF;
}

void MD_Z80_WritePort(uint16 address, uint8 data)
{
#if LOG_PORT
    error("Z80 write %02X to port %04X (%04X)\n", data, port, z80_get_reg(Z80_PC));
#endif
}


/*
    Handlers for access to unused addresses and those which make the
    machine lock up.
*/
void z80_unused_w(int address, int data)
{
    printf("Z80 unused write %04X = %02X (%04X)\n", address, data, z80_getpc());
}

int z80_unused_r(int address)
{
    printf("Z80 unused read %04X (%04X)\n", address, z80_getpc());
    return 0xFF;
}

void z80_lockup_w(int address, int data)
{
    printf("Z80 lockup write %04X = %02X (%04X)\n", address, data, z80_getpc());    
    gen_running = 0;
    //z80_end_timeslice();
    // FIXME/TODO
}

int z80_lockup_r(int address)
{
    printf("Z80 lockup read %04X (%04X)\n", address, z80_getpc());
    gen_running = 0;
    //z80_end_timeslice();
    // FIXME/TODO
    return 0xFF;
}

}
