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

#ifndef __ARG_RELATIVE_H__
#define __ARG_RELATIVE_H__

#include "argument.hpp"
#include "argregister.hpp"
#include "argimmediate.hpp"

namespace AMeteor
{
	namespace Disassembler
	{
		class ArgRelative : public Argument
		{
			public :
				ArgRelative (const ArgRegister& reg, const Argument& off,
						bool pre, bool up, bool writeback);
				ArgRelative (const ArgRelative& arg);
				~ArgRelative ();

				Argument* Clone () const;

				std::string GetString () const;

			private :
				ArgRegister m_reg;
				const Argument* m_off;
				bool m_pre;
				bool m_up;
				bool m_writeback;
		};
	}
}

#endif
