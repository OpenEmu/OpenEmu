/* Is this an MMC3 workalike piece of hardware, with the addition of
   a register at $4120 or does it have only partial MMC3 functionality?
   A good test would be to see if commands 6 and 7 can change PRG banks
   and of course test the regs >=$c000, on the real cart.
*/
#include "mapinc.h"

static uint8 IRQCount, IRQLatch, IRQa, cmd, regs[8];
static uint8 PRGBank32, Mirroring;

static void Sync(void)
{
 setprg32(0x8000, PRGBank32);
 setchr2(0x0000, regs[0] >> 1);
 setchr2(0x0800, regs[1] >> 1);
 setchr1(0x1000, regs[2]);
 setchr1(0x1400, regs[3]);
 setchr1(0x1800, regs[4]);
 setchr1(0x1c00, regs[5]);
 setmirror((Mirroring & 1) ? MI_H : MI_V);
}

static DECLFW(Mapper189_write)
{
 //if(A>=0xc000) printf("$%04x:$%02x\n",A,V);
 if((A&0xF100)==0x4100) 
 {
  PRGBank32 = V >> 4;
  Sync();
 }
 else if((A&0xF100)==0x6100) 
 {
  PRGBank32 = V & 0x3;
  Sync();
 }
 else switch(A&0xE001)
 {
   case 0xa000:Mirroring = V & 1; Sync();break;
   case 0x8000:cmd=V;break; 
   case 0x8001:regs[cmd & 0x7] = V; Sync(); break;
   case 0xc000:IRQLatch=V;break;
   case 0xc001:IRQCount=IRQLatch;break;
   case 0xe000:IRQa=0;X6502_IRQEnd(MDFN_IQEXT);break;
   case 0xe001:IRQa=1;break;
	      break;

 }
}
void m189irq(void)
{
 if(IRQa)
 {
  if(IRQCount)
  {
   IRQCount--;
   if(!IRQCount) X6502_IRQBegin(MDFN_IQEXT);
  }
 }

}

static void Power(CartInfo *info)
{
 IRQCount = IRQLatch = IRQa = 0;
 cmd = 0;
 regs[0] = 0;
 regs[1] = 2;
 regs[2] = 4;
 regs[3] = 5;
 regs[4] = 6;
 regs[5] = 7;
 PRGBank32 = 0;
 Mirroring = 0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(regs, 8),
  SFVAR(PRGBank32),
  SFVAR(IRQCount), SFVAR(IRQLatch), SFVAR(IRQa),
  SFVAR(Mirroring), SFVAR(cmd),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper189_Init(CartInfo *info)
{
 GameHBIRQHook=m189irq;
 SetWriteHandler(0x4120,0xFFFF,Mapper189_write);
 SetReadHandler(0x8000,0xFFFF,CartBR);
 info->Power = Power;
 info->StateAction = StateAction;
 return(1);
}


