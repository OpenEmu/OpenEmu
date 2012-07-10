/****************************************************************************
 *  Genesis Plus
 *  I2C Serial EEPROM (24Cxx) support
 *
 *  Copyright (C) 2007-2011  Eke-Eke (Genesis Plus GX)
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
#include "md_eeprom.h"

#define GAME_CNT 28

T_EEPROM_24C md_eeprom;

typedef struct
{
  char game_id[16];
  uint16 chk;
  T_EEPROM_TYPE type;
} T_GAME_ENTRY;

static const T_GAME_ENTRY database[GAME_CNT] = 
{
  /* ACCLAIM mappers */
  /* 24C02 (old mapper) */
  {{"T-081326"   }, 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200001, 0, 1, 1}},   /* NBA Jam (UE) */
  {{"T-81033"    }, 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200001, 0, 1, 1}},   /* NBA Jam (J) */
  /* 24C02 */
  {{"T-081276"   }, 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NFL Quarterback Club */
  /* 24C04 */
  {{"T-81406"    }, 0,      {8,  0x1FF,  0x1FF,  0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NBA Jam TE */
  /* 24C16 */
  {{"T-081586"   }, 0,      {8,  0x7FF,  0x7FF,  0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NFL Quarterback Club '96 */
  /* 24C65 */
  {{"T-81576"    }, 0,      {16, 0x1FFF, 0x1FFF, 0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* College Slam */
  {{"T-81476"    }, 0,      {16, 0x1FFF, 0x1FFF, 0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* Frank Thomas Big Hurt Baseball */

  /* EA mapper (24C01 only) */
  {{"T-50176"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* Rings of Power */
  {{"T-50396"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* NHLPA Hockey 93 */
  {{"T-50446"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* John Madden Football 93 */
  {{"T-50516"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* John Madden Football 93 (Championship Ed.) */
  {{"T-50606"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* Bill Walsh College Football */

  /* SEGA mapper (24C01 only) */
  {{"T-12046"    }, 0xAD23, {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Megaman - The Wily Wars */
  {{"T-12053"    }, 0xEA80, {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Rockman Mega World [Alt] */
  {{"MK-1215"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Evander 'Real Deal' Holyfield's Boxing */
  {{"MK-1228"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (U) */
  {{"G-5538"     }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (J) */
  {{"PR-1993"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (E) */
  {{"G-4060"     }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Wonderboy in Monster World */
  {{"00001211-00"}, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Sports Talk Baseball */
  {{"00004076-00"}, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Honoo no Toukyuuji Dodge Danpei */
  {{"G-4524"     }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Ninja Burai Densetsu */
  {{"00054503-00"}, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Game Toshokan  */

  /* CODEMASTERS mapper */
  /* 24C08 */
  {{"T-120106"   }, 0,      {8,  0x3FF, 0x3FF,   0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Brian Lara Cricket */
  {{"00000000-00"}, 0xCEE0, {8,  0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Military */
  /* 24C16 */
  {{"T-120096"   }, 0,      {8,  0x7FF,  0x7FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines 2 - Turbo Tournament */
  {{"00000000-00"}, 0x2C41, {8,  0x7FF,  0x7FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Turbo Tournament 96 */
  /* 24C65 */
  {{"T-120146-50"}, 0,      {16, 0x1FFF, 0x1FFF, 0x300000, 0x380001, 0x300000, 0, 7, 1}}    /* Brian Lara Cricket 96, Shane Warne Cricket */
};


static void eeprom_update(void);
static unsigned char eeprom_out(void);

void md_eeprom_init()
{
  int i = 0;

  /* initialize eeprom */
  memset(&md_eeprom, 0, sizeof(T_EEPROM_24C));
  md_eeprom.sda = md_eeprom.old_sda = 1;
  md_eeprom.scl = md_eeprom.old_scl = 1;
  md_eeprom.state = STAND_BY;

  /* no eeprom by default */
  sram.custom = 0;

  /* look into game database */
  while (i<GAME_CNT)
  {
    if (strstr(rominfo.product,database[i].game_id) != NULL)
    {
      /* additional check (Micro Machines, Rockman Mega World) */
      if ((database[i].chk == 0x0000) || (database[i].chk == rominfo.realchecksum))
      {
        sram.custom = 1;
        sram.on = 1;
        memcpy(&md_eeprom.type, &database[i].type, sizeof(T_EEPROM_TYPE));
        return;
      }
    }
    i++;
  }

  /* Game not found in database but ROM header indicates it uses EEPROM */
  if (sram.detected)
  {
    if ((sram.end - sram.start) < 2)
    {
      /* set SEGA mapper as default */
      sram.custom = 1;
      memcpy(&md_eeprom.type, &database[9].type, sizeof(T_EEPROM_TYPE));
    }
  }
}

unsigned int md_eeprom_read_byte(unsigned int address)
{
  if (address == md_eeprom.type.sda_out_adr)
  {
    return eeprom_out();
  }

  return READ_BYTE(cart.rom, address);
}

unsigned int md_eeprom_read_word(unsigned int address)
{
  if (address == md_eeprom.type.sda_out_adr)
  {
    return (eeprom_out() << 8);
  }

  if (address == (md_eeprom.type.sda_out_adr ^ 1))
  {
    return eeprom_out();
  }

  return *(uint16 *)(cart.rom + address);
}

void md_eeprom_write_byte(unsigned int address, unsigned int data)
{
  int do_update = 0;

  if (address == md_eeprom.type.sda_in_adr)
  {
    md_eeprom.sda = (data >> md_eeprom.type.sda_in_bit) & 1;
    do_update = 1;
  }

  if (address == md_eeprom.type.scl_adr)
  {
    md_eeprom.scl = (data >> md_eeprom.type.scl_bit) & 1;
    do_update = 1;
  }

  if (do_update)
  {
    eeprom_update();
    return;
  }

  m68k_unused_8_w(address, data);
}

void md_eeprom_write_word(unsigned int address, unsigned int data)
{
  int do_update = 0;

  if (address == md_eeprom.type.sda_in_adr)
  {
    md_eeprom.sda = (data >> (8 + md_eeprom.type.sda_in_bit)) & 1;
    do_update = 1;
  }
  else if (address == (md_eeprom.type.sda_in_adr ^1))
  {
    md_eeprom.sda = (data >> md_eeprom.type.sda_in_bit) & 1;
    do_update = 1;
  }

  if (address == md_eeprom.type.scl_adr)
  {
    md_eeprom.scl = (data >> (8 + md_eeprom.type.scl_bit)) & 1;
    do_update = 1;
  }
  else if (address == (md_eeprom.type.scl_adr ^1))
  {
    md_eeprom.scl = (data >> md_eeprom.type.scl_bit) & 1;
    do_update = 1;
  }

  if (do_update)
  {
    eeprom_update();
    return;
  }

  m68k_unused_16_w(address, data);
}


INLINE void Detect_START()
{
  if (md_eeprom.old_scl && md_eeprom.scl)
  {
    if (md_eeprom.old_sda && !md_eeprom.sda)
    {
      md_eeprom.cycles = 0;
      md_eeprom.slave_mask = 0;
      if (md_eeprom.type.address_bits == 7)
      {
        md_eeprom.word_address = 0;
        md_eeprom.state = GET_WORD_ADR_7BITS;
      }
      else md_eeprom.state = GET_SLAVE_ADR;
    }
  }
}

INLINE void Detect_STOP()
{
  if (md_eeprom.old_scl && md_eeprom.scl)
  {
    if (!md_eeprom.old_sda && md_eeprom.sda)
    {
      md_eeprom.state = STAND_BY;
    }
  }
}

static void eeprom_update(void)
{
  /* EEPROM current state */
  switch (md_eeprom.state)
  {
    /* Standby Mode */
    case STAND_BY:
    {
      Detect_START();
      Detect_STOP();
      break;
    }

    /* Suspended Mode */
    case WAIT_STOP:
    {
      Detect_STOP();
      break;
    }

    /* Get Word Address 7 bits: MODE-1 only (24C01)
     * and R/W bit
     */
    case GET_WORD_ADR_7BITS:
    {
      Detect_START();
      Detect_STOP();

      /* look for SCL LOW to HIGH transition */
      if (!md_eeprom.old_scl && md_eeprom.scl)
      {
        if (md_eeprom.cycles == 0) md_eeprom.cycles ++;
      }


      /* look for SCL HIGH to LOW transition */
      if (md_eeprom.old_scl && !md_eeprom.scl && (md_eeprom.cycles > 0))
      {
        if (md_eeprom.cycles < 8)
        {
          md_eeprom.word_address |= (md_eeprom.old_sda << (7 - md_eeprom.cycles));
        }
        else if (md_eeprom.cycles == 8)
        {
          md_eeprom.rw = md_eeprom.old_sda;
        }
        else
        {  /* ACK CYCLE */
          md_eeprom.cycles = 0;
          md_eeprom.word_address &= md_eeprom.type.size_mask;
          md_eeprom.state = md_eeprom.rw ? READ_DATA : WRITE_DATA;
        }

        md_eeprom.cycles ++;
      }
      break;
    }

    /* Get Slave Address (3bits) : MODE-2 & MODE-3 only (24C01 - 24C512) (0-3bits, depending on the array size)
     * or/and Word Address MSB: MODE-2 only (24C04 - 24C16) (0-3bits, depending on the array size)
     * and R/W bit
     */
    case GET_SLAVE_ADR:
    {
      Detect_START();
      Detect_STOP();

      /* look for SCL LOW to HIGH transition */
      if (!md_eeprom.old_scl && md_eeprom.scl)
      {
        if (md_eeprom.cycles == 0) md_eeprom.cycles ++;
      }

      /* look for SCL HIGH to LOW transition */
      if (md_eeprom.old_scl && !md_eeprom.scl && (md_eeprom.cycles > 0))
      {
        if ((md_eeprom.cycles > 4) && (md_eeprom.cycles <8))
        {
          if ((md_eeprom.type.address_bits == 16) ||
            (md_eeprom.type.size_mask < (1 << (15 - md_eeprom.cycles))))
          {
            /* this is a SLAVE ADDRESS bit */
            md_eeprom.slave_mask |= (md_eeprom.old_sda << (7 - md_eeprom.cycles));
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (md_eeprom.old_sda) md_eeprom.word_address |= (1 << (15 - md_eeprom.cycles));
            else md_eeprom.word_address &= ~(1 << (15 - md_eeprom.cycles));
          }
        }
        else if (md_eeprom.cycles == 8) md_eeprom.rw = md_eeprom.old_sda;
        else if (md_eeprom.cycles > 8)
        {
          /* ACK CYCLE */
          md_eeprom.cycles = 0;
          if (md_eeprom.type.address_bits == 16)
          {
            /* two ADDRESS bytes */
            md_eeprom.state = md_eeprom.rw ? READ_DATA : GET_WORD_ADR_HIGH;
            md_eeprom.slave_mask <<= 16;
          }
          else
          {
            /* one ADDRESS byte */
            md_eeprom.state = md_eeprom.rw ? READ_DATA : GET_WORD_ADR_LOW;
            md_eeprom.slave_mask <<= 8;
          }
        }

        md_eeprom.cycles ++;
      }
      break;
    }

    /* Get Word Address MSB (4-8bits depending on the array size)
     * MODE-3 only (24C32 - 24C512)
     */
    case GET_WORD_ADR_HIGH:
    {
      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (md_eeprom.old_scl && !md_eeprom.scl)
      {
        if (md_eeprom.cycles < 9)
        {
          if ((md_eeprom.type.size_mask + 1) < (1 << (17 - md_eeprom.cycles)))
          {
            /* ignored bit: slave mask should be right-shifted by one  */
            md_eeprom.slave_mask >>= 1;
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (md_eeprom.old_sda) md_eeprom.word_address |= (1 << (16 - md_eeprom.cycles));
            else md_eeprom.word_address &= ~(1 << (16 - md_eeprom.cycles));
          }

          md_eeprom.cycles ++;
        }
        else
        {
          /* ACK CYCLE */
          md_eeprom.cycles = 1;
          md_eeprom.state = GET_WORD_ADR_LOW;
        }
      }
      break;
    }

    /* Get Word Address LSB: 7bits (24C01) or 8bits (24C02-24C512)
     * MODE-2 and MODE-3 only (24C01 - 24C512)
     */
    case GET_WORD_ADR_LOW: 
    {
      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (md_eeprom.old_scl && !md_eeprom.scl)
      {
        if (md_eeprom.cycles < 9)
        {
          if ((md_eeprom.type.size_mask + 1) < (1 << (9 - md_eeprom.cycles)))
          {
            /* ignored bit (X24C01): slave mask should be right-shifted by one  */
            md_eeprom.slave_mask >>= 1;
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (md_eeprom.old_sda) md_eeprom.word_address |= (1 << (8 - md_eeprom.cycles));
            else md_eeprom.word_address &= ~(1 << (8 - md_eeprom.cycles));
          }

          md_eeprom.cycles ++;
        }
        else
        {
          /* ACK CYCLE */
          md_eeprom.cycles = 1;
          md_eeprom.word_address &= md_eeprom.type.size_mask;
          md_eeprom.state = WRITE_DATA;
        }
      }
      break;
    }

    /*
     * Read Cycle
     */
    case READ_DATA:
    {
      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (md_eeprom.old_scl && !md_eeprom.scl)
      {
        if (md_eeprom.cycles < 9) md_eeprom.cycles ++;
        else
        {
          md_eeprom.cycles = 1;

          /* ACK not received */
          if (md_eeprom.old_sda) md_eeprom.state = WAIT_STOP;
         }
      }
      break;
    }

    /*
     * Write Cycle
     */
    case WRITE_DATA:
    {
      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (md_eeprom.old_scl && !md_eeprom.scl)
      {
        if (md_eeprom.cycles < 9)
        {
          /* Write DATA bits (max 64kBytes) */
          uint16 sram_address = (md_eeprom.slave_mask | md_eeprom.word_address) & 0xFFFF;
          if (md_eeprom.old_sda) sram.sram[sram_address] |= (1 << (8 - md_eeprom.cycles));
          else sram.sram[sram_address] &= ~(1 << (8 - md_eeprom.cycles));

          if (md_eeprom.cycles == 8) 
          {
            /* WORD ADDRESS is incremented (roll up at maximum pagesize) */
            md_eeprom.word_address = (md_eeprom.word_address & (0xFFFF - md_eeprom.type.pagewrite_mask)) | 
                                     ((md_eeprom.word_address + 1) & md_eeprom.type.pagewrite_mask);
          }

          md_eeprom.cycles ++;
        }
        else md_eeprom.cycles = 1;  /* ACK cycle */
      }
      break;
    }
  }

  md_eeprom.old_scl = md_eeprom.scl;
  md_eeprom.old_sda = md_eeprom.sda;
}

static unsigned char eeprom_out(void)
{
  uint8 sda_out = md_eeprom.sda;

  /* EEPROM state */
  switch (md_eeprom.state)
  {
    case READ_DATA:
    {
      if (md_eeprom.cycles < 9)
      {
        /* Return DATA bits (max 64kBytes) */
        uint16 sram_address = (md_eeprom.slave_mask | md_eeprom.word_address) & 0xffff;
        sda_out = (sram.sram[sram_address] >> (8 - md_eeprom.cycles)) & 1;

        if (md_eeprom.cycles == 8)
        {
          /* WORD ADDRESS is incremented (roll up at maximum array size) */
          md_eeprom.word_address ++;
          md_eeprom.word_address &= md_eeprom.type.size_mask;
        }
      }
      break;
    }

    case GET_WORD_ADR_7BITS:
    case GET_SLAVE_ADR:
    case GET_WORD_ADR_HIGH:
    case GET_WORD_ADR_LOW:
    case WRITE_DATA:
    {
      if (md_eeprom.cycles == 9) sda_out = 0;
      break;
    }

    default:
    {
      break;
    }
  }

  return (sda_out << md_eeprom.type.sda_out_bit);
}
