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
static uint8 CHRRAM[8192];

static void Sync(void)
{
 setprg32(0x8000, latch & 0x7);
 setchr2(0x0000, (latch & 0xF0) >> 4);
}

static DECLFW(Mapper77_write)
{
 latch = V;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latch),
  SFARRAY(CHRRAM, 8192),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

static void Power(CartInfo *info)
{
 latch = 0;
 Sync();
 for(int x = 2; x < 8; x++)
  setchr1r(0x10, x * 0x400, x);
 memset(CHRRAM, 0xFF, 8192);
}

int Mapper77_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;

 SetupCartCHRMapping(0x10, CHRRAM, 8192, 1);
 SetWriteHandler(0x6000,0xffff,Mapper77_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 return(1);
}
