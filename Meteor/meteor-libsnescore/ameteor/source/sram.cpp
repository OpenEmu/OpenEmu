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

#include "ameteor/sram.hpp"
#include <cstring>
#include <fstream>
#include "globals.hpp"

namespace AMeteor
{
	Sram::Sram () :
		CartMem()
	{
		m_size = SIZE;

		*(uint32_t*)(m_data+MAX_SIZE) = m_size;
	}

	void Sram::Reset ()
	{
		std::memset(m_data, 0, SIZE);
	}

	bool Sram::Load (std::istream& f)
	{
		f.read((char*)m_data, SIZE);
		return f.good();
	}

	bool Sram::Save (std::ostream& f)
	{
		f.write((char*)m_data, SIZE);
		return f.good();
	}

	bool Sram::SaveState (std::ostream& stream)
	{
		SS_WRITE_DATA(m_data, SIZE);

		return true;
	}

	bool Sram::LoadState (std::istream& stream)
	{
		SS_READ_DATA(m_data, SIZE);

		return true;
	}
}
