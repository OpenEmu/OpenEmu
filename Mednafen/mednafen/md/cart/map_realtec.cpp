/* Mednafen - Multi-system Emulator
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

#include "../shared.h"
#include "cart.h"
#include "map_rom.h"

class MD_Cart_Type_REALTEC : public MD_Cart_Type
{
	public:

        MD_Cart_Type_REALTEC(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size);
        virtual ~MD_Cart_Type_REALTEC();
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

	const uint8 *rom;
	uint32 rom_size;

	bool boot_mode;
	uint8 bank_size;
	uint8 bank_select;
};


MD_Cart_Type_REALTEC::MD_Cart_Type_REALTEC(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size)
{
 this->rom = ROM;
 this->rom_size = ROM_size;
}

MD_Cart_Type_REALTEC::~MD_Cart_Type_REALTEC()
{

}

void MD_Cart_Type_REALTEC::Reset(void)
{
 boot_mode = TRUE;
 bank_select = 0;
 bank_size = 0;
}

void MD_Cart_Type_REALTEC::Write8(uint32 A, uint8 V)
{
 //printf("Write8: %08x %02x\n", A, V);

 switch(A & 0xC06000)
 {
  case 0x400000: boot_mode = !(V & 0x1);
                 bank_select &= ~0xC;
                 bank_select |= (V & 0x6) << 2;
                 break;

  case 0x402000: bank_size = V;
		 break;

  case 0x404000: bank_select &= ~0x7;
                 bank_select |= V & 0x7;
                 break;
 }
}

void MD_Cart_Type_REALTEC::Write16(uint32 A, uint8 V)
{
 //printf("Write16: %08x %04x\n", A, V);
 switch(A & 0xC06000)
 {
  case 0x400000: boot_mode = !(V & 0x1);
                 bank_select &= ~0xC;
                 bank_select |= (V & 0x6) << 2;
                 break;

  case 0x402000: bank_size = V;
                 break;

  case 0x404000: bank_select &= ~0x7;
                 bank_select |= V & 0x7;
                 break;
 }
}

uint8 MD_Cart_Type_REALTEC::Read8(uint32 A)
{
 if(A < 0x400000)
 {
  uint32 rom_offset;

  if(boot_mode)
   rom_offset = (A & 0x1FFF) | 0x7E000; //(rom_size &~ 0x1FFF) - 0x2000;
  else
  {
   rom_offset = (A & ((0x20000 * bank_size) - 1)) | (bank_select * 0x20000);
  }

  if(rom_offset >= rom_size)
   return(0);

  return(READ_BYTE_MSB(rom, rom_offset));
 }
 return(m68k_read_bus_8(A));
}

uint16 MD_Cart_Type_REALTEC::Read16(uint32 A)
{
 if(A < 0x400000)
 {
  uint32 rom_offset;

  if(boot_mode)
   rom_offset = (A & 0x1FFF) | 0x07E000; //rom_size &~ 0x1FFF) - 0x2000;
  else
  {
   rom_offset = (A & ((0x20000 * bank_size) - 1)) | (bank_select * 0x20000);
  }

  if(rom_offset >= rom_size)
   return(0);

  return(READ_WORD_MSB(rom, rom_offset));
 }

 return(m68k_read_bus_16(A));
}

int MD_Cart_Type_REALTEC::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(bank_size),
  SFVAR(bank_select),
  SFVAR(boot_mode),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);
 if(load)
 {

 }
 return(ret);

 return(1);
}

uint32 MD_Cart_Type_REALTEC::GetNVMemorySize(void)
{
 return(0);
}

void MD_Cart_Type_REALTEC::ReadNVMemory(uint8 *buffer)
{

}

void MD_Cart_Type_REALTEC::WriteNVMemory(const uint8 *buffer)
{

}

MD_Cart_Type *MD_Make_Cart_Type_REALTEC(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const uint32 iparam, const char *sparam)
{
 return(new MD_Cart_Type_REALTEC(ginfo, ROM, ROM_size));
}
