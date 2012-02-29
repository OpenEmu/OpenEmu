// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ameteor/memory.hpp"
#include "ameteor/io.hpp"
#include "ameteor/eeprom.hpp"
#include "ameteor/flash.hpp"
#include "ameteor/sram.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

#define RET_ADD(mem, low, high, size) \
	if (add >= low && (add+size) <= high) \
	{ \
		return mem + (add - low); \
	}

#define debugm debug

namespace AMeteor
{
	Memory::Memory () :
		m_brom(NULL),
		m_carttype(CTYPE_UNKNOWN),
		m_cart(NULL)
	{
		m_wbram = new uint8_t[0x00040000];
		m_wcram = new uint8_t[0x00008000];
		m_pram  = new uint8_t[0x00000400];
		m_vram  = new uint8_t[0x00018000];
		m_oram  = new uint8_t[0x00000400];
		m_rom   = new uint8_t[0x02000000];

		Reset();
	}

	Memory::~Memory ()
	{
		if (m_brom)
			delete [] m_brom;
		delete [] m_wbram;
		delete [] m_wcram;
		delete [] m_pram;
		delete [] m_vram;
		delete [] m_oram;
		delete [] m_rom;
		if (m_cart)
			delete m_cart;
	}

	void Memory::SetCartTypeFromSize (uint32_t size)
	{
		switch (size)
		{
			case 0x0200:
				SetCartType(CTYPE_EEPROM512);
				break;
			case 0x2000:
				SetCartType(CTYPE_EEPROM8192);
				break;
			case 0x8000:
				SetCartType(CTYPE_SRAM);
				break;
			case 0x10000:
				SetCartType(CTYPE_FLASH64);
				break;
			case 0x20000:
				SetCartType(CTYPE_FLASH128);
				break;
			default:
				met_abort("Unknown cartridge memory size");
				break;
		}
	}

	void Memory::SetCartType (uint8_t type)
	{
		if (m_cart)
			delete m_cart;
		switch (type)
		{
			case CTYPE_UNKNOWN:
				m_cart = NULL;
				break;
			case CTYPE_FLASH64:
				m_cart = new Flash(false);
				break;
			case CTYPE_FLASH128:
				m_cart = new Flash(true);
				break;
			case CTYPE_EEPROM512:
				m_cart = new Eeprom(false);
				break;
			case CTYPE_EEPROM8192:
				m_cart = new Eeprom(true);
				break;
			case CTYPE_SRAM:
				m_cart = new Sram();
				break;
			default:
				met_abort("Unknown cartridge memory type");
				break;
		}
		m_carttype = type;
	}

	void Memory::Reset (uint32_t params)
	{
		static const uint8_t InitMemoryTime[0xF] = {
			1, // 00 - BIOS
			0, // 01 - Not used
			3, // 02 - Work RAM 256K
			1, // 03 - Work RAM 32K
			1, // 04 - I/O Registers
			1, // 05 - Palette RAM
			1, // 06 - VRAM
			1, // 07 - OAM
			5, // 08 - ROM, Wait0
			5, // 09 - ROM, Wait0
			5, // 0A - ROM, Wait1
			5, // 0B - ROM, Wait1
			5, // 0C - ROM, Wait2
			5, // 0D - ROM, Wait2
			5  // 0E - SRAM
		};

		static const uint8_t InitMemoryTimeSeq[0x3] = {
			3, // 08-09 - ROM, Wait0
			5, // 0A-0B - ROM, Wait1
			9  // 0C-0D - ROM, Wait2
		};

		if (m_brom && (params & UNIT_MEMORY_BIOS))
		{
			delete [] m_brom;
			m_brom = NULL;
		}
		std::memcpy(m_memtime, InitMemoryTime, sizeof(m_memtime));
		std::memcpy(m_memtimeseq, InitMemoryTimeSeq, sizeof(m_memtimeseq));
		std::memset(m_wbram, 0, 0x00040000);
		std::memset(m_wcram, 0, 0x00008000);
		std::memset(m_pram , 0, 0x00000400);
		std::memset(m_vram , 0, 0x00018000);
		std::memset(m_oram , 0, 0x00000400);
		if (params & UNIT_MEMORY_ROM)
			std::memset(m_rom  , 0, 0x02000000);
		SetCartType(CTYPE_UNKNOWN);
		m_cartfile.clear();
	}

