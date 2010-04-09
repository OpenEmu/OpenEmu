/****************************************************************************
 *  Genesis Plus
 *  Serial EEPROM support
 *
 *  Copyright (C) 2007, 2008, 2009  Eke-Eke (GCN/Wii port)
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

#define GAME_CNT 25

T_EEPROM eeprom;

typedef struct
{
  char game_id[14];
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
  {{"T-81406"    }, 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NBA Jam TE */
  {{"T-081276"   }, 0,      {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NFL Quarterback Club */
  /* 24C16 */
  {{"T-081586"   }, 0,      {8,  0x7FF,  0x7FF,  0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* NFL Quarterback Club '96 */
  /* 24C65 */
  {{"T-81576"    }, 0,      {16, 0x1FFF, 0x1FFF, 0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* College Slam */
  {{"T-81476"    }, 0,      {16, 0x1FFF, 0x1FFF, 0x200001, 0x200001, 0x200000, 0, 0, 0}},   /* Frank Thomas Big Hurt Baseball */

  /* EA mapper (24C01 only) */
  {{"T-50396"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* NHLPA Hockey 93 (UE) */
  {{"T-50176"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 7, 7, 6}},   /* Rings of Power */

  /* SEGA mapper (24C01 only) */
  {{"T-12046"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Megaman - The Wily Wars */
  {{"T-12053"    }, 0xEA80, {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Rockman Mega World (J) [A] */
  {{"MK-1215"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Evander 'Real Deal' Holyfield's Boxing */
  {{"MK-1228"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (U) */
  {{"G-5538"     }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (J) */
  {{"PR-1993"    }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Greatest Heavyweights of the Ring (E) */
  {{"G-4060"     }, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Wonderboy in Monster World */
  {{"00001211-00"}, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Sports Talk Baseball */
  {{"00004076-00"}, 0,      {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},   /* Honoo no Toukyuuji Dodge Danpei */

  /* CODEMASTERS mapper */
  /* 24C01 */
  {{"T-120106"},    0,      {7,   0x7F, 0x7F,   0x300000, 0x380001, 0x300000, 0, 7, 1}},    /* Brian Lara Cricket */
  /* 24C08 */
  {{"T-120096"   }, 0,      {8,  0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines 2 - Turbo Tournament (E) */
  {{"00000000-00"}, 0x168B, {8,  0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Military */
  {{"00000000-00"}, 0xCEE0, {8,  0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Military (Bad)*/
  /* 24C16 */
  {{"00000000-00"}, 0x165E, {8,  0x7FF,  0x7FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Turbo Tournament 96 */
  {{"00000000-00"}, 0x2C41, {8,  0x7FF,  0x7FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},   /* Micro Machines Turbo Tournament 96 (Bad)*/
  /* 24C65 */
  {{"T-120146-50"}, 0,      {16, 0x1FFF, 0x1FFF, 0x300000, 0x380001, 0x300000, 0, 7, 1}}    /* Brian Lara Cricket 96, Shane Warne Cricket */
};

void eeprom_init()
{
  int i = 0;

  /* initialize eeprom */
  memset(&eeprom, 0, sizeof(T_EEPROM));
  eeprom.sda = eeprom.old_sda = 1;
  eeprom.scl = eeprom.old_scl = 1;
  eeprom.state = STAND_BY;

  /* no eeprom by default */
  sram.custom = 0;

  /* look into game database */
  while ((i<GAME_CNT) && (!sram.custom))
  {
    if (strstr(rominfo.product,database[i].game_id) != NULL)
    {
      /* additional check (Micro Machines, Rockman Mega World) */
      if ((database[i].chk == 0) || (database[i].chk == rominfo.checksum))
      {
        sram.custom = 1;
        sram.on = 1;
        sram.write = 1;
        memcpy(&eeprom.type, &database[i].type, sizeof(T_EEPROM_TYPE));
      }
    }
    i++;
  }

  /* Game not found in database but header seems to indicate it uses EEPROM */
  if (!sram.custom)
  {
    if ((sram.end - sram.start) < 2)
    {
      sram.custom = 1;
      sram.on     = 1;
      sram.write  = 1;
      
      /* set SEGA mapper as default */
      memcpy(&eeprom.type, &database[9].type, sizeof(T_EEPROM_TYPE));
    }
  }
}

static inline void Detect_START()
{
  if (eeprom.old_scl && eeprom.scl)
  {
    if (eeprom.old_sda && !eeprom.sda)
    {
      eeprom.cycles = 0;
      eeprom.slave_mask = 0;
      if (eeprom.type.address_bits == 7)
      {
        eeprom.word_address = 0;
        eeprom.state = GET_WORD_ADR_7BITS;
      }
      else eeprom.state = GET_SLAVE_ADR;
    }
  }
}

static inline void Detect_STOP()
{
  if (eeprom.old_scl && eeprom.scl)
  {
    if (!eeprom.old_sda && eeprom.sda)
    {
      eeprom.state = STAND_BY;
    }
  }
}

void eeprom_write(uint32 address, uint32 value, uint32 word_access)
{
  /* decode SCL and SDA value */
  if (word_access)
  {
    /* 16-bits access */
    if (eeprom.type.sda_in_adr == address) eeprom.sda = (value >> (8 + eeprom.type.sda_in_bit)) & 1;      /* MSB */
    else if (eeprom.type.sda_in_adr == (address | 1)) eeprom.sda = (value >> eeprom.type.sda_in_bit) & 1; /* LSB */
    else eeprom.sda = eeprom.old_sda;

    if (eeprom.type.scl_adr == address) eeprom.scl = (value >> (8 + eeprom.type.scl_bit)) & 1;      /* MSB */
    else if (eeprom.type.scl_adr == (address | 1)) eeprom.scl = (value >> eeprom.type.scl_bit) & 1; /* LSB */
    else eeprom.scl = eeprom.old_scl;
  }
  else
  {
    if (eeprom.type.sda_in_adr == address) eeprom.sda = (value >> eeprom.type.sda_in_bit) & 1;
    else eeprom.sda = eeprom.old_sda;

    if (eeprom.type.scl_adr == address) eeprom.scl = (value >> eeprom.type.scl_bit) & 1;
    else eeprom.scl = eeprom.old_scl;
  }

  /* EEPROM current state */
  switch (eeprom.state)
  {
    /* Standby Mode */
    case STAND_BY:
      Detect_START();
      Detect_STOP();
      break;

    /* Suspended Mode */
    case WAIT_STOP:
      Detect_STOP();
      break;

    /* Get Word Address 7 bits: MODE-1 only (24C01)
     * and R/W bit
     */
    case GET_WORD_ADR_7BITS:
      Detect_START();
      Detect_STOP();

      /* look for SCL LOW to HIGH transition */
      if (!eeprom.old_scl && eeprom.scl)
      {
        if (eeprom.cycles == 0) eeprom.cycles ++;
      }


      /* look for SCL HIGH to LOW transition */
      if (eeprom.old_scl && !eeprom.scl && (eeprom.cycles > 0))
      {
        if (eeprom.cycles < 8)
        {
          eeprom.word_address |= (eeprom.old_sda << (7 - eeprom.cycles));
        }
        else if (eeprom.cycles == 8)
        {
          eeprom.rw = eeprom.old_sda;
        }
        else
        {  /* ACK CYCLE */
          eeprom.cycles = 0;
          eeprom.word_address &= eeprom.type.size_mask;
          eeprom.state = eeprom.rw ? READ_DATA : WRITE_DATA;
        }

        eeprom.cycles ++;
      }
      break;


    /* Get Slave Address (3bits) : MODE-2 & MODE-3 only (24C01 - 24C512) (0-3bits, depending on the array size)
     * or/and Word Address MSB: MODE-2 only (24C04 - 24C16) (0-3bits, depending on the array size)
     * and R/W bit
     */
    case GET_SLAVE_ADR:

      Detect_START();
      Detect_STOP();

      /* look for SCL LOW to HIGH transition */
      if (!eeprom.old_scl && eeprom.scl)
      {
        if (eeprom.cycles == 0) eeprom.cycles ++;
      }

      /* look for SCL HIGH to LOW transition */
      if (eeprom.old_scl && !eeprom.scl && (eeprom.cycles > 0))
      {
        if ((eeprom.cycles > 4) && (eeprom.cycles <8))
        {
          if ((eeprom.type.address_bits == 16) ||
            (eeprom.type.size_mask < (1 << (15 - eeprom.cycles))))
          {
            /* this is a SLAVE ADDRESS bit */
            eeprom.slave_mask |= (eeprom.old_sda << (7 - eeprom.cycles));
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (eeprom.old_sda) eeprom.word_address |= (1 << (15 - eeprom.cycles));
            else eeprom.word_address &= ~(1 << (15 - eeprom.cycles));
          }
        }
        else if (eeprom.cycles == 8) eeprom.rw = eeprom.old_sda;
        else if (eeprom.cycles > 8)
        {
          /* ACK CYCLE */
          eeprom.cycles = 0;
          if (eeprom.type.address_bits == 16)
          {
            /* two ADDRESS bytes */
            eeprom.state = eeprom.rw ? READ_DATA : GET_WORD_ADR_HIGH;
            eeprom.slave_mask <<= 16;
          }
          else
          {
            /* one ADDRESS byte */
            eeprom.state = eeprom.rw ? READ_DATA : GET_WORD_ADR_LOW;
            eeprom.slave_mask <<= 8;
          }
        }

        eeprom.cycles ++;
      }
      break;

    /* Get Word Address MSB (4-8bits depending on the array size)
     * MODE-3 only (24C32 - 24C512)
     */
    case GET_WORD_ADR_HIGH:

      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (eeprom.old_scl && !eeprom.scl)
      {        
        if (eeprom.cycles < 9)
        {
          if ((eeprom.type.size_mask + 1) < (1 << (17 - eeprom.cycles)))
          {
            /* ignored bit: slave mask should be right-shifted by one  */
            eeprom.slave_mask >>= 1;
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (eeprom.old_sda) eeprom.word_address |= (1 << (16 - eeprom.cycles));
            else eeprom.word_address &= ~(1 << (16 - eeprom.cycles));
          }

          eeprom.cycles ++;
        }
        else
        {
          /* ACK CYCLE */
          eeprom.cycles = 1;
          eeprom.state = GET_WORD_ADR_LOW;
        }
      }
      break;


    /* Get Word Address LSB: 7bits (24C01) or 8bits (24C02-24C512)
     * MODE-2 and MODE-3 only (24C01 - 24C512)
     */
    case GET_WORD_ADR_LOW: 

      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (eeprom.old_scl && !eeprom.scl)
      {
        if (eeprom.cycles < 9)
        {
          if ((eeprom.type.size_mask + 1) < (1 << (9 - eeprom.cycles)))
          {
            /* ignored bit (X24C01): slave mask should be right-shifted by one  */
            eeprom.slave_mask >>= 1;
          }
          else
          {
            /* this is a WORD ADDRESS high bit */
            if (eeprom.old_sda) eeprom.word_address |= (1 << (8 - eeprom.cycles));
            else eeprom.word_address &= ~(1 << (8 - eeprom.cycles));
          }

          eeprom.cycles ++;
        }
        else
        {
          /* ACK CYCLE */
          eeprom.cycles = 1;
          eeprom.word_address &= eeprom.type.size_mask;
          eeprom.state = WRITE_DATA;
        }
      }
      break;


    /*
     * Read Cycle
     */
    case READ_DATA:

      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (eeprom.old_scl && !eeprom.scl)
      {
        if (eeprom.cycles < 9) eeprom.cycles ++;
        else
        {
          eeprom.cycles = 1;

          /* ACK not received */
          if (eeprom.old_sda) eeprom.state = WAIT_STOP;
         }
      }
      break;


    /*
     * Write Cycle
     */
    case WRITE_DATA:

      Detect_START();
      Detect_STOP();

      /* look for SCL HIGH to LOW transition */
      if (eeprom.old_scl && !eeprom.scl)
      {        
        if (eeprom.cycles < 9)
        {
          /* Write DATA bits (max 64kBytes) */
          uint16 sram_address = (eeprom.slave_mask | eeprom.word_address) & 0xFFFF;
          if (eeprom.old_sda) sram.sram[sram_address] |= (1 << (8 - eeprom.cycles));
          else sram.sram[sram_address] &= ~(1 << (8 - eeprom.cycles));

          if (eeprom.cycles == 8) 
          {
            /* WORD ADDRESS is incremented (roll up at maximum pagesize) */
            eeprom.word_address = (eeprom.word_address & (0xFFFF - eeprom.type.pagewrite_mask)) | 
                        ((eeprom.word_address + 1) & eeprom.type.pagewrite_mask);
          }

          eeprom.cycles ++;
        }
        else eeprom.cycles = 1;  /* ACK cycle */
      }
      break;
  }

  eeprom.old_scl = eeprom.scl;
  eeprom.old_sda = eeprom.sda;
}

uint32 eeprom_read(uint32 address, uint32 word_access)
{
  uint8 sda_out = eeprom.sda;

  /* EEPROM state */
  switch (eeprom.state)
  {
    case READ_DATA:
      if (eeprom.cycles < 9)
      {
        /* Return DATA bits (max 64kBytes) */
        uint16 sram_address = (eeprom.slave_mask | eeprom.word_address) & 0xffff;
        sda_out = (sram.sram[sram_address] >> (8 - eeprom.cycles)) & 1;

        if (eeprom.cycles == 8)
        {
          /* WORD ADDRESS is incremented (roll up at maximum array size) */
          eeprom.word_address ++;
          eeprom.word_address &= eeprom.type.size_mask;
        }
      }
      break;

    case GET_WORD_ADR_7BITS:
    case GET_SLAVE_ADR:
    case GET_WORD_ADR_HIGH:
    case GET_WORD_ADR_LOW:
    case WRITE_DATA:
      if (eeprom.cycles == 9) sda_out = 0;
      break;

    default:
      break;
  }

  /* memory access */
  if (word_access)
  {
    /* 16-bits access */
    if (eeprom.type.sda_out_adr & 1) return (sda_out << eeprom.type.sda_out_bit); /* LSB */
    else return (sda_out << (eeprom.type.sda_out_bit + 8));  /* MSB */
  }
  else return (sda_out << eeprom.type.sda_out_bit);
}
