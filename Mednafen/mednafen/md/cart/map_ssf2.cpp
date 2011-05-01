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

class MD_Cart_Type_SSF2 : public MD_Cart_Type
{
	public:

        MD_Cart_Type_SSF2(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size);
        virtual ~MD_Cart_Type_SSF2();
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
	uint8 sf2_banks[8];
	uint8 control;
};


MD_Cart_Type_SSF2::MD_Cart_Type_SSF2(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size)
{
 this->rom = ROM;
 this->rom_size = ROM_size;
}

MD_Cart_Type_SSF2::~MD_Cart_Type_SSF2()
{

}

void MD_Cart_Type_SSF2::Reset(void)
{
 control = 0;

 for(int i = 0; i < 8; i++)
  sf2_banks[i] = i;
}

void MD_Cart_Type_SSF2::Write8(uint32 A, uint8 V)
{
 switch(A)
 {
  default: printf("Write8: %08x %02x\n", A, V); break;
  case 0xA130F1: control = V & 0x3; break;
  case 0xA130F3: sf2_banks[1] = V & 0x3F; break;
  case 0xA130F5: sf2_banks[2] = V & 0x3F; break;
  case 0xA130F7: sf2_banks[3] = V & 0x3F; break;
  case 0xA130F9: sf2_banks[4] = V & 0x3F; break;
  case 0xA130FB: sf2_banks[5] = V & 0x3F; break;
  case 0xA130FD: sf2_banks[6] = V & 0x3F; break;
  case 0xA130FF: sf2_banks[7] = V & 0x3F; break;
 }
}

void MD_Cart_Type_SSF2::Write16(uint32 A, uint8 V)
{
 // Just a guess
 switch(A)
 {
  default: printf("Write16: %08x %04x\n", A, V); break;
  case 0xA130F0: control = V & 0x3; break;
  case 0xA130F2: sf2_banks[1] = V & 0x3F; break;
  case 0xA130F4: sf2_banks[2] = V & 0x3F; break;
  case 0xA130F6: sf2_banks[3] = V & 0x3F; break;
  case 0xA130F8: sf2_banks[4] = V & 0x3F; break;
  case 0xA130FA: sf2_banks[5] = V & 0x3F; break;
  case 0xA130FC: sf2_banks[6] = V & 0x3F; break;
  case 0xA130FE: sf2_banks[7] = V & 0x3F; break;
 }
}

uint8 MD_Cart_Type_SSF2::Read8(uint32 A)
{
 if(A < 0x400000)
 {
  uint32 rom_offset = (sf2_banks[(A >> 19) & 0x7] << 19) | (A & 0x7FFFF);

  if(rom_offset >= rom_size)
   return(0);

  return(READ_BYTE_MSB(rom, rom_offset));
 }
 return(m68k_read_bus_8(A));
}

uint16 MD_Cart_Type_SSF2::Read16(uint32 A)
{
 if(A < 0x400000)
 {
  uint32 rom_offset = (sf2_banks[(A >> 19) & 0x7] << 19) | (A & 0x7FFFF);

  if(rom_offset >= rom_size)
   return(0);

  return(READ_WORD_MSB(rom, rom_offset));
 }

 return(m68k_read_bus_16(A));
}

int MD_Cart_Type_SSF2::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(sf2_banks, 8),
  SFVAR(control),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);
 if(load)
 {

 }
 return(ret);

 return(1);
}

uint32 MD_Cart_Type_SSF2::GetNVMemorySize(void)
{
 return(0);
}

void MD_Cart_Type_SSF2::ReadNVMemory(uint8 *buffer)
{

}

void MD_Cart_Type_SSF2::WriteNVMemory(const uint8 *buffer)
{

}

MD_Cart_Type *MD_Make_Cart_Type_SSF2(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const uint32 iparam, const char *sparam)
{
 return(new MD_Cart_Type_SSF2(ginfo, ROM, ROM_size));
}
