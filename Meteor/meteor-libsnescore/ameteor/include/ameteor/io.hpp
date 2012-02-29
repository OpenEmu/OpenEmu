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

#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Io
	{
		public :
			enum IoAddress
			{
				// LCD I/O Registers
				DISPCNT     = 0x000,
				DISPSTAT    = 0x004,
				VCOUNT      = 0x006,
				BG0CNT      = 0x008,
				BG1CNT      = 0x00A,
				BG2CNT      = 0x00C,
				BG3CNT      = 0x00E,
				BG0HOFS     = 0x010,
				BG0VOFS     = 0x012,
				BG1HOFS     = 0x014,
				BG1VOFS     = 0x016,
				BG2HOFS     = 0x018,
				BG2VOFS     = 0x01A,
				BG3HOFS     = 0x01C,
				BG3VOFS     = 0x01E,
				BG2PA       = 0x020,
				BG2PB       = 0x022,
				BG2PC       = 0x024,
				BG2PD       = 0x026,
				BG2X_L      = 0x028,
				BG2X_H      = 0x02A,
				BG2Y_L      = 0x02C,
				BG2Y_H      = 0x02E,
				BG3PA       = 0x030,
				BG3PB       = 0x032,
				BG3PC       = 0x034,
				BG3PD       = 0x036,
				BG3X_L      = 0x038,
				BG3X_H      = 0x03A,
				BG3Y_L      = 0x03C,
				BG3Y_H      = 0x03E,
				WIN0H       = 0x040,
				WIN1H       = 0x042,
				WIN0V       = 0x044,
				WIN1V       = 0x046,
				WININ       = 0x048,
				WINOUT      = 0x04A,
				MOSAIC      = 0x04C,
				BLDCNT      = 0x050,
				BLDALPHA    = 0x052,
				BLDY        = 0x054,
				// Sound Registers
				SOUND1CNT_L = 0x060, NR10 = 0x060,
				SOUND1CNT_H = 0x062, NR11 = 0x062,
				                     NR12 = 0x063,
				SOUND1CNT_X = 0x064, NR13 = 0x064,
				                     NR14 = 0x065,
				SOUND2CNT_L = 0x068, NR21 = 0x068,
				                     NR22 = 0x069,
				SOUND2CNT_H = 0x06C, NR23 = 0x06C,
				                     NR24 = 0x06D,
				SOUND4CNT_L = 0x078, NR41 = 0x078,
				                     NR42 = 0x079,
				SOUND4CNT_H = 0x07C, NR43 = 0x07C,
				                     NR44 = 0x07D,
				SOUNDCNT_L  = 0x080, NR50 = 0x080,
				                     NR51 = 0x081,
				SOUNDCNT_H  = 0x082,
				SOUNDCNT_X  = 0x084, NR52 = 0x084,
				SOUNDBIAS   = 0x088,
				FIFO_A      = 0x0A0,
				FIFO_B      = 0x0A4,
				// DMA Transfer Channels
				DMA0SAD     = 0x0B0,
				DMA0DAD     = 0x0B4,
				DMA0CNT_L   = 0x0B8,
				DMA0CNT_H   = 0x0BA,
				DMA1SAD     = 0x0BC,
				DMA1DAD     = 0x0C0,
				DMA1CNT_L   = 0x0C4,
				DMA1CNT_H   = 0x0C6,
				DMA2SAD     = 0x0C8,
				DMA2DAD     = 0x0CC,
				DMA2CNT_L   = 0x0D0,
				DMA2CNT_H   = 0x0D2,
				DMA3SAD     = 0x0D4,
				DMA3DAD     = 0x0D8,
				DMA3CNT_L   = 0x0DC,
				DMA3CNT_H   = 0x0DE,
				// Timer Registers
				TM0CNT_L    = 0x100,
				TM0CNT_H    = 0x102,
				TM1CNT_L    = 0x104,
				TM1CNT_H    = 0x106,
				TM2CNT_L    = 0x108,
				TM2CNT_H    = 0x10A,
				TM3CNT_L    = 0x10C,
				TM3CNT_H    = 0x10E,
				// Keypad Input
				KEYINPUT    = 0x130,
				KEYCNT      = 0x132,
				// Serial Communication (2)
				RCNT        = 0x134,
				// Interrupt, WaitState, and Power-Down Control
				IE          = 0x200,
				IF          = 0x202,
				WAITCNT     = 0x204,
				IME         = 0x208,
				POSTFLG     = 0x300,
				HALTCNT     = 0x301,

				DMA_CHANSIZE = 0x00C,
				TIMER_SIZE   = 0x004,
				// TODO make tests and everything in Write*() functions so that we can
				// make IO_SIZE 0x804 (don't forget mirrors)
				IO_SIZE      = 0x1000
			};

			Io ();
			~Io ();

			void Reset ();
			void ClearSio ();
			void ClearSound ();
			void ClearOthers ();

			uint8_t Read8 (uint32_t add);
			uint16_t Read16 (uint32_t add);
			uint32_t Read32 (uint32_t add);

			void Write8 (uint32_t add, uint8_t val);
			void Write16 (uint32_t add, uint16_t val);
			void Write32 (uint32_t add, uint32_t val);

			// Direct read and write
			// Using theses functions will write directly on IO memory without
			// doing anything else (they won't call Dma::Check for example)
			// add must be the real address & 0xFFF
			// No check is done on the memory, these functions may segfault if
			// you give them wrong values !

			uint8_t DRead8 (uint16_t add)
			{
				return m_iomem[add];
			}

			uint16_t DRead16 (uint16_t add)
			{
				return *(uint16_t*)(m_iomem+add);
			}

			uint32_t DRead32 (uint16_t add)
			{
				return *(uint32_t*)(m_iomem+add);
			}

			void DWrite8 (uint16_t add, uint8_t val)
			{
				m_iomem[add] = val;
			}

			void DWrite16 (uint16_t add, uint16_t val)
			{
				*(uint16_t*)(m_iomem+add) = val;
			}

			void DWrite32 (uint16_t add, uint32_t val)
			{
				*(uint32_t*)(m_iomem+add) = val;
			}

			uint8_t& GetRef8 (uint16_t add)
			{
				return m_iomem[add];
			}

			uint16_t& GetRef16 (uint16_t add)
			{
				return *(uint16_t*)(m_iomem+add);
			}

			uint32_t& GetRef32 (uint16_t add)
			{
				return *(uint32_t*)(m_iomem+add);
			}

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		private :
			uint8_t* m_iomem;
	};
}

#endif
