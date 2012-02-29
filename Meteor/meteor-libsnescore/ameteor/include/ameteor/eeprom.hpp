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

#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "cartmem.hpp"
#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Eeprom : public CartMem
	{
		public :
			Eeprom (bool big);

			void Reset ();

			uint16_t GetSize () const
			{
				return m_size;
			}

			bool Load (std::istream& f);
			bool Save (std::ostream& f);

			uint8_t Read (uint16_t add);
			bool Write (uint16_t add, uint8_t val);

			uint16_t Read ();
			//bool Write (uint16_t val);

			bool Write (uint16_t* data, uint16_t size);
			//XXX
#if 0
			void Read (uint16_t* pOut);
#endif

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		private :
			enum State
			{
				IDLE,
				//WAITING,

				//READ_ADD,
				//READ_END,
				READ_GARBAGE,
				READ_DATA

				/*WRITE_ADD,
				WRITE_DATA,
				WRITE_END*/
			};

			uint8_t m_state;
			uint16_t m_add;
			uint8_t m_pos;
	};
}

#endif
