#include "mapinc.h"

static uint8 CHRBanks[3], PRGBank8;
static void Sync(void)
{
 setchr4(0x0000, CHRBanks[0]);
 setchr2(0x1000, CHRBanks[1]);
 setchr2(0x1800, CHRBanks[2]);
 setprg8(0x8000, PRGBank8);
}

static DECLFW(m193w)
{
 switch(A&3)
 {
  case 0:CHRBanks[0] = V >> 2;break;
  case 1:CHRBanks[1] = (V >> 1) & 0x3F; break;
  case 2:CHRBanks[2] = (V >> 1) & 0x3F; break;
  case 3:PRGBank8 = V;break;
 }
 Sync();
}

static void Power(CartInfo *info)
{
 CHRBanks[0] = 0;
 CHRBanks[1] = 2;
 CHRBanks[2] = 3;
 PRGBank8 = 0xFC;
 Sync();
 setprg8(0xa000, 0xFD);
 setprg8(0xc000, 0xFE);
 setprg8(0xe000, 0xFF);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CHRBanks, 3),
  SFVAR(PRGBank8),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}


int Mapper193_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x4018,0x7fff,m193w);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 return(1);
}
