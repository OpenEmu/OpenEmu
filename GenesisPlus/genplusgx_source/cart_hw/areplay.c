/****************************************************************************
 *  Genesis Plus
 *  Action Replay / Pro Action Replay emulation
 *
 *  Copyright (C) 2009  Eke-Eke (GCN/Wii port)
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

#define TYPE_PRO1 0x12
#define TYPE_PRO2 0x22

static struct
{
  uint8 enabled;
  uint8 status;
  uint8 *rom;
  uint8 *ram;
  uint16 regs[13];
  uint16 old[4];
  uint16 data[4];
  uint32 addr[4];
} action_replay;

static void ar_write_regs(uint32 address, uint32 data);
static void ar_write_regs_2(uint32 address, uint32 data);
static void ar_write_ram_8(uint32 address, uint32 data);

void areplay_init(void)
{
  memset(&action_replay,0,sizeof(action_replay));
  if (cart.romsize > 0x800000) return;

  /* Open Action Replay ROM */
  FILE *f = fopen(AR_ROM,"rb");
  if (!f) return;

  /* store Action replay ROM + RAM above cartridge ROM + SRAM */
  action_replay.rom = cart.rom + 0x800000;
  action_replay.ram = cart.rom + 0x810000;

  /* ROM size */
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  fseek(f, 0, SEEK_SET);

  /* detect Action Replay board type */
  switch (size)
  {
    case 0x8000:  
    {
      /* normal Action Replay (32K) */
      action_replay.enabled = TYPE_AR;
  
      /* internal registers mapped at $010000-$01ffff */
      m68k_memory_map[0x01].write16 = ar_write_regs;

      break;
    }

    case 0x10000:
    case 0x20000:
    {
      /* read Stack Pointer */
      uint8 sp[4];
      fread(&sp, 4, 1, f);
      fseek(f, 0, SEEK_SET);

      /* Detect board version */
      if (sp[1] == 0x42)
      {
        /* PRO Action Replay 1 (64/128K) */
        action_replay.enabled = TYPE_PRO1;

        /* internal registers mapped at $010000-$01ffff */
        m68k_memory_map[0x01].write16 = ar_write_regs;
      }
      else if (sp[1] == 0x60)
      {
        /* PRO Action Replay 2 (64K) */
        action_replay.enabled = TYPE_PRO2;

        /* internal registers mapped at $100000-$10ffff */
        m68k_memory_map[0x10].write16 = ar_write_regs_2;
      }

      /* internal RAM (64k), mapped at $420000-$42ffff or $600000-$60ffff */
      if (action_replay.enabled)
      {
        m68k_memory_map[sp[1]].base      = action_replay.ram;
        m68k_memory_map[sp[1]].read8     = NULL;
        m68k_memory_map[sp[1]].read16    = NULL;
        m68k_memory_map[sp[1]].write8    = ar_write_ram_8;
        m68k_memory_map[sp[1]].write16   = NULL;
      }

      break;
    }

    default:
    {
      break;
    }
  }

  if (action_replay.enabled)
  {
    /* Load ROM */
    int i = 0;
    while (i < size)
    {
      fread(action_replay.rom+i,0x1000,1,f);
      i += 0x1000;
    }

#ifdef LSB_FIRST
    /* Byteswap ROM */
    uint8 temp;
    for(i = 0; i < size; i += 2)
    {
      temp = action_replay.rom[i];
      action_replay.rom[i] = action_replay.rom[i+1];
      action_replay.rom[i+1] = temp;
    }
#endif
  }

  fclose(f);
}

void areplay_shutdown(void)
{
  /* clear existing patches */
  areplay_set_status(AR_SWITCH_OFF);

  /* disable device by default */
  action_replay.enabled = 0;
}

void areplay_reset(int hard)
{
  if (action_replay.enabled)
  {
    if (hard || (action_replay.status == AR_SWITCH_TRAINER))
    {
      /* reset internal registers */
      memset(action_replay.regs, 0, sizeof(action_replay.regs));
      memset(action_replay.old, 0, sizeof(action_replay.old));
      memset(action_replay.data, 0, sizeof(action_replay.data));
      memset(action_replay.addr, 0, sizeof(action_replay.addr));

      /* by default, internal ROM is mapped at $000000-$00FFFF */
      m68k_memory_map[0].base = action_replay.rom;

      /* reset internal RAM on power-on */
      if (hard)
      {
        memset(action_replay.ram,0xff,0x10000);
      }
    }
  }
}

