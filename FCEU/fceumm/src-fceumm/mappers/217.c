/*
#include "mapinc.h"

static uint8 cmdin;
static uint8 cmd;
static uint8 regs[8];
static uint8 master,chrm;

static void DoPRG217(void)
{
 if(master&0x80)
 {
   setprg16(0x8000,master&0x1F);
   setprg16(0xc000,master&0x1F);
 }
 else
 {
   setprg8(0x8000,regs[1]);
   setprg8(0xA000,regs[3]);
   setprg8(0xC000,~1);
   setprg8(0xE000,~0);
 }
}

static void DoCHR217(void)
{
 uint32 base=(cmd&0x80)<<5;
 int orie=(chrm&0x4)<<6;

 setchr2(0x0000^base,(orie|regs[0])>>1);
 setchr2(0x0800^base,(orie|regs[7])>>1);

 setchr1(0x1000,orie|regs[5]);
 setchr1(0x1400,orie|regs[2]);
 setchr1(0x1800,orie|regs[6]);
 setchr1(0x1c00,orie|regs[4]);
}

static DECLFW(Write217_write)
{
// if(A==0x5000)
// {
//  master=V;
//  DoPRG217();
// }
// else if(A==0x5001)
// {
//  chrm=V;
//  DoCHR217();
// }
// else if(A==0x5007)
// {
// }

 switch(A&0xE001)
 {
  case 0x4000:master=V;DoPRG217();break;
  case 0x8000:IRQCount=V;break;
  case 0xc001:break;
  case 0xe000:X6502_IRQEnd(FCEU_IQEXT);break;
  case 0xe001:break;
  case 0xa001:setmirror((V&1)^1);break;
  case 0x8001:cmd=V;cmdin=1;DoPRG217();DoCHR217();break;
  case 0xa000:if(!cmdin) break;
              regs[cmd&7]=V;
              DoPRG217();
              DoCHR217();
              cmdin=0;
              break;
 }
}

static void hooko217(void)
{
 if(IRQCount)
 {
  IRQCount--;
  if(!IRQCount)
  {
     X6502_IRQBegin(FCEU_IQEXT);
  }
 }
}

void Mapper217_init(void)
{
  int x;
  for(x=0;x<8;x++) regs[x]=0;
  master=chrm=cmd=cmdin=IRQCount=0;
  GameHBIRQHook=hooko217;
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x5000,0xFFFF,Write217_write);
  DoPRG217();
  DoCHR217();
}
*/