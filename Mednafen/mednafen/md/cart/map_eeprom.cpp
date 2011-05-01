/* Mednafen - Multi-system Emulator
 *
 *  Copyright (C) 2007 EkeEke
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 Genesis Plus GX EEPROM emulation modified 2008 for usage in Mednafen
*/

#include "../shared.h"
#include "cart.h"
#include "map_eeprom.h"

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
        uint8 address_bits;             /* number of bits needed to address memory: 7, 8 or 16 */
        uint16 size_mask;               /* depends on the max size of the memory (in bytes) */
        uint16 pagewrite_mask;  /* depends on the maximal number of bytes that can be written in a single write cycle */
        uint32 sda_in_adr;              /* 68000 memory address mapped to SDA_IN */
        uint32 sda_out_adr;             /* 68000 memory address mapped to SDA_OUT */
        uint32 scl_adr;                 /* 68000 memory address mapped to SCL */
        uint8 sda_in_bit;               /* bit offset for SDA_IN */
        uint8 sda_out_bit;              /* bit offset for SDA_OUT */
        uint8 scl_bit;                  /* bit offset for SCL */

} T_EEPROM_TYPE;

typedef struct
{
 int board_type;
 T_EEPROM_TYPE type;
} EEPROM_Config;

static const EEPROM_Config database[EEP_TOTAL] = 
{
        /* ACCLAIM mappers */
        /* 24C02 (old mapper) */
        { EEP_ACCLAIM_24C02_OLD,{8,  0xFF,   0xFF,   0x200000, 0x200000, 0x200000, 0, 1, 1}},

        /* 24C02 */
        { EEP_ACCLAIM_24C02,    {8,  0xFF,   0xFF,   0x200001, 0x200001, 0x200000, 0, 0, 0}},

        /* 24C16 */
        { EEP_ACCLAIM_24C16,    {8,  0x7FF,  0x7FF,  0x200001, 0x200001, 0x200000, 0, 0, 0}},

        /* 24C65 */
        {EEP_ACCLAIM_24C65,     {16, 0x1FFF, 0x1FFF, 0x200001, 0x200001, 0x200000, 0, 0, 0}},
        
        /* EA mapper (24C01 only) */
        { EEP_EA_24C01,         {7,  0x7F,   0x7F,   0x200000, 0x200000, 0x200000, 7, 7, 6}},
        
        /* SEGA mapper (24C01 only) */
        { EEP_SEGA_24C01,       {7,  0x7F,   0x7F,   0x200001, 0x200001, 0x200001, 0, 0, 1}},
        { EEP_SEGA_24C01_ALT,   {7,  0x7F,   0x7F,   0x200000, 0x200000, 0x200000, 0, 0, 1}},
        
        /* CODEMASTERS mapper */
        /* 24C01 */
        { EEP_CM_24C01,         {7, 0x7F, 0x7F,   0x300000, 0x380001, 0x300000, 0, 7, 1}},

        /* 24C08 */
        { EEP_CM_24C08,         {8, 0x3FF,  0x3FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},

        /* 24C16 */
        { EEP_CM_24C16,         {8,  0x7FF,  0x7FF,  0x300000, 0x380001, 0x300000, 0, 7, 1}},

        /* 24C65 */
        { EEP_CM_24C65,         {16, 0x1FFF, 0x1FFF, 0x300000, 0x380001, 0x300000, 0, 7, 1}}
};


class MD_Cart_Type_EEPROM : public MD_Cart_Type
{
	public:

        MD_Cart_Type_EEPROM(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const int BoardType);
        virtual ~MD_Cart_Type_EEPROM();
	virtual void Reset(void);

        virtual void Write8(uint32 A, uint8 V);
        virtual void Write16(uint32 A, uint8 V);
        virtual uint8 Read8(uint32 A);
        virtual uint16 Read16(uint32 A);
        virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);

        // In bytes
        virtual uint32 GetNVMemorySize(void);
        virtual void ReadNVMemory(uint8 *buffer);
        virtual void WriteNVMemory(const uint8 *buffer);

	private:

	void WriteEEPROM(unsigned int address, unsigned int value);
	unsigned int ReadEEPROM(unsigned int address);


	const uint8 *rom;
	uint32 rom_size;
	
        void Detect_START();
        void Detect_STOP();

        uint8 sda;                              /* current /SDA line state */
        uint8 scl;                              /* current /SCL line state */
        uint8 old_sda;                  /* previous /SDA line state */
        uint8 old_scl;                  /* previous /SCL line state */
        uint8 cycles;                   /* current operation cycle number (0-9) */
        uint8 rw;                               /* operation type (1:READ, 0:WRITE) */
        uint16 slave_mask;              /* device address (shifted by the memory address width)*/
        uint16 word_address;    /* memory address */

        uint8 *sram;

        T_EEPROM_STATE state;   /* current operation state */
        T_EEPROM_TYPE type;             /* EEPROM characteristics for this game */

	uint32 eeprom_start, eeprom_end;
};


