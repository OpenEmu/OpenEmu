/****************************************************************************
 *  Genesis Plus
 *  Game Genie Hardware emulation
 *
 *  Copyright (C) 2009  Eke-Eke (GCN/Wii port)
 *
 *  Based on documentation from Charles McDonald
 *  (http://cgfm2.emuviews.com/txt/genie.txt)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#include "shared.h"

static struct
{
  uint8 enabled;
  uint8 *rom;
  uint16 regs[0x20];
  uint16 old[6];
  uint16 data[6];
  uint32 addr[6];
} ggenie;

static unsigned int ggenie_read_byte(unsigned int address);
static unsigned int ggenie_read_word(unsigned int address);
static void ggenie_write_byte(unsigned int address, unsigned int data);
static void ggenie_write_word(unsigned int address, unsigned int data);
static void ggenie_write_regs(unsigned int offset, unsigned int data);

void ggenie_init(void)
{
  memset(&ggenie,0,sizeof(ggenie));

  /* Open Game Genie ROM */
  FILE *f = fopen(GG_ROM,"rb");
  if (!f) return;

  /* store Game Genie ROM above cartridge ROM + SRAM */
  if (cart.romsize > 0x600000) return;
  ggenie.rom = cart.rom + 0x600000;

  /* Load ROM */
  int i = 0;
  while (i < 0x8000)
  {
    fread(ggenie.rom+i,0x1000,1,f);
    i += 0x1000;
  }
  fclose(f);

#ifdef LSB_FIRST
  /* Byteswap ROM */
  uint8 temp;
  for(i = 0; i < 0x8000; i += 2)
  {
    temp = ggenie.rom[i];
    ggenie.rom[i] = ggenie.rom[i+1];
    ggenie.rom[i+1] = temp;
  }
#endif

  /* $0000-$7fff mirrored into $8000-$ffff */
  memcpy(ggenie.rom+0x8000,ggenie.rom,0x8000);

  /* set flag */
  ggenie.enabled = 1;
}

void ggenie_shutdown(void)
{
  if (ggenie.enabled)
  {
    ggenie_switch(0);
    ggenie.enabled = 0;
  }
}

void ggenie_reset(int hard)
{
  if (ggenie.enabled)
  {
    if (hard)
    {
      /* clear codes */
      ggenie_switch(0);

      /* reset internal state */
      memset(ggenie.regs,0,sizeof(ggenie.regs));
      memset(ggenie.old,0,sizeof(ggenie.old));
      memset(ggenie.data,0,sizeof(ggenie.data));
      memset(ggenie.addr,0,sizeof(ggenie.addr));
    }

    /* Game Genie ROM is mapped at $000000-$007fff */
    m68k_memory_map[0].base = ggenie.rom;

    /* Internal registers are mapped at $000000-$00001f */
    m68k_memory_map[0].write8   = ggenie_write_byte;
    m68k_memory_map[0].write16  = ggenie_write_word;

    /* Disable registers reads */
    m68k_memory_map[0].read16 = NULL;
  }
}

void ggenie_switch(int enable)
{
  int i;
  if (enable)
  {
    /* enable cheats */
    for (i=0; i<6; i++)
    {
      /* patch is enabled ? */
      if (ggenie.regs[0] & (1 << i))
      {
        /* save old value and patch ROM if enabled */
        ggenie.old[i] = *(uint16 *)(cart.rom + ggenie.addr[i]);
        *(uint16 *)(cart.rom + ggenie.addr[i]) = ggenie.data[i];
      }
    }
  }
  else
  {
    /* disable cheats in reversed order in case the same address is used by multiple patches */
    for (i=5; i>=0; i--)
    {
      /* patch is enabled ? */
      if (ggenie.regs[0] & (1 << i))
      {
        /* restore original ROM value */
        *(uint16 *)(cart.rom + ggenie.addr[i]) = ggenie.old[i];
      }
    }
  }
}

static unsigned int ggenie_read_byte(unsigned int address)
{
  unsigned int data = ggenie.regs[(address >> 1) & 0x1f];
  return ((address & 1) ? (data & 0xff) : ((data >> 8) & 0xff));
}

