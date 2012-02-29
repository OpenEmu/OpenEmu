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

#include "ameteor/disassembler/arguments.hpp"

namespace AMeteor
{
	namespace Disassembler
	{
		Arguments::~Arguments ()
		{
			Clear();
		}

		void Arguments::Clear ()
		{
			for (std::vector<Argument*>::iterator iter = m_args.begin();
					iter != m_args.end(); ++iter)
			{
				delete *iter;
			}
			m_args.clear();
		}

		std::string Arguments::GetString () const
		{
			std::string out;
			for (std::vector<Argument*>::const_iterator iter = m_args.begin();
					iter != m_args.end(); ++iter)
			{
				if (!out.empty())
					out += ", ";
				out += (*iter)->GetString();
			}
			return out;
		}
	}
}