MD_Cart_Type_EEPROM::MD_Cart_Type_EEPROM(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const int BoardType)
{
 bool found_board_type = FALSE;

 this->rom = ROM;
 this->rom_size = ROM_size;

 /* initialize eeprom */
 sda = old_sda = 1;
 scl = old_scl = 1;
 state = STAND_BY;

 for(int i = 0; i < EEP_TOTAL; i++)
 {
  if(database[i].board_type == BoardType)
  {
   memcpy(&type, &database[i].type, sizeof(T_EEPROM_TYPE));
   found_board_type = TRUE;
   break;
  }
 }

 // Debugging sanity check
 if(!found_board_type)
  throw(0);

 /* set SRAM start & end address */
 eeprom_start = eeprom_end = type.sda_in_adr;
 if(eeprom_end < type.sda_out_adr) eeprom_end = type.sda_out_adr;
 if(eeprom_end < type.scl_adr) eeprom_end = type.scl_adr;
 if(eeprom_start > type.sda_out_adr) eeprom_start = type.sda_out_adr;
 if(eeprom_start > type.scl_adr) eeprom_start = type.scl_adr;

 if(!(sram = (uint8 *)MDFN_malloc(type.size_mask + 1, _("Cart EEPROM"))))
 {
  // FIXME
  throw(-1);
 }

 memset(sram, 0xFF, type.size_mask + 1);

 printf("%08x %08x\n", eeprom_start, eeprom_end);
}

MD_Cart_Type_EEPROM::~MD_Cart_Type_EEPROM()
{
 if(sram)
  MDFN_free(sram);
}

INLINE void MD_Cart_Type_EEPROM::Detect_START()
{
        if (old_scl && scl)
        {
                if (old_sda && !sda)
                {
                        cycles = 0;
                        slave_mask = 0;
                        if (type.address_bits == 7)
                        {
                                word_address = 0;
                                state = GET_WORD_ADR_7BITS;
                        }
                        else state = GET_SLAVE_ADR;
                }
        }
}

INLINE void MD_Cart_Type_EEPROM::Detect_STOP()
{
        if (old_scl && scl)
        {
                if (!old_sda && sda)
                {
                        state = STAND_BY;
                }
        }
}

void MD_Cart_Type_EEPROM::Reset(void)
{

}

