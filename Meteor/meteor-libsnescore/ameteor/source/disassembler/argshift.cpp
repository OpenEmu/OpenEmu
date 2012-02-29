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

#include "ameteor/disassembler/argshift.hpp"

#include <sstream>

namespace AMeteor
{
	namespace Disassembler
	{
		ArgShift::ArgShift (const Argument& arg1, const Argument& arg2,
				ShiftType type, bool memory) :
			Argument(),
			m_arg1(arg1.Clone()),
			m_arg2(arg2.Clone()),
			m_type(type),
			m_memory(memory)
		{ }

		ArgShift::ArgShift (const ArgShift& arg) :
			Argument(),
			m_arg1(arg.m_arg1->Clone()),
			m_arg2(arg.m_arg2->Clone()),
			m_type(arg.m_type),
			m_memory(arg.m_memory)
		{ }

		ArgShift::~ArgShift ()
		{
			delete m_arg1;
			delete m_arg2;
		}

		Argument* ArgShift::Clone () const
		{
			return new ArgShift(*this);
		}

		std::string ArgShift::GetString () const
		{
			static const char* Shifts[] = {", LSL ", ", LSR ", ", ASR ", ", ROR ",
				", RRX "};

			std::ostringstream ss;

			if (m_memory)
				ss << '[';

			ss << m_arg1->GetString() << Shifts[m_type] << m_arg2->GetString();

			if (m_memory)
				ss << ']';

			return ss.str();
		}
	}
}