	void Memory::ClearWbram ()
	{
		std::memset(m_wbram, 0, 0x00040000);
	}

	void Memory::ClearWcram ()
	{
		std::memset(m_wcram, 0, 0x00008000);
	}

	void Memory::ClearPalette ()
	{
		std::memset(m_pram , 0, 0x00000400);
	}

	void Memory::ClearVram ()
	{
		std::memset(m_vram , 0, 0x00018000);
	}

	void Memory::ClearOam ()
	{
		std::memset(m_oram , 0, 0x00000400);
		LCD.OamWrite(0x07000000, 0x07000400);
	}

	void Memory::SoftReset ()
	{
		std::memset(m_wcram+0x7E00, 0, 0x200);
	}

	void Memory::TimeEvent ()
	{
		if (!m_cartfile.empty())
		{
			// FIXME, this may fail, we should do something to inform user
			std::ofstream f(m_cartfile.c_str());
			m_cart->Save(f);
		}
		CLOCK.DisableBattery();
	}

	bool Memory::LoadBios (const char* filename)
	{
		std::ifstream file(filename);
		if (!m_brom)
			m_brom = new uint8_t[0x00004000];
		memset(m_brom, 0, 0x00004000);
		file.read((char*)m_brom, 0x00004000);
		if (file.fail())
			return false;
		return true;
	}

	bool Memory::LoadRom (const char* filename)
	{
		std::ifstream file(filename);
		std::memset(m_rom, 0, 0x02000000);
		file.read((char*)m_rom, 0x02000000);
		if (file.bad())
			return false;
		return true;
	}

	void Memory::LoadRom (const uint8_t* data, uint32_t size)
	{
		uint32_t until = std::min(size, 0x02000000u);
		std::memcpy(m_rom, data, until);
		std::memset(m_rom+until, 0, 0x02000000-until);
	}

