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

#ifndef __SRAM_H__
#define __SRAM_H__

#include "cartmem.hpp"
#include <stdint.h>
#include <fstream>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Sram : public CartMem
	{
		public :
			Sram ();

			void Reset ();

			bool Load (std::istream& f);
			bool Save (std::ostream& f);

			uint8_t Read (uint16_t add)
			{
				return m_data[add % SIZE];
			}
			bool Write (uint16_t add, uint8_t val)
			{
				m_data[add % SIZE] = val;
				return true;
			}

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		private :
			static const uint16_t SIZE = 0x8000;
	};
}

#endif
