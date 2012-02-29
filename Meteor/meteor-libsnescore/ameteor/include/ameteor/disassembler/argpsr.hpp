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

#ifndef __ARG_PSR_H__
#define __ARG_PSR_H__

#include "argument.hpp"

#include <stdint.h>

namespace AMeteor
{
	namespace Disassembler
	{
		class ArgPsr : public Argument
		{
			public :
				ArgPsr (bool spsr, uint8_t fields = 0xFF) :
					m_spsr(spsr),
					m_fields(fields)
				{ }

				Argument* Clone () const;

				std::string GetString () const;

			private :
				bool m_spsr;
				uint8_t m_fields;
		};
	}
}

#endif
