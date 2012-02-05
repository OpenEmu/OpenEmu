/****************************************************************************
 *  Genesis Plus
 *  I2C EEPROM support
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

#ifndef _EEPROM_H_
#define _EEPROM_H_

typedef enum
{
  STAND_BY = 0,
  WAIT_STOP,
  GET_SLAVE_ADR,
  GET_WORD_ADR_7BITS,
  GET_WORD_ADR_HIGH,
  GET_WORD_ADR_LOW,
  WRITE_DATA,
  READ_DATA,

} T_EEPROM_STATE;

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

} T_EEPROM_TYPE;


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
  T_EEPROM_STATE state; /* current operation state */
  T_EEPROM_TYPE type;   /* EEPROM characteristics for this game */

} T_EEPROM;

/* global variables */
extern T_EEPROM eeprom;

/* Function prototypes */
extern void eeprom_init();
extern void eeprom_write(unsigned int address, unsigned int data, int word_access);
extern unsigned int eeprom_read(int word_access);

#endif
