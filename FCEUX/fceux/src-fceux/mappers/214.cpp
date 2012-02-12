#include "mapinc.h"

static DECLFW(Mapper214_write)
{
// FCEU_printf("%02x:%02x\n",A,V);
 ROM_BANK16(0x8000,(A>>2)&3);
 ROM_BANK16(0xC000,(A>>2)&3);
 VROM_BANK8(A&3);
}

void Mapper214_init(void)
{
 ROM_BANK16(0x8000,0);
 ROM_BANK16(0xC000,0);
 VROM_BANK8(0);
 SetWriteHandler(0x8000,0xFFFF,Mapper214_write);
}
