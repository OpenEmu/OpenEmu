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

#ifndef __FLASH_H__
#define __FLASH_H__

#include "cartmem.hpp"
#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Flash : public CartMem
	{
		public :
			Flash (bool big);

			void Reset ();

			bool Load (std::istream& f);
			bool Save (std::ostream& f);

			uint8_t Read (uint16_t add);
			bool Write (uint16_t add, uint8_t val);

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		private :
			uint8_t m_device_id;
			uint8_t m_manufacturer_id;

			enum State
			{
				NORMAL,
				CMD1,
				CMD2,
				ID,
				ERASE1,
				ERASE2,
				ERASE3,
				WRITE
			};

			State m_state;
	};
}

#endif
