#include "mapinc.h"

static uint8 latch;

static void Sync(void)
{
 setmirror((latch & 1) ? MI_H : MI_V);
 setchr8(latch & 0xF);
 setprg16(0x8000, (latch >> 4));
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

static void Power(CartInfo *info)
{
 latch = 0;
 setchr8(0);
 setprg16(0xc000, 0xF);
 Sync();
}


static DECLFW(Write)
{
 latch = V; 
 Sync();
}

int Mapper93_Init(CartInfo *info)
{
 SetWriteHandler(0x8000, 0xFFFF, Write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->StateAction = StateAction;
 info->Power = Power;
 return(1);
}
