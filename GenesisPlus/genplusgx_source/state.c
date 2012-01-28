/***************************************************************************************
 *  Genesis Plus
 *  Savestate support
 *
 *  Copyright (C) 2007-2011  Eke-Eke (GCN/Wii port)
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
 *
 ****************************************************************************************/

#include "shared.h"

int state_load(unsigned char *buffer)
{
  /* buffer size */
  int bufferptr = 0;

  /* first allocate state buffer */
  unsigned char *state = (unsigned char *)malloc(STATE_SIZE);
  if (!state) return 0;

  /* uncompress savestate */
  unsigned long inbytes, outbytes;
  memcpy(&inbytes, buffer, 4);
  outbytes = STATE_SIZE;
  uncompress ((Bytef *)state, &outbytes, (Bytef *)(buffer + 4), inbytes);

  /* signature check (GENPLUS-GX x.x.x) */
  char version[17];
  load_param(version,16);
  version[16] = 0;
  if (strncmp(version,STATE_VERSION,11))
  {
    free(state);
    return -1;
  }

  /* version check (1.4.0 and above) */
  if ((version[11] < 0x31) || ((version[11] == 0x31) && (version[13] < 0x34)))
  {
    free(state);
    return -1;
  }

  /* reset system */
  system_reset();

  // GENESIS
  if (system_hw == SYSTEM_PBC)
  {
    load_param(work_ram, 0x2000);
  }
  else
  {
    load_param(work_ram, sizeof(work_ram));
    load_param(zram, sizeof(zram));
    load_param(&zstate, sizeof(zstate));
    load_param(&zbank, sizeof(zbank));
    if (zstate == 3)
    {
      m68k_memory_map[0xa0].read8   = z80_read_byte;
      m68k_memory_map[0xa0].read16  = z80_read_word;
      m68k_memory_map[0xa0].write8  = z80_write_byte;
      m68k_memory_map[0xa0].write16 = z80_write_word;
    }
    else
    {
      m68k_memory_map[0xa0].read8   = m68k_read_bus_8;
      m68k_memory_map[0xa0].read16  = m68k_read_bus_16;
      m68k_memory_map[0xa0].write8  = m68k_unused_8_w;
      m68k_memory_map[0xa0].write16 = m68k_unused_16_w;
    }
  }

  /* 1.4.1 and above */
  if ((version[11] > 0x31) || (version[13] > 0x34) || (version[15] > 0x30))
  {
    /* extended state */
    load_param(&mcycles_68k, sizeof(mcycles_68k));
    load_param(&mcycles_z80, sizeof(mcycles_z80));
  }

  // IO
  if (system_hw == SYSTEM_PBC)
  {
    load_param(&io_reg[0], 1);
  }
  else
  {
    load_param(io_reg, sizeof(io_reg));
    io_reg[0] = region_code | 0x20 | (config.tmss & 1);
  }

  // VDP
  bufferptr += vdp_context_load(&state[bufferptr], version);

  // SOUND 
  bufferptr += sound_context_load(&state[bufferptr], version);

  // 68000 
  if (system_hw != SYSTEM_PBC)
  {
    uint16 tmp16;
    uint32 tmp32;
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D0, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D1, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D2, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D3, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D4, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D5, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D6, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D7, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A0, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A1, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A2, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A3, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A4, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A5, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A6, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A7, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_PC, tmp32);  
    load_param(&tmp16, 2); m68k_set_reg(M68K_REG_SR, tmp16);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_USP,tmp32);
  }

  // Z80 
  load_param(&Z80, sizeof(Z80_Regs));
  Z80.irq_callback = z80_irq_callback;

  // Cartridge HW
  if (system_hw == SYSTEM_PBC)
  {
    bufferptr += sms_cart_context_load(&state[bufferptr], version);
  }
  else
  {
    bufferptr += md_cart_context_load(&state[bufferptr], version);
  }

  free(state);
  return 1;
}

int state_save(unsigned char *buffer)
{
  /* buffer size */
  int bufferptr = 0;

  /* first allocate state buffer */
  unsigned char *state = (unsigned char *)malloc(STATE_SIZE);
  if (!state) return 0;

  /* version string */
  char version[16];
  strncpy(version,STATE_VERSION,16);
  save_param(version, 16);

  // GENESIS
  if (system_hw == SYSTEM_PBC)
  {
    save_param(work_ram, 0x2000);
  }
  else
  {
    save_param(work_ram, sizeof(work_ram));
    save_param(zram, sizeof(zram));
    save_param(&zstate, sizeof(zstate));
    save_param(&zbank, sizeof(zbank));
  }
  save_param(&mcycles_68k, sizeof(mcycles_68k));
  save_param(&mcycles_z80, sizeof(mcycles_z80));

  // IO
  if (system_hw == SYSTEM_PBC)
  {
    save_param(&io_reg[0], 1);
  }
  else
  {
    save_param(io_reg, sizeof(io_reg));
  }

  // VDP
  bufferptr += vdp_context_save(&state[bufferptr]);

  // SOUND
  bufferptr += sound_context_save(&state[bufferptr]);

  // 68000 
  if (system_hw != SYSTEM_PBC)
  {
    uint16 tmp16;
    uint32 tmp32;
    tmp32 = m68k_get_reg(NULL, M68K_REG_D0);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_D1);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_D2);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_D3);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_D4);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_D5);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_D6);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_D7);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A0);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A1);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A2);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A3);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A4);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A5);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A6);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_A7);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(NULL, M68K_REG_PC);  save_param(&tmp32, 4);
    tmp16 = m68k_get_reg(NULL, M68K_REG_SR);  save_param(&tmp16, 2); 
    tmp32 = m68k_get_reg(NULL, M68K_REG_USP); save_param(&tmp32, 4);
  }

  // Z80 
  save_param(&Z80, sizeof(Z80_Regs));

  // Cartridge HW
  if (system_hw == SYSTEM_PBC)
  {
    bufferptr += sms_cart_context_save(&state[bufferptr]);
  }
  else
  {
    bufferptr += md_cart_context_save(&state[bufferptr]);
  }

  /* compress state file */
  unsigned long inbytes   = bufferptr;
  unsigned long outbytes  = STATE_SIZE;
  compress2 ((Bytef *)(buffer + 4), &outbytes, (Bytef *)state, inbytes, 9);
  memcpy(buffer, &outbytes, 4);
  free(state);

  /* return total size */
  return (outbytes + 4);
}
