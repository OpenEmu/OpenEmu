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

#ifndef __LCD_H__
#define __LCD_H__

#include "clock.hpp"

#include "memory.hpp"
#include "io.hpp"
#include "graphics/screen.hpp"

#include <stdint.h>
#include <istream>
#include <ostream>

#include "signal.hpp"

namespace AMeteor
{
	class Lcd
	{
		public :
			Lcd ();

			void Reset ();

			const uint16_t* GetSurface () const
			{
				return m_screen.GetSurface();
			}

			Graphics::Screen& GetScreen()
			{
				return m_screen;
			}
			const Graphics::Screen& GetScreen() const
			{
				return m_screen;
			}

			void SetFrameskip (uint8_t skip)
			{
				m_screen.SetFrameskip(skip);
			}

			void UpdateDispCnt (uint16_t dispcnt)
			{
				m_screen.UpdateDispCnt(dispcnt);
			}

#define UPDATE_BG_CNT(num) \
	void UpdateBg##num##Cnt (uint16_t cnt) \
	{ \
		m_screen.UpdateBg##num##Cnt(cnt); \
	}
			UPDATE_BG_CNT(0)
			UPDATE_BG_CNT(1)
			UPDATE_BG_CNT(2)
			UPDATE_BG_CNT(3)
#undef UPDATE_BG_CNT

#define UPDATE_BG_OFF(num, coord) \
	void UpdateBg##num##coord##Off (uint16_t cnt) \
	{ \
		m_screen.UpdateBg##num##coord##Off(cnt); \
	}
			UPDATE_BG_OFF(0, X)
			UPDATE_BG_OFF(0, Y)
			UPDATE_BG_OFF(1, X)
			UPDATE_BG_OFF(1, Y)
			UPDATE_BG_OFF(2, X)
			UPDATE_BG_OFF(2, Y)
			UPDATE_BG_OFF(3, X)
			UPDATE_BG_OFF(3, Y)
#undef UPDATE_BG_OFF

#define UPDATE_BG_REF(num, coord) \
	void UpdateBg##num##Ref##coord (int32_t cnt) \
	{ \
		m_screen.UpdateBg##num##Ref##coord (cnt); \
	}
			UPDATE_BG_REF(2, X)
			UPDATE_BG_REF(2, Y)
			UPDATE_BG_REF(3, X)
			UPDATE_BG_REF(3, Y)
#undef UPDATE_BG_REF

			void OamWrite (uint32_t begin, uint32_t end)
			{
				m_screen.OamWrite(begin, end);
			}
			void OamWrite16 (uint32_t add)
			{
				m_screen.OamWrite16(add);
			}
			void OamWrite32 (uint32_t add)
			{
				m_screen.OamWrite32(add);
			}

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

			syg::signal<void> sig_vblank;

		private :
			Graphics::Screen m_screen;

			void TimeEvent ();

			friend void Clock::Commit();
	};
}

#endif
