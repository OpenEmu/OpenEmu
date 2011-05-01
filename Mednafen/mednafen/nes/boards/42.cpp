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

static uint8 regs[4], CHRBank8;
static uint16 IRQCount;

static void Sync(void)
{
 setchr8(CHRBank8);
 setprg8(0x6000, regs[0] & 0xF);
 setmirror((regs[1] & 0x08) ? MI_H : MI_V);
}

static DECLFW(NicolWrite)
{
 CHRBank8 = V;
 Sync();
}

static DECLFW(Write)
{
 regs[A & 3] = V;
 if((A & 3) == 2)
  if(!(V & 2))
  {
   X6502_IRQEnd(MDFN_IQEXT);
   IRQCount = 0;
  }
 Sync();
}

static void IRQHook(int a)
{
 if(regs[2] & 2)
 {
  IRQCount += a;
  if(IRQCount >= 24576)
  {
   X6502_IRQBegin(MDFN_IQEXT);
   IRQCount = 24576;
  }
 }
}

static void Power(CartInfo *info)
{
 int x;

 CHRBank8 = 0;

 IRQCount = 0;

 for(x = 0; x < 4; x++)
  regs[x] = 0;

 setprg8(0x8000, 0xC);
 setprg8(0xa000, 0xD);
 setprg8(0xc000, 0xE);
 setprg8(0xe000, 0xF);

 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(regs, 4),
  SFVAR(IRQCount),
  SFVAR(CHRBank8),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  Sync();
 }

 return(ret);
}

int BioMiracleA_Init(CartInfo *info)
{
 MapIRQHook = IRQHook;
 info->Power = Power;
 info->StateAction = StateAction;

 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x8000, 0x9FFF, NicolWrite);
 SetWriteHandler(0xe000, 0xFFFF, Write); 

 return(1);
}
