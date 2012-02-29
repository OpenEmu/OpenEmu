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

#include "ameteor/disassembler/argmulregisters.hpp"

#include <sstream>

namespace AMeteor
{
	namespace Disassembler
	{
		std::string ArgMulRegisters::GetString () const
		{
			std::ostringstream ss;
			ss << '{';

			bool open = false;

			for (Registers::const_iterator iter = m_regs.begin();
					iter != m_regs.end(); ++iter)
			{
				if (open &&
						iter + 1 < m_regs.end() &&
						*(iter + 1) == (*iter) + 1)
				{
					continue;
				}
				else if (iter + 2 < m_regs.end() &&
						*(iter + 1) == (*iter) + 1 &&
						*(iter + 2) == (*iter) + 2)
				{
					ss << 'r' << (int)*iter << '-';
					open = true;
				}
				else
				{
					ss << 'r' << (int)*iter;
					open = false;
					if (iter + 1 != m_regs.end())
						ss << ", ";
				}
			}

			if (m_lastreg == SPREG_LR)
			{
				if (ss.width() != 1)
					ss << ", ";
				ss << "lr";
			}
			else if (m_lastreg == SPREG_PC)
			{
				if (ss.width() != 1)
					ss << ", ";
				ss << "pc";
			}

			ss << '}';

			if (m_forceuser)
				ss << '^';

			return ss.str();
		}

		Argument* ArgMulRegisters::Clone () const
		{
			return new ArgMulRegisters(*this);
		}
	}
}
