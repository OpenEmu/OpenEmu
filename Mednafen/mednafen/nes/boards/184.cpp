#include "mapinc.h"

static uint8 latch;

static void Sync(void)
{
 setchr4(0x0000, latch & 0xf);
 setchr4(0x1000, latch >> 4);
}

static DECLFW(Mapper184_write)
{
 latch = V;
 Sync();
}

static void Power(CartInfo *info)
{
 latch = 0;
 Sync();
 setprg32(0x8000, 0);
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

int Mapper184_Init(CartInfo *info)
{
 SetWriteHandler(0x6000,0xffff,Mapper184_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;

 return(1);
}

