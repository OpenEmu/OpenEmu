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
#include "map_sram.h"

// TODO:  Add suport for 8-bit mapped into 16-bit space correctly

class MD_Cart_Type_SRAM : public MD_Cart_Type
{
	public:

        MD_Cart_Type_SRAM(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size);
        virtual ~MD_Cart_Type_SRAM();
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
	
	uint32 sram_start;
	uint32 sram_end;
	uint32 sram_size;

	bool sram_enabled;
	uint8 *sram;
};


MD_Cart_Type_SRAM::MD_Cart_Type_SRAM(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size)
{
 this->rom = ROM;
 this->rom_size = ROM_size;

 sram_start = ginfo->sram_start;
 sram_end = ginfo->sram_end;

 sram_size = 0;

 sram_start &= 0xFFFFFE;
 sram_end |= 0x000001;

 if((uint32)(sram_end - sram_start) >= 0x10000 || sram_start == sram_end)
 {
  sram_end = sram_start + 0xFFFF;
 }
 sram_size = sram_end - sram_start + 1;

 printf("%08x %08x %08x\n", sram_start, sram_end, sram_size);

 if(!(sram = (uint8 *)MDFN_malloc(sram_size, _("Cart SRAM"))))
 {
  // FIXME
  throw(-1);
 }
 memset(sram, 0xFF, sram_size);
}

MD_Cart_Type_SRAM::~MD_Cart_Type_SRAM()
{
 if(sram)
  free(sram);
}

void MD_Cart_Type_SRAM::Reset(void)
{
 if(rom_size > sram_start)
  sram_enabled = FALSE;
 else
  sram_enabled = TRUE;
}

void MD_Cart_Type_SRAM::Write8(uint32 A, uint8 V)
{
 if(A == 0xA130F1)
  sram_enabled = V & 1;

 if(A >= sram_start && A <= sram_end)
 {
  WRITE_BYTE_MSB(sram, A - sram_start, V);
 }
}

void MD_Cart_Type_SRAM::Write16(uint32 A, uint8 V)
{
 if(A == 0xA130F0)
  sram_enabled = V & 1;

 if(A >= sram_start && A <= sram_end)
 {
  WRITE_WORD_MSB(sram, A - sram_start, V);
 }
}

uint8 MD_Cart_Type_SRAM::Read8(uint32 A)
{
 if(sram_enabled && A >= sram_start && A <= sram_end)
 {
  return(READ_BYTE_MSB(sram, A - sram_start));
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

uint16 MD_Cart_Type_SRAM::Read16(uint32 A)
{
 if(sram_enabled && A >= sram_start && A <= sram_end)
 {
  return(READ_WORD_MSB(sram, A - sram_start));
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

int MD_Cart_Type_SRAM::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(sram_enabled),
  SFARRAY(sram, sram_size),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);
 if(load)
 {

 }
 return(ret);
}

uint32 MD_Cart_Type_SRAM::GetNVMemorySize(void)
{
 return(sram_size);
}

void MD_Cart_Type_SRAM::ReadNVMemory(uint8 *buffer)
{
 memcpy(buffer, sram, sram_size);
}

void MD_Cart_Type_SRAM::WriteNVMemory(const uint8 *buffer)
{
 memcpy(sram, buffer, sram_size);
}

MD_Cart_Type *MD_Make_Cart_Type_SRAM(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const uint32 iparam, const char *sparam)
{
 return(new MD_Cart_Type_SRAM(ginfo, ROM, ROM_size));
}
