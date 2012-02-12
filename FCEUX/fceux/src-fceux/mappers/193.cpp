#include "mapinc.h"

static DECLFW(m193w)
{
  //printf("$%04x:$%02x\n",A,V);
 switch(A&3)
 {
  case 0:VROM_BANK4(0x0000,V>>2);break;
  case 1:VROM_BANK2(0x1000,V>>1);break;
  case 2:VROM_BANK2(0x1800,V>>1);break;
  case 3:ROM_BANK8(0x8000,V);break;
 }
}

void Mapper193_init(void)
{
 ROM_BANK32(~0);
 SetWriteHandler(0x4018,0x7fff,m193w);
 SetReadHandler(0x4018,0x7fff,0);
}
