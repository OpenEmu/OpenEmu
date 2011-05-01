/*
    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald

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

uint8 (*MD_ExtRead8)(uint32 address) = NULL;
uint16 (*MD_ExtRead16)(uint32 address) = NULL;
void (*MD_ExtWrite8)(uint32 address, uint8 value) = NULL;
void (*MD_ExtWrite16)(uint32 address, uint16 value) = NULL;

uint8 *cart_rom = NULL; //[0x400000];   /* Cartridge ROM */
uint8 work_ram[0x10000];    /* 68K work RAM */
uint8 zram[0x2000];         /* Z80 work RAM */
uint8 zbusreq;              /* /BUSREQ from Z80 */
uint8 zreset;               /* /RESET to Z80 */
uint8 zbusack;              /* /BUSACK to Z80 */
uint8 zirq;                 /* /IRQ to Z80 */
uint32 zbank;               /* Address of Z80 bank window */

uint8 gen_running;

c68k_struc Main68K;
MDVDP MainVDP;

/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown functions                                          */
/*--------------------------------------------------------------------------*/
int vdp_int_ack_callback(int int_level)
{
 return(MainVDP.IntAckCallback(int_level));
}

void gen_init(void)
{
 C68k_Init(&Main68K, vdp_int_ack_callback);
 C68k_Set_TAS_Hack(&Main68K, 1);

 C68k_Set_ReadB(&Main68K, MD_ReadMemory8);
 C68k_Set_ReadW(&Main68K, MD_ReadMemory16);

 C68k_Set_WriteB(&Main68K, MD_WriteMemory8);
 C68k_Set_WriteW(&Main68K, MD_WriteMemory16);
}

void gen_reset(void)
{
    /* Clear RAM */
    memset(work_ram, 0, sizeof(work_ram));
    memset(zram, 0, sizeof(zram));

    gen_running = 1;
    zreset  = 0;    /* Z80 is reset */
    zbusreq = 0;    /* Z80 has control of the Z bus */
    zbusack = 1;    /* Z80 is busy using the Z bus */
    zbank   = 0;    /* Assume default bank is 000000-007FFF */
    zirq    = 0;    /* No interrupts occuring */

    gen_io_reset();

    C68k_Reset(&Main68K);
    z80_reset();
}

void gen_shutdown(void)
{

}

/*--------------------------------------------------------------------------*/
/* Bus controller chip functions                                            */
/*--------------------------------------------------------------------------*/

int gen_busack_r(void)
{
	//printf("busack_r: %d, %d\n", zbusack, md_timestamp);
    return (zbusack & 1);
}

void gen_busreq_w(int state)
{
    //printf("BUSREQ: %d, %d, %d\n", state, md_timestamp, scanline);
    zbusreq = (state & 1);
    zbusack = 1 ^ (zbusreq & zreset);
}

void gen_reset_w(int state)
{
	//printf("ZRESET: %d, %d\n", state, md_timestamp);
    zreset = (state & 1);
    zbusack = 1 ^ (zbusreq & zreset);

    MDSound_SetYM2612Reset(!zreset);

    if(zreset == 0)
    {
     z80_reset();
    }
}


void gen_bank_w(int state)
{
    zbank = ((zbank >> 1) | ((state & 1) << 23)) & 0xFF8000;
}

}
