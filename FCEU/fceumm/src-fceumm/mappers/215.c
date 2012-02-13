/*
#include "mapinc.h"

static uint8 cmdin;
static uint8 cmd;
static uint8 regs[8];
static uint8 master,chrm;

static void DoPRG215(void)
{
 if(master&0x80)
 {
   setprg16(0x8000,master&0x1F);
   setprg16(0xc000,master&0x1F);
 }
 else
 {
   setprg8(0x8000,regs[4]); 6
   setprg8(0xA000,regs[6]); 7   0, 2, 5, 3, 6, 1, 7, 4
   setprg8(0xC000,~1);
   setprg8(0xE000,~0);
 }

}

static void DoCHR215(void)
{
 uint32 base=(cmd&0x80)<<5;
 int orie=(chrm&0x4)<<6;

 setchr2(0x0000^base,(orie|regs[0])>>1); 0
 setchr2(0x0800^base,(orie|regs[5])>>1); 1

 setchr1(0x1000,orie|regs[1]); 2
 setchr1(0x1400,orie|regs[3]); 3
 setchr1(0x1800,orie|regs[7]); 4
 setchr1(0x1c00,orie|regs[2]); 5
}

static DECLFW(Write215_write)
{
 switch(A&0xF001)
 {
  case 0xF001:IRQCount=V+2;break;
  case 0xF000:X6502_IRQEnd(FCEU_IQEXT);break;
 }
 if(A==0x5000)
 {
  master=V;
  DoPRG215();
//  DoCHR215();
 }
 else if(A==0x5001)
 {
  chrm=V;
  DoCHR215();
 }
 else
 switch(A&0xE001)
 {
  case 0xC000:setmirror(((V|(V>>7))&1)^1);break;
  case 0xa000:cmd=V;cmdin=1;DoCHR215();break;
  case 0x8001:if(!cmdin) break;
              regs[cmd&7]=V;
              DoPRG215();
              DoCHR215();
              cmdin=0;
              break;
 }
}

static void hooko215(void)
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

void Mapper215_init(void)
{
  int x;
  for(x=0;x<8;x++) regs[x]=0;
  master=chrm=cmd=cmdin=IRQCount=0;
  GameHBIRQHook=hooko215;
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x5000,0xFFFF,Write215_write);
  DoPRG215();
  DoCHR215();
}

*/
