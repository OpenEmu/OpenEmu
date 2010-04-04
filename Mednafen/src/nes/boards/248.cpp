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

static uint8 cmd, lpa, prgl[2], Mirroring, IRQLatch, IRQCount, IRQa, CHRBanks[6];

static void PRGSynco(void)
{
 if(lpa&0x80)
 {
  setprg16(0x8000,lpa&0xF);
 }
 else
 {
  setprg8(0x8000,prgl[0]&0x1F);
  setprg8(0xa000,prgl[1]&0x1F);
 }
}
static void DoCHR(void)
{
 setchr2(0x0000, CHRBanks[0]);
 setchr2(0x0800, CHRBanks[1]);
 for(int x = 0; x < 4; x++)
  setchr1(0x1000 + x * 1024, CHRBanks[2 + x]);
}
static DECLFW(Mapper248_writelow)
{
 lpa=V;
 PRGSynco();
}

static DECLFW(Mapper248_write)
{
 switch(A&0xF001)
 {
  case 0xa000:Mirroring = V & 1; setmirror(Mirroring ? MI_H : MI_V);break; // Not sure if this is right.  Mirroring may be hard wired...
  case 0xc000:IRQLatch=V;break;
  case 0xc001:IRQCount=IRQLatch;break;
  case 0xe000:IRQa=0;X6502_IRQEnd(MDFN_IQEXT);break;
  case 0xe001:IRQa=1;break;
  case 0x8000:cmd=V;break;
  case 0x8001:switch(cmd&7)
	      {
	       case 0:CHRBanks[0] = V >> 1;DoCHR();break;
	       case 1:CHRBanks[1] = V >> 1;DoCHR();break;
	       case 2:CHRBanks[2] = V;DoCHR();break;
	       case 3:CHRBanks[3] = V;DoCHR();break;
	       case 4:CHRBanks[4] = V;DoCHR();break;
	       case 5:CHRBanks[5] = V;DoCHR();break;
	       case 6:prgl[0]=V;PRGSynco();break;
	       case 7:prgl[1]=V;PRGSynco();break;
	      }
	      break;
 }
}

static void Mapper248_hb(void)
{
 if(IRQa)
 {
  IRQCount--;
  if(IRQCount == 0xFF)
  {
   X6502_IRQBegin(MDFN_IQEXT);
   IRQCount = IRQLatch;
  }
 }
}

static void Power(CartInfo *info)
{
 CHRBanks[0] = 0;
 CHRBanks[1] = 1;
 CHRBanks[2] = 4;
 CHRBanks[3] = 5;
 CHRBanks[4] = 6;
 CHRBanks[5] = 7;
 IRQCount = IRQLatch = IRQa = 0;

 Mirroring = (info->mirror & 1) ^ 1;
 setmirror(Mirroring ^ 1);
 cmd = 0;
 lpa = 0;
 prgl[0] = 0;
 prgl[1] = 1;
 setprg16(0xc000, 0x7F);
 PRGSynco();
 DoCHR();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CHRBanks, 6),
  SFARRAY(prgl, 2),
  SFVAR(lpa),
  SFVAR(cmd),
  SFVAR(Mirroring),
  SFVAR(IRQCount),
  SFVAR(IRQLatch),
  SFVAR(IRQa),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  Mirroring &= 1;
  setmirror(Mirroring ^ 1);
  DoCHR();
  PRGSynco();
 }
 return(ret);
}

int Mapper248_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x6000,0x6fff,Mapper248_writelow);
 SetWriteHandler(0x8000,0xffff,Mapper248_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 GameHBIRQHook=Mapper248_hb;

 return(1);
}

