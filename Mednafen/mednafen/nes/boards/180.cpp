#include "mapinc.h"

static uint8 latch;

static void Sync(void)
{
 setprg16(0xc000, latch & 0xF);
}

static DECLFW(Mapper180_write)
{
 latch = V & 0xF;
 Sync();
}

static void Power(CartInfo *info)
{
 setchr8(0);
 setprg16(0x8000, 0);
 latch = 0xF;
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

int Mapper180_Init(CartInfo *info)
{
 SetWriteHandler(0x8000,0xffff,Mapper180_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;

 return(1);
}

