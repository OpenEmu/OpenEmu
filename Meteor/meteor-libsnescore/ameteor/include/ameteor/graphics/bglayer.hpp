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

#ifndef __GRAPHICS_BG_LAYER_H__
#define __GRAPHICS_BG_LAYER_H__

#include "memory.hpp"
#include "io.hpp"

#include <vector>
#include <stdint.h>

namespace AMeteor
{
	namespace Graphics
	{
		class BgLayer
		{
			public :
				BgLayer (int8_t num, Memory& memory, Io& io, uint16_t* pPalette);
				~BgLayer ();

				inline uint8_t GetNum () const;
				inline uint8_t GetPriority () const;

				void DrawLine0 (uint8_t line, uint16_t* ptr);
				void DrawLine2 (uint16_t* ptr,
						int32_t refX, int32_t refY,
						int16_t dx, int16_t dy);
				void DrawLine3 (uint16_t* ptr,
						int32_t refX, int32_t refY,
						int16_t dx, int16_t dy);
				void DrawLine4 (uint8_t line, uint16_t* ptr,
						int32_t curX, int32_t curY,
						int16_t dx, int16_t dmx, int16_t dy, int16_t dmy, bool frame1);
				void FillList ();

				void UpdateCnt (uint16_t cnt);
				inline void UpdateXOff (uint16_t off);
				inline void UpdateYOff (uint16_t off);

			private :
				Memory& m_memory;
				Io& m_io;

				const uint8_t m_num;
				uint8_t m_priority;

				uint16_t m_cnt;
				bool m_hicolor;
				uint16_t m_xoff, m_yoff;
				// in text mode
				uint8_t m_tWidth, m_tHeight;
				// in rotation/scale mode
				uint8_t m_rWidth, m_rHeight;

				uint32_t m_mapAdd;
				uint32_t m_charAdd;
				uint16_t* m_pPalette;
		};

		inline uint8_t BgLayer::GetNum () const
		{
			return m_num;
		}

		inline uint8_t BgLayer::GetPriority () const
		{
			return m_priority;
		}

		inline void BgLayer::UpdateXOff (uint16_t off)
		{
			m_xoff = off;
		}

		inline void BgLayer::UpdateYOff (uint16_t off)
		{
			m_yoff = off;
		}
	}
}

#endif