	Memory::CartError Memory::LoadCart ()
	{
		struct stat buf;
		if (stat(m_cartfile.c_str(), &buf) == -1)
			return errno == ENOENT ? CERR_NOT_FOUND : CERR_FAIL;
		SetCartTypeFromSize(buf.st_size);
		std::ifstream f(m_cartfile.c_str());
		if (!m_cart->Load(f))
			return CERR_FAIL;
		return CERR_NO_ERROR;
	}

#ifdef __LIBSNES__
	bool Memory::LoadCartInferred ()
	{
		uint32_t size = *(uint32_t*)(CartMemData+CartMem::MAX_SIZE);
		if (!size)
			return false;
		SetCartTypeFromSize(size);
		std::istringstream ss;
		ss.str(std::string((char*)CartMemData, CartMem::MAX_SIZE));
		if (!m_cart->Load(ss))
			return false;
		return true;
	}
#endif

#if 0
	uint8_t* Memory::GetRealAddress (uint32_t add, uint8_t size)
	{
		RET_ADD(m_brom , 0x00000000u, 0x00004000u, size);
		RET_ADD(m_wbram, 0x02000000u, 0x02040000u, size);
		RET_ADD(m_wbram, 0x02040000u, 0x02080000u, size); // mirror
		RET_ADD(m_wcram, 0x03000000u, 0x03008000u, size);
		RET_ADD(m_wcram, 0x03008000u, 0x03010000u, size); // mirror
		RET_ADD(m_wcram, 0x03010000u, 0x03018000u, size); // mirror
		RET_ADD(m_wcram, 0x03FF8000u, 0x04000000u, size); // mirror
		RET_ADD(m_pram , 0x05000000u, 0x05000400u, size);
		RET_ADD(m_vram , 0x06000000u, 0x06018000u, size);
		RET_ADD(m_oram , 0x07000000u, 0x07000400u, size);
		RET_ADD(m_oram , 0x07000400u, 0x07000800u, size);
		RET_ADD(m_rom  , 0x08000000u, 0x0A000000u, size);
		RET_ADD(m_rom  , 0x0A000000u, 0x0C000000u, size);
		RET_ADD(m_rom  , 0x0C000000u, 0x0E000000u, size);
		RET_ADD(m_sram , 0x0E000000u, 0x0E010000u, size);

		return NULL;
	}
#else
	uint8_t* Memory::GetRealAddress (uint32_t add, uint8_t size __attribute__((unused)))
	{
		uint32_t loadd = add & 0x00FFFFFF;
		switch (add >> 24)
		{
			case 0x0:
				if (m_brom)
					return m_brom+(loadd & 0x3FFF);
				else
					return NULL;
			case 0x2:
				return m_wbram+(loadd & 0x3FFFF);
			case 0x3:
				return m_wcram+(loadd & 0x7FFF);
			case 0x5:
				return m_pram+(loadd & 0x3FF);
			case 0x6:
				// we have 64K+32K+(32K mirror) and this whole thing is mirrored
				// TODO : can't we simplify this with an AND ?
				if ((loadd % 0x20000) > 0x18000)
					loadd -= 0x8000;
				return m_vram+(loadd & 0x1FFFF);
			case 0x7:
				return m_oram+(loadd & 0x3FF);
			case 0x8:
				return m_rom+loadd;
			case 0x9:
				return m_rom+0x01000000+loadd;
			case 0xA:
				return m_rom+loadd;
			case 0xB:
				return m_rom+0x01000000+loadd;
			case 0xC:
				return m_rom+loadd;
			case 0xD:
				return m_rom+0x01000000+loadd;
			default:
				return NULL;
		}
	}
#endif

	void Memory::UpdateWaitStates (uint16_t waitcnt)
	{
		static const uint8_t GamePakTimeFirstAccess[] = { 5, 4, 3, 9 };

		// SRAM
		m_memtime[0xE] = GamePakTimeFirstAccess[waitcnt & 0x3];

		// Second access
		if (waitcnt & (0x1 << 4))
			m_memtimeseq[0] = 2;
		else
			m_memtimeseq[0] = 3;
		if (waitcnt & (0x1 << 7))
			m_memtimeseq[1] = 2;
		else
			m_memtimeseq[1] = 5;
		if (waitcnt & (0x1 << 10))
			m_memtimeseq[2] = 2;
		else
			m_memtimeseq[2] = 9;

		// First access
		m_memtime[0x8] = m_memtime[0x9] =
			GamePakTimeFirstAccess[(waitcnt >> 2) & 0x3];
		m_memtime[0xA] = m_memtime[0xB] =
			GamePakTimeFirstAccess[(waitcnt >> 5) & 0x3];
		m_memtime[0xC] = m_memtime[0xD] =
			GamePakTimeFirstAccess[(waitcnt >> 8) & 0x3];
	}

	uint8_t Memory::GetCycles16NoSeq (uint32_t add, uint32_t count)
	{
		add >>= 24;
		switch (add)
		{
			case 0x00 :
			case 0x03 :
			case 0x04 :
			case 0x07 :
				// 32 bits bus
				return m_memtime[add] * count;
			case 0x08 :
			case 0x09 :
			case 0x0A :
			case 0x0B :
			case 0x0C :
			case 0x0D :
				// 16 bits bus
				// sequencial and non sequencial reads don't take the same time
				return m_memtime[add] + m_memtimeseq[(add-0x08) & 0xFE] * (count-1);
			default :
				// 16 bits bus (and 8 for SRAM, but only 8 bits accesses are
				// authorized in this area, so we don't care about 16 and 32 bits
				// accesses)
				return m_memtime[add] * count;
		}
	}

