#include "mapinc.h"

static uint8 latch;

static void Sync(void)
{
 setprg32(0x8000, latch & 3);
 setchr8((latch >> 4) & 0xF);
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
 Sync();
}


static DECLFW(Write)
{
 latch = V & CartBR(A);
 Sync();
}

static DECLFW(Write50282)
{
 latch = (V & 0xFE & CartBR(A)) | (CartBR(A) & 1);
 Sync();
}

int Mapper11_Init(CartInfo *info)
{
 SetWriteHandler(0x8000, 0xFFFF, Write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->StateAction = StateAction;
 info->Power = Power;
 return(1);
}

int AGCI50282_Init(CartInfo *info)
{
 SetWriteHandler(0x8000, 0xFFFF, Write50282);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->StateAction = StateAction;
 info->Power = Power;
 return(1);
}
