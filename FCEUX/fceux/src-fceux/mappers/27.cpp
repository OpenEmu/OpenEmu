#include "mapinc.h"

static uint32 regchr[9];

static DECLFW(Mapper27_write)
{
 int regnum;
 A&=0xF00F;
 if((A>=0xB000) && (A<=0xE003)) {
    regnum=((((A>>12)+1)&0x03)<<1)|((A&0x02)>>1);
    if(A&1)
       regchr[regnum]=(regchr[regnum]&0x00F)|(V<<4);
    else
       regchr[regnum]=(regchr[regnum]&0x1F0)|(V&0xF);
    VROM_BANK1(regnum<<10,regchr[regnum]);
 }
 switch(A)
 {
  case 0x8000: ROM_BANK8(0x8000|((regchr[8]&2)<<13),V); break;
  case 0xA000: ROM_BANK8(0xa000,V); break;
  case 0x9000: switch(V&3){
                  case 0:setmirror(MI_V);break;
                  case 1:setmirror(MI_H);break;
                  case 2:setmirror(MI_0);break;
                  case 3:setmirror(MI_1);break;
               }
  case 0x9002: regchr[8]=V; break;
  case 0xF000: //X6502_IRQEnd(FCEU_IQEXT);
               IRQLatch=(IRQLatch&0xF0)|(V&0x0F);
                           break;
  case 0xF001: //X6502_IRQEnd(FCEU_IQEXT);
               IRQLatch=(IRQLatch&0x0F)|((V&0xF)<<4);
                           break;
  case 0xF003: IRQa=((IRQa&0x1)<<1)|(IRQa&0x1);
                           X6502_IRQEnd(FCEU_IQEXT);
               break;
  case 0xF002: IRQa=V&3;
                           if(IRQa&0x02) IRQCount=IRQLatch-1;
//                           X6502_IRQEnd(FCEU_IQEXT);
                           break;
 }
// if((A&0xF000)==0xF000) FCEU_printf("$%04x:$%02x, %d\n",A,V, scanline);
}

static void Mapper27_hb(void)
{
//   FCEU_printf("%02x-%d,%d,%d\n",scanline,IRQa,IRQCount,IRQLatch);
        if(IRQa&0x2){
           if(IRQCount==0xFF){
             X6502_IRQBegin(FCEU_IQEXT);
              IRQCount=IRQLatch+1;
         } else {
           IRQCount++;
           }
        }
}

void Mapper27_init(void)
{
  int i;
  for (i=0; i<9; i++) {
    regchr[i]=0;
  }
  IRQa=0;
  IRQCount=IRQLatch=0;
  SetWriteHandler(0x8000,0xffff,Mapper27_write);
  GameHBIRQHook=Mapper27_hb;
}

