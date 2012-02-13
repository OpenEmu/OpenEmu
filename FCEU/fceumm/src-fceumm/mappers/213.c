#include "mapinc.h"

static DECLFW(Mapper213_write)
{
 ROM_BANK32((A>>1)&3);
 VROM_BANK8((A>>3)&7);
}

void Mapper213_init(void)
{
 ROM_BANK32(0);
 VROM_BANK8(0);
 SetWriteHandler(0x8000,0xFFFF,Mapper213_write);
}
