#include "mapinc.h"

static uint8 PRGBanks[2], CHRBanks[8], IRQCount, IRQLatch, IRQa;
int32 acount;

static void Sync(void)
{
 int x;

 setprg8(0x8000, PRGBanks[0]);
 setprg8(0xa000, PRGBanks[1]);
 for(x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
}


static DECLFW(Write)
{
 A &= 0xF003;
 if((A & 0xF000) == 0x8000)
  PRGBanks[0] = V;
 else if((A & 0xF000) == 0xA000)
  PRGBanks[1] = V;
 else switch(A)
 {
  case 0xb000: CHRBanks[0] = V; break;
  case 0xb002: CHRBanks[1] = V; break;
  case 0xc000: CHRBanks[2] = V; break;
  case 0xc002: CHRBanks[3] = V; break;
  case 0xd000: CHRBanks[4] = V; break;
  case 0xd002: CHRBanks[5] = V; break;
  case 0xe000: CHRBanks[6] = V; break;
  case 0xe002: CHRBanks[7] = V; break;
  case 0xf000: IRQLatch = V; break;
  case 0xf001: X6502_IRQEnd(MDFN_IQEXT); IRQa = 0; IRQCount = IRQLatch; break;
  case 0xf002: IRQa = 1; break;
  default: printf("%04x: %02x\n", A, V);break;
 }

 Sync();
}

static void Power(CartInfo *info)
{
 IRQCount = IRQa = 0;
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 setprg16(0xc000, 0x7F);
 for(unsigned int x = 0; x < 8; x++)
  CHRBanks[x] = x;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(PRGBanks, 2),
  SFARRAY(CHRBanks, 8),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

static void IRQHook(int a)
{
 if(IRQa)
 {
  acount += a * 3;
  while(acount >= 341)
  {
   acount -= 341;
   IRQCount++;
   if(IRQCount == 0)
   {
    X6502_IRQBegin(MDFN_IQEXT);
    IRQa=0;
   }
  }
 }
}


int Mapper222_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 SetWriteHandler(0x8000, 0xFFFF, Write);
 MapIRQHook = IRQHook;
 return(1);
}
