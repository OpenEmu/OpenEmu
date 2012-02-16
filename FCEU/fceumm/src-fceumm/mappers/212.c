#include "mapinc.h"

static DECLFW(Mapper212_write)
{
  if((A&0x4000)==0x4000)
   {
     ROM_BANK32((A&6)>>1);
   }
  else
   {
     ROM_BANK16(0x8000,A&7);
     ROM_BANK16(0xc000,A&7);
   }
 VROM_BANK8(A&7);
 MIRROR_SET((A>>3)&1);
}

void Mapper212_init(void)
{
 ROM_BANK32(~0);
 VROM_BANK8(~0);
 SetWriteHandler(0x8000,0xFFFF,Mapper212_write);
}
