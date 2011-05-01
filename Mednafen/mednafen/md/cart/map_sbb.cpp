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

class MD_Cart_Type_SBB : public MD_Cart_Type
{
	public:

        MD_Cart_Type_SBB(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size);
        virtual ~MD_Cart_Type_SBB();

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
	
};


MD_Cart_Type_SBB::MD_Cart_Type_SBB(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size)
{
 this->rom = ROM;
 this->rom_size = ROM_size;
}

MD_Cart_Type_SBB::~MD_Cart_Type_SBB()
{

}


void MD_Cart_Type_SBB::Write8(uint32 A, uint8 V)
{

}

void MD_Cart_Type_SBB::Write16(uint32 A, uint8 V)
{

}

uint8 MD_Cart_Type_SBB::Read8(uint32 A)
{
 if(A < 0x400000)
 {
  if(A > rom_size)
  {
   printf("Moo: %08x\n", A);
   return(0);
  }
  return(READ_BYTE_MSB(rom, A));
 }

 if(A == 0x400000)
  return(0x55);
 if(A == 0x400002)
  return(0x0f);

 printf("Moo: %08x\n", A);
 return(m68k_read_bus_8(A));
}

uint16 MD_Cart_Type_SBB::Read16(uint32 A)
{
 if(A < 0x400000)
 {
  if(A > rom_size)
  {
   printf("Moo: %08x\n", A);
   return(0);
  }
  return(READ_WORD_MSB(rom, A));
 }

 if(A == 0x400000)
  return(0x55);
 if(A == 0x400002)
  return(0x0f);

 printf("Moo: %08x\n", A);
 return(m68k_read_bus_16(A));
}

int MD_Cart_Type_SBB::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 return(1);
}

uint32 MD_Cart_Type_SBB::GetNVMemorySize(void)
{
 return(0);
}

void MD_Cart_Type_SBB::ReadNVMemory(uint8 *buffer)
{

}

void MD_Cart_Type_SBB::WriteNVMemory(const uint8 *buffer)
{

}

MD_Cart_Type *MD_Make_Cart_Type_SBB(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const uint32 iparam, const char *sparam)
{
 return(new MD_Cart_Type_SBB(ginfo, ROM, ROM_size));
}
