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

#include "ameteor/keypad.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

namespace AMeteor
{
	Keypad::Keypad () :
		m_keyinput(IO.GetRef16(Io::KEYINPUT)),
		m_keycnt(IO.GetRef16(Io::KEYCNT))
	{
	}

	void Keypad::KeyPressed(int code)
	{
		if (m_keys.count(code))
			m_keyinput &= ~m_keys[code];
	}

	void Keypad::KeyReleased(int code)
	{
		if (m_keys.count(code))
			m_keyinput |= m_keys[code];
	}

	void Keypad::JoyButtonPressed (uint16_t joyid, uint16_t button)
	{
		uint32_t id = ((int)joyid) << 16 | button;
		if (m_joys.count(id))
			m_keyinput &= ~m_joys[id];
	}

	void Keypad::JoyButtonReleased (uint16_t joyid, uint16_t button)
	{
		uint32_t id = ((int)joyid) << 16 | button;
		if (m_joys.count(id))
			m_keyinput |= m_joys[id];
	}

	void Keypad::JoyMoved (uint16_t joyid, uint16_t axis, float pos)
	{
		uint32_t id = (((int)joyid) << 16) | ((pos < 0) << 15) | (axis & 0x7FFF);
		// if pos is 0, we disable the positive and negative targets
		if (pos == 0)
		{
			if (m_axis.count(id))
				m_keyinput |= m_axis[id];
			if (m_axis.count(id | (1 << 15)))
				m_keyinput |= m_axis[id | (1 << 15)];
		}
		else
		{
			// we enable the corresponding button
			if (m_axis.count(id))
				m_keyinput &= ~((uint16_t)m_axis[id]);
			// we disable the opposite button (we may have skipped 0)
			if (m_axis.count(id ^ 0x8000))
				m_keyinput |= m_axis[id ^ 0x8000];
		}
	}

	void Keypad::VBlank ()
	{
		// if keypad IRQ are enabled
		if (m_keycnt & (0x1 << 14))
			// if irq condition is and
			if (m_keycnt & (0x1 << 15))
			{
				// if condition is satisfied
				if ((~m_keyinput & m_keycnt & 0x3FF) == (m_keycnt & 0x3FF))
					CPU.SendInterrupt(0x1000);
			}
			// if irq condition is or
			else
			{
				// if condition is satisfied
				if (~m_keyinput & m_keycnt & 0x3FF)
					CPU.SendInterrupt(0x1000);
			}
	}
}