	uint8_t Memory::GetCycles16Seq (uint32_t add, uint32_t count)
	{
		add >>= 24;
		switch (add)
		{
			case 0x00 :
			case 0x03 :
			case 0x04 :
			case 0x07 :
				return m_memtime[add] * count;
			case 0x08 :
			case 0x09 :
			case 0x0A :
			case 0x0B :
			case 0x0C :
			case 0x0D :
				return m_memtimeseq[(add-0x08) & 0xFE] * count;
			default :
				return m_memtime[add] * count;
		}
	}

	uint8_t Memory::GetCycles32NoSeq (uint32_t add, uint32_t count)
	{
		add >>= 24;
		switch (add)
		{
			case 0x00 :
			case 0x03 :
			case 0x04 :
			case 0x07 :
				return m_memtime[add] * count;
			case 0x08 :
			case 0x09 :
			case 0x0A :
			case 0x0B :
			case 0x0C :
			case 0x0D :
				return m_memtime[add] + m_memtimeseq[(add-0x08) & 0xFE] * (count*2-1);
			default :
				return m_memtime[add] * count * 2;
		}
	}

	uint8_t Memory::GetCycles32Seq (uint32_t add, uint32_t count)
	{
		add >>= 24;
		switch (add)
		{
			case 0x00 :
			case 0x03 :
			case 0x04 :
			case 0x07 :
				return m_memtime[add] * count;
			case 0x08 :
			case 0x09 :
			case 0x0A :
			case 0x0B :
			case 0x0C :
			case 0x0D :
				return m_memtimeseq[(add-0x08) & 0xFE] * count * 2;
			default :
				return m_memtime[add] * count * 2;
		}
	}

	bool Memory::SaveState (std::ostream& stream)
	{
		SS_WRITE_ARRAY(m_memtime);
		SS_WRITE_ARRAY(m_memtimeseq);
		// write if we have a custom bios and write it too
		bool b = m_brom;
		SS_WRITE_VAR(b);
		if (b)
			SS_WRITE_DATA(m_brom, 0x00004000);
		SS_WRITE_DATA(m_wbram, 0x00040000);
		SS_WRITE_DATA(m_wcram, 0x00008000);
		SS_WRITE_DATA(m_pram , 0x00000400);
		SS_WRITE_DATA(m_vram , 0x00018000);
		SS_WRITE_DATA(m_oram , 0x00000400);

		SS_WRITE_VAR(m_carttype);

		if (m_cart)
			if (!m_cart->SaveState(stream))
				return false;

		return true;
	}

	bool Memory::LoadState (std::istream& stream)
	{
		Reset(0);

		SS_READ_ARRAY(m_memtime);
		SS_READ_ARRAY(m_memtimeseq);
		// read if we have a custom bios and write it too
		bool b;
		SS_READ_VAR(b);
		if (b)
			SS_READ_DATA(m_brom , 0x00004000);
		else
			UnloadBios();
		SS_READ_DATA(m_wbram, 0x00040000);
		SS_READ_DATA(m_wcram, 0x00008000);
		SS_READ_DATA(m_pram , 0x00000400);
		SS_READ_DATA(m_vram , 0x00018000);
		SS_READ_DATA(m_oram , 0x00000400);

		SS_READ_VAR(m_carttype);

		this->SetCartType(m_carttype);
		if (m_cart)
			if (!m_cart->LoadState(stream))
				return false;

		return true;
	}

////////////////////////////////////////////////////////////////////////////////
// Read
////////////////////////////////////////////////////////////////////////////////

	uint8_t Memory::Read8 (uint32_t add)
	{
		switch (add >> 24)
		{
			case 0x00:
				if (R(15) < 0x01000000)
					return m_brom[add & 0x3FFF];
				else
					return 0x0E;
			case 0x04:
				return IO.Read8(add);
			case 0x0E:
				return ReadCart(add);
			default:
				uint8_t *r = (uint8_t*)GetRealAddress(add, 1);
				if (!r)
				{
					debugm("Unknown address for Read8 : " << IOS_ADD << add);
					// FIXME : in arm state, vba returns read8(r15 + (add & 3))
					// and in thumb read8(r15 + (add & 1))
					return Read8(R(15));
				}
				return *r;
		}
	}

