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

static uint16 IRQCount;
static uint8 IRQa, PRGBank16, WRAM[8192];

static void Sync(void)
{
 setprg16(0x8000, PRGBank16);
}

static DECLFW(Mapper73_write)
{
 //if(A>=0xd000 && A<=0xdfff) 
  X6502_IRQEnd(MDFN_IQEXT);	/* How are IRQs acknowledged on this chip? */
 switch(A&0xF000)
 {
  //default: printf("$%04x:$%02x\n",A,V);break;
  case 0x8000:IRQCount&=0xFFF0;IRQCount|=(V&0xF);break;
  case 0x9000:IRQCount&=0xFF0F;IRQCount|=(V&0xF)<<4;break;
  case 0xa000:IRQCount&=0xF0FF;IRQCount|=(V&0xF)<<8;break;
  case 0xb000:IRQCount&=0x0FFF;IRQCount|=(V&0xF)<<12;break;
  case 0xc000:IRQa=V&2;break;
  case 0xf000:PRGBank16 = V; Sync(); break;
 }
}

static void Mapper73IRQHook(int a)
{
 if(IRQa)
 {
  uint32 tmp = IRQCount;
  tmp += a;
  if(tmp >= 0xFFFF)
  {
   IRQCount&=0xFFFF;
   IRQa=0;
   X6502_IRQBegin(MDFN_IQEXT);
  }
  else
   IRQCount = tmp;
 }
}

static void Power(CartInfo *info)
{
 IRQCount = IRQa = 0;
 PRGBank16 = 0;
 setprg16(0xc000, 0xFF);
 setchr8(0);
 setprg8r(0x10, 0x6000, 0);
 Sync();

 if(!info->battery)
  memset(WRAM, 0xFF, 8192);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(IRQCount), SFVAR(IRQa), SFVAR(PRGBank16),
  SFARRAY(WRAM, 8192),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}
int Mapper73_Init(CartInfo *info)
{
 info->StateAction = StateAction;

 SetupCartPRGMapping(0x10, WRAM, 8192, 1);
 SetWriteHandler(0x8000,0xffff,Mapper73_write);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 info->Power = Power;
 MapIRQHook=Mapper73IRQHook;

 if(info->battery)
 {
  memset(WRAM, 0xFF, 8192);
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 return(1);
}

