#include "mapinc.h"

static uint8 latch;

static void Sync(void)
{
 setprg16(0x8000, (latch >> 4) & 0x07);
 setchr8(latch & 0x0F);
 setmirror((latch >> 7) ? MI_1 : MI_0);
}

static DECLFW(Mapper152_write)
{
 latch = V;
 Sync();
}

static void Power(CartInfo *info)
{
 latch = 0;
 setprg16(0xc000, 0x07);
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

int Mapper152_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x6000,0xffff,Mapper152_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}

