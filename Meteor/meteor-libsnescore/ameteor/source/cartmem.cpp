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

#include "ameteor/cartmem.hpp"
#include "globals.hpp"

namespace AMeteor
{
#ifdef __LIBSNES__
	uint8_t CartMemData[CartMem::MAX_SIZE+4];
#endif

	CartMem::CartMem() :
#ifdef __LIBSNES__
		m_data(CartMemData)
#else
		m_data(new uint8_t[MAX_SIZE+4])
#endif
	{
	}

	CartMem::~CartMem()
	{
#ifndef __LIBSNES__
		delete [] m_data;
#endif
	}

	bool CartMem::SaveState (std::ostream& stream)
	{
		stream.write((char*)m_data, MAX_SIZE);
		SS_WRITE_VAR(m_size);

		return true;
	}

	bool CartMem::LoadState (std::istream& stream)
	{
		stream.read((char*)m_data, MAX_SIZE);
		SS_READ_VAR(m_size);

		return true;
	}
}
