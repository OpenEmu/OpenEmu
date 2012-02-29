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

#include "ameteor/disassembler/argregister.hpp"

#include <sstream>

namespace AMeteor
{
	namespace Disassembler
	{
		std::string ArgRegister::GetString () const
		{
			static const char* SpeRegisters[] = {"SP", "LR", "PC"};

			std::ostringstream ss;

			if (m_memory)
				ss << '[';

			if (m_special)
				ss << SpeRegisters[m_reg-13];
			else
				ss << 'r' << (int)m_reg;

			if (m_memory)
				ss << ']';

			if (m_writeback)
				ss << '!';

			return ss.str();
		}

		Argument* ArgRegister::Clone () const
		{
			return new ArgRegister(*this);
		}
	}
}
