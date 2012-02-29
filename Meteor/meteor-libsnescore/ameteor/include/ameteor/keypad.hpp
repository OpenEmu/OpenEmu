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

#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <stdint.h>
#include <map>

namespace AMeteor
{
	class Keypad
	{
		public :
			enum Button
			{
				BTN_A      = 0x001,
				BTN_B      = 0x002,
				BTN_SELECT = 0x004,
				BTN_START  = 0x008,
				BTN_RIGHT  = 0x010,
				BTN_LEFT   = 0x020,
				BTN_UP     = 0x040,
				BTN_DOWN   = 0x080,
				BTN_R      = 0x100,
				BTN_L      = 0x200
			};

			Keypad ();

			void Reset ()
			{
			}

			void BindKey(int code, Button btn)
			{
				m_keys[code] = (uint16_t)btn;
			}
			void UnbindKey(int code)
			{
				m_keys.erase(code);
			}
			void BindJoy(uint16_t joyid, uint16_t button, Button btn)
			{
				m_joys[((int)joyid) << 16 | button] = (uint16_t)btn;
			}
			void UnbindJoy(uint16_t joyid, uint16_t button)
			{
				m_joys.erase(((int)joyid) << 16 | button);
			}
			void BindAxis(uint16_t joyid, uint16_t axis, Button btn)
			{
				m_axis[((int)joyid) << 16 | axis] = (uint16_t)btn;
			}
			void UnbindAxis(uint16_t joyid, uint16_t axis)
			{
				m_axis.erase(((int)joyid) << 16 | axis);
			}

			void ResetBindings()
			{
				m_keys.clear();
				m_joys.clear();
				m_axis.clear();
			}

			inline void SetPadState(uint16_t keys);

			void KeyPressed(int code);
			void KeyReleased(int code);
			void JoyButtonPressed (uint16_t joyid, uint16_t button);
			void JoyButtonReleased (uint16_t joyid, uint16_t button);
			void JoyMoved (uint16_t joyid, uint16_t axis, float pos);

			void VBlank ();

		private :
			uint16_t& m_keyinput;
			uint16_t& m_keycnt;

			std::map<int, uint16_t> m_keys;
			std::map<int, uint16_t> m_joys;
			std::map<int, uint16_t> m_axis;
	};

	void Keypad::SetPadState(uint16_t keys)
	{
		m_keyinput = keys;
	}
}

#endif
