#include "mapinc.h"

static uint8 PRGBank32, CHRBanks[2], WRAM[8192];

static void Sync(void)
{
 setprg32(0x8000, PRGBank32);
 setchr4(0x0000, CHRBanks[0]);
 setchr4(0x1000, CHRBanks[1]);
}

static DECLFW(Mapper34_write)
{
 switch(A)
 {
  case 0x7FFD:PRGBank32 = V;break;
  case 0x7FFE:CHRBanks[0] = V;break;
  case 0x7fff:CHRBanks[1] = V;break;
 }
 if(A>=0x8000)
  PRGBank32 = V;

 Sync();
}

static void Power(CartInfo *info)
{
 if(!info->battery)
  memset(WRAM, 0xFF, 8192);
 PRGBank32 = 0;
 CHRBanks[0] = 0;
 CHRBanks[1] = 1;
 setprg8r(0x10, 0x6000, 0);
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(PRGBank32),
  SFARRAY(CHRBanks, 2),
  SFARRAY(WRAM, 8192),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper34_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetupCartPRGMapping(0x10, WRAM, 8192, 1);
 if(info->battery)
 {
  memset(WRAM, 0xFF, 8192);
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 SetWriteHandler(0x7ffd,0xffff,Mapper34_write);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 return(1);
}