	uint16_t Memory::Read16 (uint32_t add)
	{
		switch (add >> 24)
		{
			case 0x00:
				if (R(15) < 0x01000000)
					return *(uint16_t*)(m_brom+(add & 0x3FFE));
				else
					return 0xF00E;
			case 0x04:
				return IO.Read16(add);
			case 0x0D:
				if (m_carttype == CTYPE_EEPROM512 || m_carttype == CTYPE_EEPROM8192)
					return static_cast<Eeprom*>(m_cart)->Read();
			default:
				uint16_t *r = (uint16_t*)GetRealAddress(add, 2);
				if (!r)
				{
					debugm("Unknown address for Read16 : " << IOS_ADD << add);
					if (R(15) == add)
						met_abort("Illegal PC");
					// FIXME : in arm state, vba returns read16(r15 + (add & 2))
					return Read16(R(15));
				}
				return *r;
		}
	}

	uint32_t Memory::Read32 (uint32_t add)
	{
		switch (add >> 24)
		{
			case 0x00:
				if (R(15) < 0x01000000)
					return *(uint32_t*)(m_brom+(add & 0x3FFC));
				else
					// TODO a better bios protection than this one (read8 and read16 too)
					// this value corresponds to MOVS r15, r14
					return 0xE1B0F00E;
			case 0x04:
				return IO.Read32(add);
			default:
				uint32_t *r = (uint32_t*)GetRealAddress(add, 4);
				if (!r)
				{
					debugm("Unknown address for Read32 : " << IOS_ADD << add);
					if (R(15) == add)
						met_abort("Illegal PC");
					if (FLAG_T)
					{
						uint16_t o = Read16(R(15));
						return o | o << 16;
					}
					else
						return Read32(R(15));
				}
				return *r;
		}
	}