static unsigned int ggenie_read_word(unsigned int address)
{
  return ggenie.regs[(address >> 1) & 0x1f];
}

static void ggenie_write_byte(unsigned int address, unsigned int data)
{
  /* Register offset */
  uint8 offset = (address >> 1) & 0x1f;

  /* /LWR and /UWR are used to decode writes */
  if (address & 1)
  {
    data = (ggenie.regs[offset] & 0xff00) | (data & 0xff);
  }
  else
  {
    data = (ggenie.regs[offset] & 0x00ff) | ((data & 0xff) << 8);
  }

  /* Update internal register */
  ggenie_write_regs(offset,data);
}

static void ggenie_write_word(unsigned int address, unsigned int data)
{
  /* Register offset */
  uint8 offset = (address >> 1) & 0x1f;

  /* Write internal register (full WORD) */
  ggenie_write_regs(offset,data);
}

static void ggenie_write_regs(unsigned int offset, unsigned int data)
{
  /* update internal register */
  ggenie.regs[offset] = data;

  /* Mode Register */
  if (offset == 0)
  {
    /* MODE bit */
    if (data & 0x400)
    {
      /* $0000-$7ffff reads mapped to Cartridge ROM */
      m68k_memory_map[0].base = cart.rom;
      m68k_memory_map[0].read8 = NULL; 
      m68k_memory_map[0].read16 = NULL; 
    }
    else
    {
      /* $0000-$7ffff reads mapped to Game Genie ROM */
      m68k_memory_map[0].base = ggenie.rom;
      m68k_memory_map[0].read8 = NULL; 
      m68k_memory_map[0].read16 = NULL; 

      /* READ_ENABLE bit */
      if (data & 0x200)
      {
        /* $0000-$7ffff reads mapped to Game Genie Registers */
        /* code doing this should execute in RAM so we don't need to modify base address */
        m68k_memory_map[0].read8 = ggenie_read_byte; 
        m68k_memory_map[0].read16 = ggenie_read_word; 
      }
    }

    /* LOCK bit */
    if (data & 0x100)
    {
      /* decode patch address (ROM area only)*/
      /* note: Charles's doc is wrong, first register holds bits 23-16 of patch address */
      ggenie.addr[0] = ((ggenie.regs[2]   & 0x3f) << 16) | ggenie.regs[3];
      ggenie.addr[1] = ((ggenie.regs[5]   & 0x3f) << 16) | ggenie.regs[6];
      ggenie.addr[2] = ((ggenie.regs[8]   & 0x3f) << 16) | ggenie.regs[9];
      ggenie.addr[3] = ((ggenie.regs[11]  & 0x3f) << 16) | ggenie.regs[12];
      ggenie.addr[4] = ((ggenie.regs[14]  & 0x3f) << 16) | ggenie.regs[15];
      ggenie.addr[5] = ((ggenie.regs[17]  & 0x3f) << 16) | ggenie.regs[18];

      /* decode patch data */
      ggenie.data[0] = ggenie.regs[4];
      ggenie.data[1] = ggenie.regs[7];
      ggenie.data[2] = ggenie.regs[10];
      ggenie.data[3] = ggenie.regs[13];
      ggenie.data[4] = ggenie.regs[16];
      ggenie.data[5] = ggenie.regs[19];

      /* disable internal registers */
      m68k_memory_map[0].write8   = m68k_unused_8_w;
      m68k_memory_map[0].write16  = m68k_unused_16_w;

      /* patch ROM when GG program exits (LOCK bit set) */
      /* this is done here to handle patched program reads faster & more easily */
      /* on real HW, address decoding would be done on each reads */
      ggenie_switch(1);
    }
    else
    {
      m68k_memory_map[0].write8   = ggenie_write_byte;
      m68k_memory_map[0].write16  = ggenie_write_word;
    }
  }

  /* RESET register */
  else if (offset == 1)
  {
    ggenie.regs[1] |= 1;
  }
}
