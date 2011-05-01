/*
    memvdp.c --
    Memory handlers for when the VDP reads the V-bus during DMA.
*/

#include "shared.h"
#include "cart/cart.h"

namespace MDFN_IEN_MD
{

unsigned int vdp_dma_r(unsigned int address)
{
    switch((address >> 21) & 7)
    {
        case 0: /* Cartridge ROM */
        case 1: 
        case 2: /* Unused */
        case 3:
	    return(MD_ExtRead16(address));
            //return READ_WORD_MSB(cart_rom, address);
            //return 0xFF00;

        case 4: /* Work RAM */
        case 6:
        case 7:
            return READ_WORD_MSB(work_ram, address & 0xFFFF);

        case 5: /* Z80 area and I/O chip */

            /* Z80 area always returns $FFFF */
            if(address <= 0xA0FFFF)
            {
                /* Return $FFFF only when the Z80 isn't hogging the Z-bus.
                   (e.g. Z80 isn't reset and 68000 has the bus) */
                return (zbusack == 0)
                    ? 0xFFFF 
                    : READ_WORD_MSB(work_ram, address & 0xFFFF);
            }
            else

            /* The I/O chip and work RAM try to drive the data bus which
               results in both values being combined in random ways when read.
               We return the I/O chip values which seem to have precedence, */
            if(address <= 0xA1001F)
            {
                uint8 temp = gen_io_r((address >> 1) & 0x0F);
                return (temp << 8 | temp);
            }
            else

            /* All remaining locations access work RAM */
            return READ_WORD_MSB(work_ram, address & 0xFFFF);
    }

    return -1;
}

}
