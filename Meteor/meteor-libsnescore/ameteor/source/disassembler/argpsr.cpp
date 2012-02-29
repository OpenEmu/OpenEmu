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

#include "ameteor/disassembler/argpsr.hpp"

namespace AMeteor
{
	namespace Disassembler
	{
		std::string ArgPsr::GetString () const
		{
			std::string out;

			if (m_spsr)
				out = "SPSR";
			else
				out = "CPSR";

			if (m_fields <= 0xF)
			{
				out += '_';
				if (m_fields & 0x1)
					out += 'c';
				if (m_fields & 0x2)
					out += 'x';
				if (m_fields & 0x4)
					out += 's';
				if (m_fields & 0x8)
					out += 'f';
			}

			return out;
		}

		Argument* ArgPsr::Clone () const
		{
			return new ArgPsr(*this);
		}
	}
}
