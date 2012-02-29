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

#ifndef __DMA_H__
#define __DMA_H__

#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Dma
	{
		public :
			enum Reason
			{
				Immediately = 0,
				VBlank,
				HBlank,
				Special
			};

			Dma () :
				m_graphic(false)
			{ }

			void Reset ();

			bool GraphicDma () const
			{
				return m_graphic;
			}

			void SetReload(uint8_t channum, uint16_t reload)
			{
				m_chans[channum].reload = reload;
			}

			void UpdateCnt (uint8_t channum);
			void Check(uint8_t channum, uint8_t reason);
			inline void CheckAll(uint8_t reason)
			{
				Check(0, reason);
				Check(1, reason);
				Check(2, reason);
				Check(3, reason);
			}

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		private :
			struct Channel
			{
				Channel () :
					reload(0),
					src(0),
					dest(0),
					count(0),
					control(0)
				{ }

				uint16_t reload;
				uint32_t src;
				uint32_t dest;
				uint16_t count;
				union Control
				{
					Control(uint16_t v) :
						w(v)
					{ }

					uint16_t w;
					struct
					{
						unsigned int unused : 5;
						unsigned int dest   : 2;
						unsigned int src    : 2;
						unsigned int repeat : 1;
						unsigned int type   : 1;
						unsigned int drq    : 1;
						unsigned int start  : 2;
						unsigned int irq    : 1;
						unsigned int enable : 1;
					} b;
				} control;
			};

			Channel m_chans[4];
			bool m_graphic;

			void Process(uint8_t channel);
			void Copy (uint32_t& src, uint32_t& dest, int8_t s_inc, int8_t d_inc,
					uint32_t count, bool word);
	};
}

#endif
