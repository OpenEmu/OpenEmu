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

#ifndef _MD_EEPROM_H_
#define _MD_EEPROM_H_

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

} T_EEPROM_24C;

/* global variables */
extern T_EEPROM_24C md_eeprom;

/* Function prototypes */
extern void md_eeprom_init();
extern unsigned int md_eeprom_read_byte(unsigned int address);
extern unsigned int md_eeprom_read_word(unsigned int address);
extern void md_eeprom_write_byte(unsigned int address, unsigned int data);
extern void md_eeprom_write_word(unsigned int address, unsigned int data);

#endif
