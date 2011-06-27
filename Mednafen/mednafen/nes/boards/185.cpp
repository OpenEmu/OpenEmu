#include "mapinc.h"

static uint8 latch;
static uint8 CHRBogus[8192];

static void Sync(void)
{
 //printf("%02x\n",mapbyte1[0]);
 //if((mapbyte1[0]&3)==3)
 if(CHRmask8[0] == 0)
 {
  if((latch&3) == 1)
   setchr8(0);
  else
   setchr8r(0x10,0);
 }
 else if(CHRmask8[0] == 1)
 {
  if((latch&2)) setchr8(latch&1);
  else setchr8r(0x10,0);
 }
}

static DECLFW(Mapper185_write)
{
 latch=V;
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

static void Power(CartInfo *info)
{
 setprg32(0x8000, 0);
 latch = 0x0;
 Sync();
}

int Mapper185_Init(CartInfo *info)
{
 memset(CHRBogus, 0xFF, 8192);
 info->Power = Power;
 info->StateAction = StateAction;

 SetupCartCHRMapping(0x10,CHRBogus,8192,0);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 SetWriteHandler(0x8000,0xFFFF,Mapper185_write);

 return(1);
}
