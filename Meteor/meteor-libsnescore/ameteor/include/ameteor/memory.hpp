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

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "cartmem.hpp"
//XXX
#include "eeprom.hpp"

#include <stdint.h>
#include <string>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Memory
	{
		public :
			// in cycles
			static const uint32_t CART_SAVE_TIME = 16*1024*1024; // 1 second

			enum CartType
			{
				CTYPE_UNKNOWN,
				CTYPE_EEPROM512,
				CTYPE_EEPROM8192,
				CTYPE_FLASH64,
				CTYPE_FLASH128,
				CTYPE_SRAM
			};
			enum CartError
			{
				CERR_NO_ERROR,
				CERR_NOT_FOUND,
				CERR_FAIL
			};

			Memory ();
			~Memory ();

			uint8_t GetCartType () const
			{
				return m_carttype;
			}
			// erases cartridge memory
			void SetCartTypeFromSize (uint32_t size);
			void SetCartType (uint8_t type);
			void SetCartFile (const char* filename)
			{
				m_cartfile = filename;
			}

			void Reset (uint32_t params = ~0);
			void ClearWbram ();
			void ClearWcram ();
			void ClearPalette ();
			void ClearVram ();
			void ClearOam ();
			void SoftReset ();

			bool LoadBios (const char* filename);
			void UnloadBios ()
			{
				if (m_brom)
				{
					delete [] m_brom;
					m_brom = NULL;
				}
			}
			bool LoadRom (const char* filename);
			void LoadRom (const uint8_t* data, uint32_t size);
			CartError LoadCart ();
#ifdef __LIBSNES__
			bool LoadCartInferred ();
#endif

			bool HasBios () const
			{
				return m_brom;
			}

			uint8_t GetCycles16NoSeq (uint32_t add, uint32_t count);
			uint8_t GetCycles16Seq (uint32_t add, uint32_t count);
			uint8_t GetCycles32NoSeq (uint32_t add, uint32_t count);
			uint8_t GetCycles32Seq (uint32_t add, uint32_t count);
			void UpdateWaitStates (uint16_t waitcnt);

			uint8_t* GetRealAddress(uint32_t add, uint8_t size = 0);

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

			// TODO make const members
			uint8_t Read8 (uint32_t add);
			uint16_t Read16 (uint32_t add);
			uint32_t Read32 (uint32_t add);

			void Write8 (uint32_t add, uint8_t val);
			void Write16 (uint32_t add, uint16_t val);
			void Write32 (uint32_t add, uint32_t val);

			void WriteEepromDma (uint32_t src, uint16_t size);
			//void ReadEepromDma (uint32_t dest, uint16_t size);

			void TimeEvent ();

		private :
			// times for a 8 or 16 bits access
			uint8_t m_memtime[0xF];
			// times for a sequential 8 or 16 bits access in GamePak ROM
			uint8_t m_memtimeseq[0x3];

			// General Internal Memory
			uint8_t* m_brom;  // BIOS - System ROM
			uint8_t* m_wbram; // WRAM - On-board Work RAM
			uint8_t* m_wcram; // WRAM - In-chip Work RAM
			// Internal Display Memory
			uint8_t* m_pram;  // BG/OBJ Palette RAM
			uint8_t* m_vram;  // VRAM - Video RAM
			uint8_t* m_oram;  // OAM - OBJ Attributes
			// External Memory (Game Pak)
			uint8_t* m_rom;   // Game Pake ROM/FlashROM (max 32MB)

			uint8_t m_carttype;
			CartMem* m_cart;
			std::string m_cartfile;

			uint8_t ReadCart (uint16_t add);
			void WriteCart (uint16_t add, uint8_t val);
	};
}

#endif
