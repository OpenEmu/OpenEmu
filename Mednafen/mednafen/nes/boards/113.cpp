/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mapinc.h"

/*      I'm getting the feeling this is another "jam two different bank
        switching hardwares into one mapper".
*/        

static uint8 latch;

/* HES 4-in-1 */
static DECLFW(Mapper113_write)
{
	latch = V;

	setprg32(0x8000, (V >> 3) & 0x7);
	setchr8(V & 0x7);
}

      
/*      Deathbots */
static DECLFW(Mapper113_writeh)
{
	latch = ((V & 0x7) << 3);
	setprg32(0x8000, V & 0x7);
}

static void Power(CartInfo *info)
{
 latch = 0;
 setprg32(0x8000, 0);
 setchr8(0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latch),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "M113");
 if(load)
 {
  setprg32(0x8000, (latch >> 3) & 0x7);
  setchr8(latch & 0x7);
 }
 return(ret);
}
int Mapper113_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x4020,0x7fff,Mapper113_write);
 SetWriteHandler(0x8000,0xffff,Mapper113_writeh);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}