	uint8_t Memory::ReadCart (uint16_t add)
	{
		if (m_cart)
			return m_cart->Read(add);
		else
			return 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Write
////////////////////////////////////////////////////////////////////////////////

	void Memory::Write8 (uint32_t add, uint8_t val)
	{
		uint8_t baseadd = add >> 24;
		switch (baseadd)
		{
			case 0x04:
				IO.Write8(add, val);
				break;
			case 0x00:
			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B:
			case 0x0C:
			case 0x0D:
				debugm("Writing on read only memory");
				break;
			case 0x0E:
				WriteCart(add & 0xFFFF, val);
				break;
			default:
				uint8_t *r = (uint8_t*)GetRealAddress(add, 1);
				if (!r)
				{
					debugm("Unknown address for Write8 : " << IOS_ADD << add);
				}
				else
				{
					*r = val;
					if (baseadd == 5 || baseadd == 6)
						r[1] = val;
					else if (baseadd == 7)
						met_abort("not implemented");
				}
				break;
		}
	}

	void Memory::Write16 (uint32_t add, uint16_t val)
	{
		add &= 0xFFFFFFFE;
		uint8_t baseadd = add >> 24;
		switch (baseadd)
		{
			case 0x04:
				IO.Write16(add, val);
				break;
			case 0x00:
			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B:
			case 0x0C:
			case 0x0D:
				debugm("Writing on read only memory");
				break;
			case 0x0E:
				met_abort("Writing 16 bytes in SRAM/Flash");
				break;
			default:
				uint16_t *r = (uint16_t*)GetRealAddress(add, 2);
				if (!r)
				{
					debugm("Unknown address for Write16 : " << IOS_ADD << add);
				}
				else
				{
					*r = val;
					if (!DMA.GraphicDma() && baseadd == 7)
						LCD.OamWrite16((add & 0x3FF) | 0x07000000);
				}
				break;
		}
	}

	void Memory::Write32 (uint32_t add, uint32_t val)
	{
		add &= 0xFFFFFFFC;
		uint8_t baseadd = add >> 24;
		switch (baseadd)
		{
			case 0x04:
				IO.Write32(add, val);
				break;
			case 0x00:
			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B:
			case 0x0C:
			case 0x0D:
				debugm("Writing on read only memory");
				break;
			case 0x0E:
				met_abort("Writing 32 bytes in SRAM/Flash");
				break;
			default:
				uint32_t *r = (uint32_t*)GetRealAddress(add, 4);
				if (!r)
				{
					debugm("Unknown address for Write32 : " << IOS_ADD << add);
				}
				else
				{
					*r = val;
					if (!DMA.GraphicDma() && baseadd == 7)
						LCD.OamWrite32((add & 0x3FF) | 0x07000000);
				}
				break;
		}
	}

	void Memory::WriteEepromDma (uint32_t src, uint16_t size)
	{
		if (m_carttype == CTYPE_UNKNOWN)
		{
			if (size == 17 || size == 81)
				SetCartType(CTYPE_EEPROM8192);
			else if (size == 9 || size == 73)
				SetCartType(CTYPE_EEPROM512);
			else
				met_abort("Unknown DMA3 size for EEPROM");
		}
		else if (m_carttype != CTYPE_EEPROM512 && m_carttype != CTYPE_EEPROM8192)
			met_abort("EEPROM DMA3 on non EEPROM cartridge");

		Eeprom* eeprom = static_cast<Eeprom*>(m_cart);
		if (size == 17 || size == 81)
		{
			if (eeprom->GetSize() != 0x2000)
				met_abort("DMA3 size not corresponding to EEPROM size");
		}
		else if (size ==  9 || size == 73)
		{
			if (eeprom->GetSize() != 0x0200)
				met_abort("DMA3 size not corresponding to EEPROM size");
		}
		else
			met_abort("Unknown size for EEPROM DMA");

		if (eeprom->Write((uint16_t*)GetRealAddress(src), size))
			CLOCK.SetBattery(CART_SAVE_TIME);
	}

#if 0
	void Memory::ReadEepromDma (uint32_t dest, uint16_t size)
	{
		if (m_carttype != CTYPE_EEPROM)
			met_abort("EEPROM DMA3 on non EEPROM or unknown cartridge");
		if (size != 68)
			met_abort("EEPROM DMA3 read with invalid size");
		Eeprom* eeprom = static_cast<Eeprom*>(m_cart);
		eeprom->Read((uint16_t*)GetRealAddress(dest));
	}
#endif

#ifdef NO_MEMMEM // memmem() is a GNU extension, and does not exist in at least MinGW.
#define memmem memmem_compat
	// Implementation from Git.
	static void *memmem_compat(const void *haystack, size_t haystack_len,
			const void *needle, size_t needle_len)
	{
		const char *begin = (const char*)haystack;
		const char *last_possible = begin + haystack_len - needle_len;

		if (needle_len == 0)
			return (void *)begin;

		if (haystack_len < needle_len)
			return NULL;

		for (; begin <= last_possible; begin++)
		{
			if (!memcmp(begin, needle, needle_len))
				return (void *)begin;
		}

		return NULL;
	}
#endif

	void Memory::WriteCart (uint16_t add, uint8_t val)
	{
		if (m_carttype == CTYPE_EEPROM512 || m_carttype == CTYPE_EEPROM8192)
			met_abort("Writing in SRAM/FLASH while using EEPROM");
		if (!m_cart)
			if (add == 0x5555)
			{
				if (memmem((char*)m_rom, 0x02000000, "FLASH1M_V", 9))
					SetCartType(CTYPE_FLASH128);
				else
					SetCartType(CTYPE_FLASH64);
			}
			else
				SetCartType(CTYPE_SRAM);
		if (m_cart->Write(add, val))
			CLOCK.SetBattery(CART_SAVE_TIME);
	}
}
