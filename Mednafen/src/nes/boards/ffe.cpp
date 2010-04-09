#include "mapinc.h"

static uint8 IRQa, MirRegs[2], latch, CHRBanks[8], PRGBanks[4], MirSelect, ModeSelect;
static uint16 IRQCount;
static uint8 WRAM[8192];

static void FFEIRQHook(int a)
{
  if(IRQa)
  {
   if(0)
   {
    uint16 last = IRQCount;
    IRQCount -= a;
    if(IRQCount > last)
    {
     X6502_IRQBegin(MDFN_IQEXT);
     IRQa=0;
     IRQCount=0;
    }
   }
   else
   {
    uint16 last = IRQCount;
    IRQCount+=a;
    if(IRQCount < last)
    {
     X6502_IRQBegin(MDFN_IQEXT);
     IRQa=0;
     IRQCount=0;
    }
   }
  }
}

static void DoMirroring(void)
{
 if(!MirSelect)
  setmirror(((MirRegs[0] >> 4) & 1) ? MI_1 : MI_0);
 else
  setmirror(((MirRegs[1] >> 4) & 1) ? MI_H : MI_V);
}

static void DoPRG(void)
{
 if(!ModeSelect)
 {
  for(int x = 0; x < 4; x++)
   setprg8(0x8000 + x * 8192, PRGBanks[x]);
 }
 else
 {
  setprg16(0x8000, (latch >> 2) & 0x3F);
  setprg16(0xc000, 0x7);
 }
}

static void DoCHR(void)
{
 if(!ModeSelect)
 {
  for(int x = 0; x < 8; x++)
   setchr1(x * 1024, CHRBanks[x]);
 }
 else
 {
  setchr8(latch & 3);
 }
}

static DECLFW(Mapper6_write)
{
	if(A >= 0x4500 && A <= 0x4503)
	printf("%04x: %02x\n", A, V);
	if(A < 0x8000)
	{
	 switch(A)
	 {
	  default: printf("Oops: %04x : %02x\n", A, V);break;
	  case 0x42FE: MirSelect = 0; MirRegs[0] = V; DoMirroring(); break;
	  case 0x42FF: MirSelect = 1; MirRegs[1] = V; DoMirroring(); break;
	  case 0x4501: IRQa = V & 1; X6502_IRQEnd(MDFN_IQEXT); break;
	  case 0x4502: IRQCount &= 0xFF00; IRQCount |= V; X6502_IRQEnd(MDFN_IQEXT); break;
	  case 0x4503: IRQCount &= 0x00FF; IRQCount |= V << 8; IRQa = 1; X6502_IRQEnd(MDFN_IQEXT); break;
          case 0x4504: PRGBanks[0] = V; DoPRG(); break;
          case 0x4505: PRGBanks[1] = V; DoPRG(); break;
          case 0x4506: PRGBanks[2] = V; DoPRG(); break;
          case 0x4507: PRGBanks[3] = V; DoPRG(); break;
          case 0x4510: CHRBanks[0] = V; DoCHR(); break;
          case 0x4511: CHRBanks[1] = V; DoCHR(); break;
          case 0x4512: CHRBanks[2] = V; DoCHR(); break;
          case 0x4513: CHRBanks[3] = V; DoCHR(); break;
          case 0x4514: CHRBanks[4] = V; DoCHR(); break;
          case 0x4515: CHRBanks[5] = V; DoCHR(); break;
          case 0x4516: CHRBanks[6] = V; DoCHR(); break;
          case 0x4517: CHRBanks[7] = V; DoCHR(); break;
	 }
	}
	else
	{
	 latch = V;
	 DoPRG();
	 DoCHR();
	}
}

static void Reset(CartInfo *info)
{
 int x;

 if(!info->battery)
  memset(WRAM, 0x00, 8192);

 latch = 0;
 MirRegs[0] = 0x00;
 MirRegs[1] = 0x00 | (((info->mirror & 1) << 4) ^ 0x10);
 MirSelect = 1;
 DoMirroring();

 for(x = 0; x < 8; x++)
  CHRBanks[x] = x;
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 PRGBanks[2] = 0xFE;
 PRGBanks[3] = 0xFF;
 DoPRG();
 DoCHR();

 setprg8r(0x10, 0x6000, 0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, 8192),
  SFARRAY(CHRBanks, 8),
  SFARRAY(PRGBanks, 4),
  SFARRAY(MirRegs, 2),
  SFVAR(MirSelect),
  SFVAR(latch),
  SFVAR(IRQCount),
  SFVAR(IRQa),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  DoCHR();
  DoPRG();
  DoMirroring();
 }

 return(ret);
}

int Mapper6_Init(CartInfo *info)
{
 ModeSelect = 1;
 MapIRQHook = FFEIRQHook;
 info->Power = info->Reset = Reset;

 SetWriteHandler(0x4020,0x5fff,Mapper6_write);
 SetWriteHandler(0x8000,0xffff,Mapper6_write);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 SetupCartPRGMapping(0x10, WRAM, 8192, 1);

 if(info->battery)
 {
  memset(WRAM, 0x00, 8192);
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }

 info->StateAction = StateAction;
 return(1);
}

int Mapper17_Init(CartInfo *info)
{
 Mapper6_Init(info);
 ModeSelect = 0;
 return(1);
}
