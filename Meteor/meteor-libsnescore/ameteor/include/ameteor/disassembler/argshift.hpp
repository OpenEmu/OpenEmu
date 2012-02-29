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

#ifndef __ARG_SHIFT_H__
#define __ARG_SHIFT_H__

#include "argument.hpp"

namespace AMeteor
{
	namespace Disassembler
	{
		enum ShiftType
		{
			SHIFT_LSL = 0,
			SHIFT_LSR,
			SHIFT_ASR,
			SHIFT_ROR,
			SHIFT_RRX
		};

		class ArgShift : public Argument
		{
			public :
				ArgShift (const Argument& arg1, const Argument& arg2,
						ShiftType type, bool memory);
				ArgShift (const ArgShift& arg);
				~ArgShift ();

				Argument* Clone () const;

				std::string GetString () const;

			private :
				const Argument* m_arg1;
				const Argument* m_arg2;
				ShiftType m_type;
				bool m_memory;
		};
	}
}

#endif
