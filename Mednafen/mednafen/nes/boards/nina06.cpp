/* Mednafen - Multi-system Emulator
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

static uint8 latch;
static void Sync(void)
{
 setprg32(0x8000, (latch >> 3) & 1);
 setchr8(latch & 0x7);
}

static DECLFW(Write)
{
 if(A<0x8000 && ((A^0x4100)==0))
 {
  latch = V;
  Sync();
 }
 if(A >= 0x8000)
 {
  latch &= ~0x7;
  latch |= V & 0x7;
  Sync();
 }
}

static void Power(CartInfo *info)
{
 latch = 0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 { SFVAR(latch), SFEND };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();

 return(ret);
}

int NINA06_Init(CartInfo *info)
{
 info->StateAction = StateAction;
 info->Power = Power;
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 SetWriteHandler(0x4020,0x5fff, Write);
 SetWriteHandler(0x8000, 0xFFFF, Write);

 return(1);
}

