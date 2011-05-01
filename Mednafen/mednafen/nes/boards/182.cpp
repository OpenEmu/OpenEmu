#include "mapinc.h"

static uint8 IRQCount, IRQa, Mirroring, cmd, regs[8];

static void Sync(void)
{
 setmirror((Mirroring & 1) ^ 1);
 setchr2(0x0000, regs[0] >> 1);
 setchr1(0x1400, regs[1]);
 setchr2(0x0800, regs[2] >> 1);
 setchr1(0x1c00, regs[3]);
 setprg8(0x8000, regs[4]);
 setprg8(0xa000, regs[5]);
 setchr1(0x1000, regs[6]);
 setchr1(0x1800, regs[7]);
}

static DECLFW(Mapper182_write)
{
  switch(A&0xf003)
  {
   case 0xe003:IRQCount=V;IRQa=1;X6502_IRQEnd(MDFN_IQEXT);break;
   case 0x8001:Mirroring = V&1; Sync();break;
   case 0xA000:cmd = V; break;
   case 0xC000: regs[cmd & 7] = V; Sync(); break;
  }
}

static void blop(void)
{
 if(IRQa)
  {
   if(IRQCount)
   {
    IRQCount--;
    if(!IRQCount)
    {
	IRQa=0;
	X6502_IRQBegin(MDFN_IQEXT);
    }
   }
  }
}

static void Power(CartInfo *info)
{
 IRQCount = IRQa = Mirroring = cmd = 0;
 for(int x = 0; x < 8; x++)
  regs[x] = 0xFF;

 Sync();
 setprg16(0xc000, ~0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(IRQCount), SFVAR(IRQa), SFVAR(Mirroring), SFVAR(cmd),
  SFARRAY(regs, 8),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper182_Init(CartInfo *info)
{
 SetWriteHandler(0x8000,0xFFFF,Mapper182_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;
 GameHBIRQHook=blop;

 return(1);
}

