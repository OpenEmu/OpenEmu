#include "mapinc.h"

static uint8 CHRBanks[8], PRGBank16;
static uint8 WRAM[8192];

static void Sync(void)
{
 setprg16(0x8000, PRGBank16);
 for(int x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
}

static DECLFW(M156Write)
{
 if(A>=0xc000 && A<=0xC003)
  CHRBanks[A & 3] = V;
 else if(A>=0xc008 &&  A<=0xc00b)
  CHRBanks[4 + (A & 3)] = V;
 if(A==0xc010) 
  PRGBank16 = V;
 Sync();
}

static void Power(CartInfo *info)
{
 for(int x = 0; x < 8; x++)
  CHRBanks[x] = x;
 PRGBank16 = 0;
 Sync();
 setprg16(0xc000, 0xFF);
 setprg8r(0x10, 0x6000, 0);
 if(!info->battery)
  memset(WRAM, 0xFF, 8192);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CHRBanks, 8),
  SFVAR(PRGBank16),
  SFARRAY(WRAM, 8192),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper156_Init(CartInfo *info)
{
 setmirror(MI_0);
 SetWriteHandler(0xc000,0xc010,M156Write);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;
 SetupCartPRGMapping(0x10, WRAM, 8192, 1);
 if(info->battery)
 {
  memset(WRAM, 0xFF, 8192);
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 return(1);
}

