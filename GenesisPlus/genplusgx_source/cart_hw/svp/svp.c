/*
   basic, incomplete SSP160x (SSP1601?) interpreter
   with SVP memory controller emu

   (c) Copyright 2008, Grazvydas "notaz" Ignotas
   Free for non-commercial use.

   For commercial use, separate licencing terms must be obtained.

   Modified for Genesis Plus GX (Eke-Eke): added BIG ENDIAN support, fixed addr/code inversion
*/

#include "shared.h"

svp_t *svp = NULL;
int16 SVP_cycles = 800; 

void svp_init(void)
{
  svp = (void *) ((char *)cart.rom + 0x200000);
  memset(svp, 0, sizeof(*svp));
}

void svp_reset(void)
{
  memcpy(svp->iram_rom + 0x800, cart.rom + 0x800, 0x20000 - 0x800);
  ssp1601_reset(&svp->ssp1601);
}

void svp_write_dram(uint32 address, uint32 data)
{
  *(uint16 *)(svp->dram + (address & 0x1fffe)) = data;
  if ((address == 0x30fe06) && data) svp->ssp1601.emu_status &= ~SSP_WAIT_30FE06;
  if ((address == 0x30fe08) && data) svp->ssp1601.emu_status &= ~SSP_WAIT_30FE08;
}

uint32 svp_read_cell_1(uint32 address)
{
  address >>= 1;
  address = (address & 0x7001) | ((address & 0x3e) << 6) | ((address & 0xfc0) >> 5);
  return *(uint16 *)(svp->dram + (address & 0x1fffe));
}

uint32 svp_read_cell_2(uint32 address)
{
  address >>= 1;
  address = (address & 0x7801) | ((address & 0x1e) << 6) | ((address & 0x7e0) >> 4);
  return *(uint16 *)(svp->dram + (address & 0x1fffe));
}

