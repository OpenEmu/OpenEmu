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

#define GAME_CNT 28

/* this defines the type of EEPROM inside the game cartridge as Backup RAM
 *
 * Here are some notes from 8BitWizard (http://www.spritesmind.net/_GenDev/forum):
 *
 * Mode 1 (7-bit) - the chip takes a single byte with a 7-bit memory address and a R/W bit (24C01)
 * Mode 2 (8-bit) - the chip takes a 7-bit device address and R/W bit followed by an 8-bit memory address;
 * the device address may contain up to three more memory address bits (24C01 - 24C16).
 * You can also string eight 24C01, four 24C02, two 24C08, or various combinations, set their address config lines correctly,
 * and the result appears exactly the same as a 24C16
 * Mode 3 (16-bit) - the chip takes a 7-bit device address and R/W bit followed by a 16-bit memory address (24C32 and larger)
 *
 * Also, while most 24Cxx are addressed at 200000-2FFFFF, I have found two different ways of mapping the control lines. 
 * EA uses SDA on D7 (read/write) and SCL on D6 (write only), and I have found boards using different mapping (I think Accolade)
 * which uses D1-read=SDA, D0-write=SDA, D1-write=SCL. Accolade also has a custom-chip mapper which may even use a third method. 
 */

typedef struct
{
  uint8 address_bits;     /* number of bits needed to address memory: 7, 8 or 16 */
  uint16 size_mask;       /* depends on the max size of the memory (in bytes) */
  uint16 pagewrite_mask;  /* depends on the maximal number of bytes that can be written in a single write cycle */
  uint32 sda_in_adr;      /* 68000 memory address mapped to SDA_IN */
  uint32 sda_out_adr;     /* 68000 memory address mapped to SDA_OUT */
  uint32 scl_adr;         /* 68000 memory address mapped to SCL */
  uint8 sda_in_bit;       /* bit offset for SDA_IN */
  uint8 sda_out_bit;      /* bit offset for SDA_OUT */
  uint8 scl_bit;          /* bit offset for SCL */
} T_CONFIG_I2C;

typedef enum
{
  STAND_BY = 0,
  WAIT_STOP,
  GET_SLAVE_ADR,
  GET_WORD_ADR_7BITS,
  GET_WORD_ADR_HIGH,
  GET_WORD_ADR_LOW,
  WRITE_DATA,
  READ_DATA
} T_STATE_I2C;

typedef struct
{
  uint8 sda;            /* current /SDA line state */
  uint8 scl;            /* current /SCL line state */
  uint8 old_sda;        /* previous /SDA line state */
  uint8 old_scl;        /* previous /SCL line state */
  uint8 cycles;         /* current operation cycle number (0-9) */
  uint8 rw;             /* operation type (1:READ, 0:WRITE) */
  uint16 slave_mask;    /* device address (shifted by the memory address width)*/
  uint16 word_address;  /* memory address */
  T_STATE_I2C state;    /* current operation state */
  T_CONFIG_I2C config;  /* EEPROM characteristics for this game */
} T_EEPROM_I2C;

