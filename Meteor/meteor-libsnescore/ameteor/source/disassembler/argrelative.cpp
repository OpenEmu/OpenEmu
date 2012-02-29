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

#include "ameteor/disassembler/argrelative.hpp"

#include <sstream>

namespace AMeteor
{
	namespace Disassembler
	{
		ArgRelative::ArgRelative (const ArgRegister& reg, const Argument& off,
				bool pre, bool up, bool writeback) :
			Argument(),
			m_reg(reg),
			m_off(off.Clone()),
			m_pre(pre),
			m_up(up),
			m_writeback(writeback)
		{ }

		ArgRelative::ArgRelative (const ArgRelative& arg) :
			Argument(),
			m_reg(arg.m_reg),
			m_off(arg.m_off->Clone()),
			m_pre(arg.m_pre),
			m_up(arg.m_up),
			m_writeback(arg.m_writeback)
		{ }

		ArgRelative::~ArgRelative ()
		{
			delete m_off;
		}

		Argument* ArgRelative::Clone () const
		{
			return new ArgRelative(*this);
		}

		std::string ArgRelative::GetString () const
		{
			std::ostringstream ss;
			ss << "[r" << (int)m_reg.GetRegister();

			if (m_pre)
			{
				ss << (m_up ? ", +" : ", -") << m_off->GetString() << ']';
				if (m_writeback)
					ss << '!';
			}
			else
				ss << (m_up ? "], +" : "], -") << m_off->GetString();

			return ss.str();
		}
	}
}
