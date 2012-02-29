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

#include "ameteor/flash.hpp"
#include "globals.hpp"
#include <cstring>
#include <fstream>

namespace AMeteor
{
	Flash::Flash (bool big) :
		CartMem(),
		m_state(NORMAL)
	{
		if (big)
		{
			m_device_id = 0x13;
			m_manufacturer_id = 0x62;
			m_size = 128*1024;
		}
		else
		{
			m_device_id = 0x1B;
			m_manufacturer_id = 0x32;
			m_size = 64*1024;
		}

		*(uint32_t*)(m_data+MAX_SIZE) = m_size;
	}

	void Flash::Reset ()
	{
		std::memset(m_data, 0, m_size);
	}

	bool Flash::Load (std::istream& f)
	{
		f.read((char*)m_data, m_size);
		return f.good();
	}

	bool Flash::Save (std::ostream& f)
	{
		f.write((char*)m_data, m_size);
		return f.good();
	}

	uint8_t Flash::Read (uint16_t add)
	{
		switch (m_state)
		{
			case NORMAL:
				return m_data[add];
			case ID:
				switch (add)
				{
					case 0: return m_manufacturer_id;
					case 1: return m_device_id;
					default: return 0;
				}
			// FIXME vba returns 0xff after an erase regardless of the read address
			default:
				return 0;
		}
	}

	bool Flash::Write (uint16_t add, uint8_t val)
	{
		switch (m_state)
		{
			case ID:
			case NORMAL:
				if (add == 0x5555 && val == 0xAA)
					m_state = CMD1;
				else
					m_state = NORMAL; // for case ID
				break;
			case CMD1:
				if (add == 0x2AAA && val == 0x55)
					m_state = CMD2;
				else
					m_state = NORMAL;
				break;
			case CMD2:
				if (add == 0x5555)
					switch (val)
					{
						case 0x80: // erase mode
							m_state = ERASE1;
							break;
						case 0x90: // id mode
							m_state = ID;
							break;
						case 0xA0: // write byte
							m_state = WRITE;
							break;
						case 0xF0:
							m_state = NORMAL;
							break;
						default:
							m_state = NORMAL;
							break;
					}
				else
					m_state = NORMAL;
				break;
			case ERASE1:
				if (add == 0x5555 && val == 0xAA)
					m_state = ERASE2;
				else
					m_state = NORMAL;
				break;
			case ERASE2:
				if (add == 0x2AAA && val == 0x55)
					m_state = ERASE3;
				else
					m_state = NORMAL;
				break;
			case ERASE3:
			// according to vba, after a whole erase command we can juste repeat the
			// last byte of the command to execute another erase command
				switch (val)
				{
					case 0x10: // erase entire chip
						if (add == 0x5555)
							memset(m_data, 0xFF, m_size);
						m_state = NORMAL;
						break;
					case 0x30: // erase sector
						if (!(add & 0x0FFF))
							memset(m_data+add, 0xFF, 0x1000);
						m_state = NORMAL;
						break;
					default:
						m_state = NORMAL;
						break;
				}
				break;
			case WRITE:
				// I think this is how it works
				m_data[add] &= val;
				m_state = NORMAL;
				return true;
		}
		return false;
	}

	bool Flash::SaveState (std::ostream& stream)
	{
		SS_WRITE_VAR(m_state);

		SS_WRITE_DATA(m_data, m_size);

		return true;
	}

	bool Flash::LoadState (std::istream& stream)
	{
		SS_READ_VAR(m_state);

		SS_READ_DATA(m_data, m_size);

		return true;
	}
}
