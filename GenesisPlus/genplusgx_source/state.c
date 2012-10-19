/***************************************************************************************
 *  Genesis Plus
 *  Savestate support
 *
 *  Copyright (C) 2007-2012  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"

int state_load(unsigned char *state)
{
  int i, bufferptr = 0;

  /* signature check (GENPLUS-GX x.x.x) */
  char version[17];
  load_param(version,16);
  version[16] = 0;
  if (memcmp(version,STATE_VERSION,11))
  {
    return 0;
  }

  /* version check (1.7.1 and above only) */
  if ((version[11] < 0x31) || (version[13] < 0x37) || (version[15] < 0x31))
  {
    return 0;
  }

  /* reset system */
  system_reset();

  /* enable VDP access for TMSS systems */
  for (i=0xc0; i<0xe0; i+=8)
  {
    m68k.memory_map[i].read8    = vdp_read_byte;
    m68k.memory_map[i].read16   = vdp_read_word;
    m68k.memory_map[i].write8   = vdp_write_byte;
    m68k.memory_map[i].write16  = vdp_write_word;
    zbank_memory_map[i].read    = zbank_read_vdp;
    zbank_memory_map[i].write   = zbank_write_vdp;
  }

  /* GENESIS */
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    load_param(work_ram, sizeof(work_ram));
    load_param(zram, sizeof(zram));
    load_param(&zstate, sizeof(zstate));
    load_param(&zbank, sizeof(zbank));
    if (zstate == 3)
    {
      m68k.memory_map[0xa0].read8   = z80_read_byte;
      m68k.memory_map[0xa0].read16  = z80_read_word;
      m68k.memory_map[0xa0].write8  = z80_write_byte;
      m68k.memory_map[0xa0].write16 = z80_write_word;
    }
    else
    {
      m68k.memory_map[0xa0].read8   = m68k_read_bus_8;
      m68k.memory_map[0xa0].read16  = m68k_read_bus_16;
      m68k.memory_map[0xa0].write8  = m68k_unused_8_w;
      m68k.memory_map[0xa0].write16 = m68k_unused_16_w;
    }
  }
  else
  {
    load_param(work_ram, 0x2000);
  }

  /* IO */
  load_param(io_reg, sizeof(io_reg));
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    io_reg[0] = region_code | 0x20 | (config.bios & 1);
  }
  else
  {
    io_reg[0] = 0x80 | (region_code >> 1);
  }

  /* VDP */
  bufferptr += vdp_context_load(&state[bufferptr]);

  /* SOUND */
  bufferptr += sound_context_load(&state[bufferptr]);
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    SN76489_Init(snd.blips[0][0], snd.blips[0][1], SN_INTEGRATED);
    SN76489_Config(0, config.psg_preamp, config.psgBoostNoise, 0xff);
  }
  else
  {
    SN76489_Init(snd.blips[0][0], snd.blips[0][1], (system_hw < SYSTEM_MARKIII) ? SN_DISCRETE : SN_INTEGRATED);
    SN76489_Config(0, config.psg_preamp, config.psgBoostNoise, io_reg[6]);
  }

  /* 68000 */
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
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
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_ISP,tmp32);

  	load_param(&m68k.cycles, sizeof(m68k.cycles));
    load_param(&m68k.int_level, sizeof(m68k.int_level));
    load_param(&m68k.stopped, sizeof(m68k.stopped));
  }

  /* Z80 */ 
  load_param(&Z80, sizeof(Z80_Regs));
  Z80.irq_callback = z80_irq_callback;

  /* Extra HW */
  if (system_hw == SYSTEM_MCD)
  {
    /* handle case of MD cartridge using or not CD hardware */
    char id[5];
    load_param(id,4);
    id[4] = 0;

    /* check if CD hardware was enabled before attempting to restore */
    if (memcmp(id,"SCD!",4))
    {
       return 0;
    }

    /* CD hardware */
    bufferptr += scd_context_load(&state[bufferptr]);
  }
  else if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {  
    /* MD cartridge hardware */
    bufferptr += md_cart_context_load(&state[bufferptr]);
  }
  else
  {
    /* MS cartridge hardware */
    bufferptr += sms_cart_context_load(&state[bufferptr]);
    sms_cart_switch(~io_reg[0x0E]);
  }

  return bufferptr;
}

int state_save(unsigned char *state)
{
  /* buffer size */
  int bufferptr = 0;

  /* version string */
  char version[16];
  strncpy(version,STATE_VERSION,16);
  save_param(version, 16);

  /* GENESIS */
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    save_param(work_ram, sizeof(work_ram));
    save_param(zram, sizeof(zram));
    save_param(&zstate, sizeof(zstate));
    save_param(&zbank, sizeof(zbank));
  }
  else
  {
    save_param(work_ram, 0x2000);
  }

  /* IO */
  save_param(io_reg, sizeof(io_reg));

  /* VDP */
  bufferptr += vdp_context_save(&state[bufferptr]);

  /* SOUND */
  bufferptr += sound_context_save(&state[bufferptr]);

  /* 68000 */ 
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    uint16 tmp16;
    uint32 tmp32;
    tmp32 = m68k_get_reg(M68K_REG_D0);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D1);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D2);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D3);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D4);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D5);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D6);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D7);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A0);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A1);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A2);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A3);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A4);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A5);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A6);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A7);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_PC);  save_param(&tmp32, 4);
    tmp16 = m68k_get_reg(M68K_REG_SR);  save_param(&tmp16, 2); 
    tmp32 = m68k_get_reg(M68K_REG_USP); save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_ISP); save_param(&tmp32, 4);

    save_param(&m68k.cycles, sizeof(m68k.cycles));
    save_param(&m68k.int_level, sizeof(m68k.int_level));
    save_param(&m68k.stopped, sizeof(m68k.stopped));
  }

  /* Z80 */ 
  save_param(&Z80, sizeof(Z80_Regs));

  /* External HW */
  if (system_hw == SYSTEM_MCD)
  {
    /* CD hardware ID flag */
    char id[5];
    strncpy(id,"SCD!",4);
    save_param(id, 4);

    /* CD hardware */
    bufferptr += scd_context_save(&state[bufferptr]);
  }
  else if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    /* MD cartridge hardware */
    bufferptr += md_cart_context_save(&state[bufferptr]);
  }
  else
  {
    /* MS cartridge hardware */
    bufferptr += sms_cart_context_save(&state[bufferptr]);
  }

  /* return total size */
  return bufferptr;
}