int areplay_get_status(void)
{
  if (action_replay.enabled)
  {
    return action_replay.status;
  }

  return -1;
}

void areplay_set_status(int status)
{
  if (action_replay.enabled)
  {
    /* no Trainer mode for normal Action Replay */
    if ((action_replay.enabled == TYPE_AR) && (status == AR_SWITCH_TRAINER))
    {
      status = AR_SWITCH_OFF;
    }

    /* check status changes */
    switch (status)
    {
      case AR_SWITCH_OFF:
      case AR_SWITCH_TRAINER:
      {
        /* check that patches were previously enabled */
        if (action_replay.status == AR_SWITCH_ON)
        {
          /* restore original data */
          *(uint16 *)(cart.rom + action_replay.addr[0]) = action_replay.old[0];
          *(uint16 *)(cart.rom + action_replay.addr[1]) = action_replay.old[1];
          *(uint16 *)(cart.rom + action_replay.addr[2]) = action_replay.old[2];
          *(uint16 *)(cart.rom + action_replay.addr[3]) = action_replay.old[3];
        }
        break;
      }

      case AR_SWITCH_ON:
      {
        /* check that patches were previously disabled */
        if (action_replay.status != AR_SWITCH_ON)
        {
          /* decode patch data */
          action_replay.data[0] = action_replay.regs[0];
          action_replay.data[1] = action_replay.regs[4];
          action_replay.data[2] = action_replay.regs[7];
          action_replay.data[3] = action_replay.regs[10];

          /* decode patch address ($000000-$7fffff) */
          action_replay.addr[0] = (action_replay.regs[1]  | ((action_replay.regs[2]   & 0x3f00) << 8)) << 1;
          action_replay.addr[1] = (action_replay.regs[5]  | ((action_replay.regs[6]   & 0x3f00) << 8)) << 1;
          action_replay.addr[2] = (action_replay.regs[8]  | ((action_replay.regs[9]   & 0x3f00) << 8)) << 1;
          action_replay.addr[3] = (action_replay.regs[11] | ((action_replay.regs[12]  & 0x3f00) << 8)) << 1;

          /* save original data */
          action_replay.old[0] = *(uint16 *)(cart.rom + action_replay.addr[0]);
          action_replay.old[1] = *(uint16 *)(cart.rom + action_replay.addr[1]);
          action_replay.old[2] = *(uint16 *)(cart.rom + action_replay.addr[2]);
          action_replay.old[3] = *(uint16 *)(cart.rom + action_replay.addr[3]);

          /* patch new data */
          *(uint16 *)(cart.rom + action_replay.addr[0]) = action_replay.data[0];
          *(uint16 *)(cart.rom + action_replay.addr[1]) = action_replay.data[1];
          *(uint16 *)(cart.rom + action_replay.addr[2]) = action_replay.data[2];
          *(uint16 *)(cart.rom + action_replay.addr[3]) = action_replay.data[3];
        }
        break;
      }

      default:
      {
        return;
      }
    }

    /* update status */
    action_replay.status = status;
  }
}

static void ar_write_regs(uint32 address, uint32 data)
{
  /* register offset */
  int offset = (address & 0xffff) >> 1;
  if (offset > 12)
  {
    m68k_unused_16_w(address,data);
    return;
  }

  /* update internal register */
  action_replay.regs[offset] = data;

  /* MODE register */
  if (action_replay.regs[3] == 0xffff)
  {
    /* check switch status */
    if (action_replay.status == AR_SWITCH_ON)
    {
      /* reset existing patches */
      areplay_set_status(AR_SWITCH_OFF);
      areplay_set_status(AR_SWITCH_ON);
    }

    /* enable Cartridge ROM */
    m68k_memory_map[0].base = cart.rom;
  }
}

static void ar_write_regs_2(uint32 address, uint32 data)
{
  /* enable Cartridge ROM */
  if (((address & 0xff) == 0x78) && (data == 0xffff))
  {
    m68k_memory_map[0].base = cart.rom;
  }
}

static void ar_write_ram_8(uint32 address, uint32 data)
{
  /* byte writes are handled as word writes, with LSB duplicated in MSB (/LWR is not used) */
  *(uint16 *)(action_replay.ram + (address & 0xfffe)) = (data | (data << 8));
}