typedef struct
{
  char game_id[16];
  uint16 chk;
  T_CONFIG_I2C config;
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

  /* EA mapper (X24C01 only) */
  {{"T-50176"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* Rings of Power */
  {{"T-50396"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* NHLPA Hockey 93 */
  {{"T-50446"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* John Madden Football 93 */
  {{"T-50516"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* John Madden Football 93 (Championship Ed.) */
  {{"T-50606"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* Bill Walsh College Football */

  /* SEGA mapper (X24C01 only) */
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

static T_EEPROM_I2C eeprom_i2c;

static unsigned int eeprom_i2c_read_byte(unsigned int address);
static unsigned int eeprom_i2c_read_word(unsigned int address);
static void eeprom_i2c_write_byte(unsigned int address, unsigned int data);
static void eeprom_i2c_write_word(unsigned int address, unsigned int data);

void eeprom_i2c_init()
{
  int i = 0;

  /* initialize eeprom */
  memset(&eeprom_i2c, 0, sizeof(T_EEPROM_I2C));
  eeprom_i2c.sda = eeprom_i2c.old_sda = 1;
  eeprom_i2c.scl = eeprom_i2c.old_scl = 1;
  eeprom_i2c.state = STAND_BY;

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
        memcpy(&eeprom_i2c.config, &database[i].config, sizeof(T_CONFIG_I2C));
        i = GAME_CNT;
      }
    }
    i++;
  }

  /* Game not found in database but ROM header indicates it uses EEPROM */
  if (!sram.custom && sram.detected)
  {
    if ((sram.end - sram.start) < 2)
    {
      /* set SEGA mapper as default */
      sram.custom = 1;
      memcpy(&eeprom_i2c.config, &database[9].config, sizeof(T_CONFIG_I2C));
    }
  }

  /* initialize m68k bus handlers */
  if (sram.custom)
  {
    m68k.memory_map[eeprom_i2c.config.sda_out_adr >> 16].read8   = eeprom_i2c_read_byte;
    m68k.memory_map[eeprom_i2c.config.sda_out_adr >> 16].read16  = eeprom_i2c_read_word;
    m68k.memory_map[eeprom_i2c.config.sda_in_adr >> 16].read8    = eeprom_i2c_read_byte;
    m68k.memory_map[eeprom_i2c.config.sda_in_adr >> 16].read16   = eeprom_i2c_read_word;
    m68k.memory_map[eeprom_i2c.config.scl_adr >> 16].write8      = eeprom_i2c_write_byte;
    m68k.memory_map[eeprom_i2c.config.scl_adr >> 16].write16     = eeprom_i2c_write_word;
    zbank_memory_map[eeprom_i2c.config.sda_out_adr >> 16].read   = eeprom_i2c_read_byte;
    zbank_memory_map[eeprom_i2c.config.sda_in_adr >> 16].read    = eeprom_i2c_read_byte;
    zbank_memory_map[eeprom_i2c.config.scl_adr >> 16].write      = eeprom_i2c_write_byte;
  }
}

INLINE void Detect_START()
{
  if (eeprom_i2c.old_scl && eeprom_i2c.scl)
  {
    if (eeprom_i2c.old_sda && !eeprom_i2c.sda)
    {
      eeprom_i2c.cycles = 0;
      eeprom_i2c.slave_mask = 0;
      if (eeprom_i2c.config.address_bits == 7)
      {
        eeprom_i2c.word_address = 0;
        eeprom_i2c.state = GET_WORD_ADR_7BITS;
      }
      else eeprom_i2c.state = GET_SLAVE_ADR;
    }
  }
}

INLINE void Detect_STOP()
{
  if (eeprom_i2c.old_scl && eeprom_i2c.scl)
  {
    if (!eeprom_i2c.old_sda && eeprom_i2c.sda)
    {
      eeprom_i2c.state = STAND_BY;
    }
  }
}

static void eeprom_i2c_update(void)
{
  /* EEPROM current state */
  switch (eeprom_i2c.state)
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
      if (!eeprom_i2c.old_scl && eeprom_i2c.scl)
      {
        if (eeprom_i2c.cycles == 0) eeprom_i2c.cycles ++;
      }


      /* look for SCL HIGH to LOW transition */
      if (eeprom_i2c.old_scl && !eeprom_i2c.scl && (eeprom_i2c.cycles > 0))
      {
        if (eeprom_i2c.cycles < 8)
        {
          eeprom_i2c.word_address |= (eeprom_i2c.old_sda << (7 - eeprom_i2c.cycles));
        }
        else if (eeprom_i2c.cycles == 8)
        {
          eeprom_i2c.rw = eeprom_i2c.old_sda;
        }
        else
        {  /* ACK CYCLE */
          eeprom_i2c.cycles = 0;
          eeprom_i2c.word_address &= eeprom_i2c.config.size_mask;
          eeprom_i2c.state = eeprom_i2c.rw ? READ_DATA : WRITE_DATA;
        }

        eeprom_i2c.cycles ++;
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
      if (!eeprom_i2c.old_scl && eeprom_i2c.scl)
      {
        if (eeprom_i2c.cycles == 0) eeprom_i2c.cycles ++;
      }

      /* look for SCL HIGH to LOW transition */
      if (eeprom_i2c.old_scl && !eeprom_i2c.scl && (eeprom_i2c.cycles > 0))
      {
        if ((eeprom_i2c.cycles > 4) && (eeprom_i2c.cycles <8))
        {
          if ((eeprom_i2c.config.address_bits == 16) ||
            (eeprom_i2c.config.size_mask < (1 << (15 - eeprom_i2c.cycles))))
          {
            /* this is a SLAVE ADDRESS bit */
            eeprom_i2c.slave_mask |= (eeprom_i2c.old_sda << (7 - eeprom_i2c.cycles));
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (eeprom_i2c.old_sda) eeprom_i2c.word_address |= (1 << (15 - eeprom_i2c.cycles));
            else eeprom_i2c.word_address &= ~(1 << (15 - eeprom_i2c.cycles));
          }
        }
        else if (eeprom_i2c.cycles == 8) eeprom_i2c.rw = eeprom_i2c.old_sda;
        else if (eeprom_i2c.cycles > 8)
        {
          /* ACK CYCLE */
          eeprom_i2c.cycles = 0;
          if (eeprom_i2c.config.address_bits == 16)
          {
            /* two ADDRESS bytes */
            eeprom_i2c.state = eeprom_i2c.rw ? READ_DATA : GET_WORD_ADR_HIGH;
            eeprom_i2c.slave_mask <<= 16;
          }
          else
          {
            /* one ADDRESS byte */
            eeprom_i2c.state = eeprom_i2c.rw ? READ_DATA : GET_WORD_ADR_LOW;
            eeprom_i2c.slave_mask <<= 8;
          }
        }

        eeprom_i2c.cycles ++;
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
      if (eeprom_i2c.old_scl && !eeprom_i2c.scl)
      {
        if (eeprom_i2c.cycles < 9)
        {
          if ((eeprom_i2c.config.size_mask + 1) < (1 << (17 - eeprom_i2c.cycles)))
          {
            /* ignored bit: slave mask should be right-shifted by one  */
            eeprom_i2c.slave_mask >>= 1;
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (eeprom_i2c.old_sda) eeprom_i2c.word_address |= (1 << (16 - eeprom_i2c.cycles));
            else eeprom_i2c.word_address &= ~(1 << (16 - eeprom_i2c.cycles));
          }

          eeprom_i2c.cycles ++;
        }
        else
        {
          /* ACK CYCLE */
          eeprom_i2c.cycles = 1;
          eeprom_i2c.state = GET_WORD_ADR_LOW;
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
      if (eeprom_i2c.old_scl && !eeprom_i2c.scl)
      {
        if (eeprom_i2c.cycles < 9)
        {
          if ((eeprom_i2c.config.size_mask + 1) < (1 << (9 - eeprom_i2c.cycles)))
          {
            /* ignored bit (X24C01): slave mask should be right-shifted by one  */
            eeprom_i2c.slave_mask >>= 1;
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (eeprom_i2c.old_sda) eeprom_i2c.word_address |= (1 << (8 - eeprom_i2c.cycles));
            else eeprom_i2c.word_address &= ~(1 << (8 - eeprom_i2c.cycles));
          }

          eeprom_i2c.cycles ++;
        }
        else
        {
          /* ACK CYCLE */
          eeprom_i2c.cycles = 1;
          eeprom_i2c.word_address &= eeprom_i2c.config.size_mask;
          eeprom_i2c.state = WRITE_DATA;
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
      if (eeprom_i2c.old_scl && !eeprom_i2c.scl)
      {
        if (eeprom_i2c.cycles < 9) eeprom_i2c.cycles ++;
        else
        {
          eeprom_i2c.cycles = 1;

          /* ACK not received */
          if (eeprom_i2c.old_sda) eeprom_i2c.state = WAIT_STOP;
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
      if (eeprom_i2c.old_scl && !eeprom_i2c.scl)
      {
        if (eeprom_i2c.cycles < 9)
        {
          /* Write DATA bits (max 64kBytes) */
          uint16 sram_address = (eeprom_i2c.slave_mask | eeprom_i2c.word_address) & 0xFFFF;
          if (eeprom_i2c.old_sda) sram.sram[sram_address] |= (1 << (8 - eeprom_i2c.cycles));
          else sram.sram[sram_address] &= ~(1 << (8 - eeprom_i2c.cycles));

          if (eeprom_i2c.cycles == 8) 
          {
            /* WORD ADDRESS is incremented (roll up at maximum pagesize) */
            eeprom_i2c.word_address = (eeprom_i2c.word_address & (0xFFFF - eeprom_i2c.config.pagewrite_mask)) | 
                                     ((eeprom_i2c.word_address + 1) & eeprom_i2c.config.pagewrite_mask);
          }

          eeprom_i2c.cycles ++;
        }
        else eeprom_i2c.cycles = 1;  /* ACK cycle */
      }
      break;
    }
  }

  eeprom_i2c.old_scl = eeprom_i2c.scl;
  eeprom_i2c.old_sda = eeprom_i2c.sda;
}

static unsigned char eeprom_i2c_out(void)
{
  uint8 sda_out = eeprom_i2c.sda;

  /* EEPROM state */
  switch (eeprom_i2c.state)
  {
    case READ_DATA:
    {
      if (eeprom_i2c.cycles < 9)
      {
        /* Return DATA bits (max 64kBytes) */
        uint16 sram_address = (eeprom_i2c.slave_mask | eeprom_i2c.word_address) & 0xffff;
        sda_out = (sram.sram[sram_address] >> (8 - eeprom_i2c.cycles)) & 1;

        if (eeprom_i2c.cycles == 8)
        {
          /* WORD ADDRESS is incremented (roll up at maximum array size) */
          eeprom_i2c.word_address ++;
          eeprom_i2c.word_address &= eeprom_i2c.config.size_mask;
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
      if (eeprom_i2c.cycles == 9) sda_out = 0;
      break;
    }

    default:
    {
      break;
    }
  }

  return (sda_out << eeprom_i2c.config.sda_out_bit);
}

static unsigned int eeprom_i2c_read_byte(unsigned int address)
{
  if (address == eeprom_i2c.config.sda_out_adr)
  {
    return eeprom_i2c_out();
  }

  return READ_BYTE(cart.rom, address);
}

static unsigned int eeprom_i2c_read_word(unsigned int address)
{
  if (address == eeprom_i2c.config.sda_out_adr)
  {
    return (eeprom_i2c_out() << 8);
  }

  if (address == (eeprom_i2c.config.sda_out_adr ^ 1))
  {
    return eeprom_i2c_out();
  }

  return *(uint16 *)(cart.rom + address);
}

static void eeprom_i2c_write_byte(unsigned int address, unsigned int data)
{
  int do_update = 0;

  if (address == eeprom_i2c.config.sda_in_adr)
  {
    eeprom_i2c.sda = (data >> eeprom_i2c.config.sda_in_bit) & 1;
    do_update = 1;
  }

  if (address == eeprom_i2c.config.scl_adr)
  {
    eeprom_i2c.scl = (data >> eeprom_i2c.config.scl_bit) & 1;
    do_update = 1;
  }

  if (do_update)
  {
    eeprom_i2c_update();
    return;
  }

  m68k_unused_8_w(address, data);
}

static void eeprom_i2c_write_word(unsigned int address, unsigned int data)
{
  int do_update = 0;

  if (address == eeprom_i2c.config.sda_in_adr)
  {
    eeprom_i2c.sda = (data >> (8 + eeprom_i2c.config.sda_in_bit)) & 1;
    do_update = 1;
  }
  else if (address == (eeprom_i2c.config.sda_in_adr ^1))
  {
    eeprom_i2c.sda = (data >> eeprom_i2c.config.sda_in_bit) & 1;
    do_update = 1;
  }

  if (address == eeprom_i2c.config.scl_adr)
  {
    eeprom_i2c.scl = (data >> (8 + eeprom_i2c.config.scl_bit)) & 1;
    do_update = 1;
  }
  else if (address == (eeprom_i2c.config.scl_adr ^1))
  {
    eeprom_i2c.scl = (data >> eeprom_i2c.config.scl_bit) & 1;
    do_update = 1;
  }

  if (do_update)
  {
    eeprom_i2c_update();
    return;
  }

  m68k_unused_16_w(address, data);
}
