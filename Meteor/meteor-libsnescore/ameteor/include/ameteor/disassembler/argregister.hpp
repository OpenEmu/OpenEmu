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

#ifndef __ARG_REGISTER_H__
#define __ARG_REGISTER_H__

#include "argument.hpp"

#include <stdint.h>

namespace AMeteor
{
	namespace Disassembler
	{
		class ArgRegister : public Argument
		{
			public :
				ArgRegister (uint8_t reg, bool writeback = false,
						bool special = false, bool memory = false) :
					m_reg(reg),
					m_writeback(writeback),
					m_special(special),
					m_memory(memory)
				{ }

				Argument* Clone () const;

				std::string GetString () const;
				uint8_t GetRegister () const
				{
					return m_reg;
				}

			private :
				uint8_t m_reg;
				bool m_writeback;
				bool m_special;
				bool m_memory;
		};
	}
}

#endif
