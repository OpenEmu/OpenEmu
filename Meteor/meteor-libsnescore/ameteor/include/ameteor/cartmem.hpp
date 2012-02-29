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

#ifndef __CART_MEM_H__
#define __CART_MEM_H__

#include <fstream>
#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class CartMem
	{
		public:
			static const unsigned int MAX_SIZE = 0x20000;

			CartMem();
			virtual ~CartMem();

			virtual void Reset () = 0;

			virtual bool Load (std::istream& stream) = 0;
			virtual bool Save (std::ostream& stream) = 0;

			virtual uint8_t Read (uint16_t add) = 0;
			// returns true if memory has been updated
			virtual bool Write (uint16_t add, uint8_t val) = 0;

			virtual bool SaveState (std::ostream& stream);
			virtual bool LoadState (std::istream& stream);

		protected:
			uint8_t* m_data;
			uint32_t m_size;
	};

#ifdef __LIBSNES__
	extern uint8_t CartMemData[CartMem::MAX_SIZE+4];
#endif
}

#endif
