#include "mapinc.h"

static uint8 latch;

static void Sync(void)
{
 setprg32(0x8000, latch >> 1);
 setchr8(latch & 0x07);
}

static DECLFW(m107w)
{
 latch = V;
 Sync();
}

static void Power(CartInfo *info)
{
 latch = 0;
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

int Mapper107_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x8000,0xffff,m107w);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}