void MD_Cart_Type_EEPROM::WriteEEPROM(unsigned int address, unsigned int value)
{
        uint16 sram_address = 0;

	//printf("Write: %08x %08x\n", address, value);

        /* decode SCL and SDA value */
        if (type.sda_in_adr == address) sda = (value >> type.sda_in_bit) & 1;
        else sda = old_sda;
        if (type.scl_adr == address) scl = (value >> type.scl_bit) & 1;
        else scl = old_scl;
        
        /* EEPROM current state */
        switch (state)
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
                        if (!old_scl && scl)
                        {
                                if (cycles == 0) cycles ++;
                        }

                        /* look for SCL HIGH to LOW transition */
                        if (old_scl && !scl && (cycles > 0))
                        {                               
                                if (cycles < 8) word_address |= (old_sda << (7 - cycles));
                                else if (cycles == 8) rw = old_sda;
                                else
                                {       /* ACK CYCLE */
                                        cycles = 0;
                                        word_address &= type.size_mask;
                                        state = rw ? READ_DATA : WRITE_DATA;
                                }

                                cycles ++;
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
                        if (!old_scl && scl)
                        {
                                if (cycles == 0) cycles ++;
                        }

                        /* look for SCL HIGH to LOW transition */
                        if (old_scl && !scl && (cycles > 0))
                        {
                                if ((cycles > 4) && (cycles <8))
                                {
                                        if ((type.address_bits == 16) ||
                                                (type.size_mask < (1 << (15 - cycles))))
                                        {
                                                /* this is a SLAVE ADDRESS bit */
                                                slave_mask |= (old_sda << (7 - cycles));
                                        }
                                        else
                                        {
                                                /* this is a WORD ADDRESS high bit */
                                                if (old_sda) word_address |= (1 << (15 - cycles));
                                                else word_address &= ~(1 << (15 - cycles));
                                        }
                                }
                                else if (cycles == 8) rw = old_sda;
                                else if (cycles > 8)
                                {
                                        /* ACK CYCLE */
                                        cycles = 0;
                                        if (type.address_bits == 16)
                                        {
                                                /* two ADDRESS bytes */
                                                state = rw ? READ_DATA : GET_WORD_ADR_HIGH;
                                                slave_mask <<= 16;
                                        }
                                        else
                                        {
                                                /* one ADDRESS byte */
                                                state = rw ? READ_DATA : GET_WORD_ADR_LOW;
                                                slave_mask <<= 8;
                                        }
                                }

                                cycles ++;
                        }
                        break;

                /* Get Word Address MSB (4-8bits depending on the array size)
                 * MODE-3 only (24C32 - 24C512)
                 */
                case GET_WORD_ADR_HIGH:

                        Detect_START();
                        Detect_STOP();

                        /* look for SCL HIGH to LOW transition */
                        if (old_scl && !scl)
                        {                               
                                if (cycles < 9)
                                {
                                        if ((type.size_mask + 1) < (1 << (17 - cycles)))
                                        {
                                                /* ignored bit: slave mask should be right-shifted by one  */
                                                slave_mask >>= 1;
                                        }
                                        else
                                        {
                                                /* this is a WORD ADDRESS high bit */
                                                if (old_sda) word_address |= (1 << (16 - cycles));
                                                else word_address &= ~(1 << (16 - cycles));
                                        }

                                        cycles ++;
                                }
                                else
                                {
                                        /* ACK CYCLE */
                                        cycles = 1;
                                        state = GET_WORD_ADR_LOW;
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
                        if (old_scl && !scl)
                        {
                                if (cycles < 9)
                                {
                                        if ((type.size_mask + 1) < (1 << (9 - cycles)))
                                        {
                                                /* ignored bit (X24C01): slave mask should be right-shifted by one  */
                                                slave_mask >>= 1;
                                        }
                                        else
                                        {
                                                /* this is a WORD ADDRESS high bit */
                                                if (old_sda) word_address |= (1 << (8 - cycles));
                                                else word_address &= ~(1 << (8 - cycles));
                                        }

                                        cycles ++;
                                }
                                else
                                {
                                        /* ACK CYCLE */
                                        cycles = 1;
                                        word_address &= type.size_mask;
                                        state = WRITE_DATA;
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
                        if (old_scl && !scl)
                        {
                                if (cycles < 9) cycles ++;
                                else
                                {
                                        cycles = 1;

                                        /* ACK not received */
                                        if (old_sda) state = WAIT_STOP;
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
                        if (old_scl && !scl)
                        {                               
                                if (cycles < 9)
                                {
                                        /* Write DATA bits (max 64kBytes) */
                                        sram_address = (slave_mask | word_address) & 0xFFFF;
                                        if (old_sda) sram[sram_address] |= (1 << (8 - cycles));
                                        else sram[sram_address] &= ~(1 << (8 - cycles));

                                        if (cycles == 8) 
                                        {
                                                /* WORD ADDRESS is incremented (roll up at maximum pagesize) */
                                                word_address = (word_address & (0xFFFF - type.pagewrite_mask)) | 
                                                                                          ((word_address + 1) & type.pagewrite_mask);
                                        }

                                        cycles ++;
                                }
                                else cycles = 1;        /* ACK cycle */
                        }
                        break;
        }

        old_scl = scl;
        old_sda = sda;
}

unsigned int MD_Cart_Type_EEPROM::ReadEEPROM(unsigned int address)
{
        uint16 sram_address;
        uint8 sda_out = sda;

        //printf("Read: %08x, %08x\n", address, type.sda_out_adr);

        /* EEPROM state */
        switch (state)
        {
                case READ_DATA:
                        if (cycles < 9)
                        {
                                /* Return DATA bits (max 64kBytes) */
                                sram_address = (slave_mask | word_address) & 0xFFFF;
                                sda_out = (sram[sram_address] >> (8 - cycles)) & 1;

                                if (cycles == 8)
                                {
                                        /* WORD ADDRESS is incremented (roll up at maximum array size) */
                                        word_address ++;
                                        word_address &= type.size_mask;
                                }
                        }
                        break;

                case GET_WORD_ADR_7BITS:
                case GET_SLAVE_ADR:
                case GET_WORD_ADR_HIGH:
                case GET_WORD_ADR_LOW:
                case WRITE_DATA:
                        if (cycles == 9) sda_out = 0;
                        break;

                default:
                        break;
        }

        if (address == type.sda_out_adr) return (sda_out << type.sda_out_bit);
        else return 0;
}


void MD_Cart_Type_EEPROM::Write8(uint32 A, uint8 V)
{
 if(A >= eeprom_start && A <= eeprom_end)
 {
  WriteEEPROM(A, V);
 }
}

void MD_Cart_Type_EEPROM::Write16(uint32 A, uint8 V)
{
 if(A >= eeprom_start && A <= eeprom_end)
 {
  WriteEEPROM(A, V);
 }
}

uint8 MD_Cart_Type_EEPROM::Read8(uint32 A)
{
 if(A >= eeprom_start && A <= eeprom_end)
 {
  return(ReadEEPROM(A));
 }

 if(A < 0x400000)
 {
  if(A > rom_size)
  {
   printf("Moo: %08x\n", A);
   return(0);
  }
  return(READ_BYTE_MSB(rom, A));
 }
 return(m68k_read_bus_8(A));
}

uint16 MD_Cart_Type_EEPROM::Read16(uint32 A)
{
 if(A >= eeprom_start && A <= eeprom_end)
 {
  return(ReadEEPROM(A));
 }

 if(A < 0x400000)
 {
  if(A > rom_size)
  {
   printf("Moo: %08x\n", A);
   return(0);
  }
  return(READ_WORD_MSB(rom, A));
 }

 return(m68k_read_bus_16(A));
}

int MD_Cart_Type_EEPROM::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(sda),
  SFVAR(scl),
  SFVAR(old_sda),
  SFVAR(old_scl),
  SFVAR(cycles),
  SFVAR(rw),
  SFVAR(slave_mask),
  SFVAR(word_address),
  SFVAR(state),
  SFARRAY(sram, type.size_mask + 1),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);
 if(load)
 {

 }
 return(ret);
}

uint32 MD_Cart_Type_EEPROM::GetNVMemorySize(void)
{
 return(type.size_mask + 1);
}

void MD_Cart_Type_EEPROM::ReadNVMemory(uint8 *buffer)
{
 memcpy(buffer, sram, type.size_mask + 1);
}

void MD_Cart_Type_EEPROM::WriteNVMemory(const uint8 *buffer)
{
 memcpy(sram, buffer, type.size_mask + 1);
}

MD_Cart_Type *MD_Make_Cart_Type_EEPROM(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const uint32 iparam, const char *sparam)
{
 return(new MD_Cart_Type_EEPROM(ginfo, ROM, ROM_size, iparam));
}
