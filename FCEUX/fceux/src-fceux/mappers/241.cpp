#include "mapinc.h"

static DECLFW(M241wr)
{
//  FCEU_printf("Wr: $%04x:$%02x, $%04x\n",A,V,X.PC);
 if(A<0x8000)
 {
// printf("$%04x:$%02x, $%04x\n",A,V,X.PC);
 }
 else
  ROM_BANK32(V);
}

static DECLFR(M241rd)
{
 //DumpMem("out",0x8000,0xffff);
 //printf("Rd: $%04x, $%04x\n",A,X.PC);
 return(0x50);
}

void Mapper241_init(void)
{
 ROM_BANK32(0);
 SetWriteHandler(0x5000,0x5fff,M241wr);
 SetWriteHandler(0x8000,0xFFFF,M241wr);
 SetReadHandler(0x4020,0x5fff,M241rd);
}
