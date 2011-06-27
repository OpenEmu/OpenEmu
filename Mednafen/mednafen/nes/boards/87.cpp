#include "mapinc.h"

static uint8 latch;

static void Sync(void)
{
 setchr8(latch);
}

static DECLFW(Mapper87_write)
{
 latch = V >> 1;
 Sync();
}

static void Power(CartInfo *info)
{
 latch = 0;
 setprg32(0x8000, 0);
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFVAR(latch),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper87_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x6000,0x7fff,Mapper87_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}
