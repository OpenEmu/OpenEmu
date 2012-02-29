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

#include "ameteor/clock.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

namespace AMeteor
{
	void Clock::Reset ()
	{
		// lcd is enabled by default
		m_first = m_count = m_cycles = m_lcd = m_sound = 0;
		// timers and battery are disabled by default
		m_battery = m_timer[0] = m_timer[1] = m_timer[2] = m_timer[3] =
			INT_MAX;
	}

	void Clock::Commit ()
	{
		unsigned short tocommit;

		m_count += m_cycles;

		// this loop is here because a timer can trigger a dma which will take a
		// long time, during this time the lcd must draw and the timers continue
		while (m_cycles >= m_first)
		{
			tocommit = m_cycles;
			m_cycles = 0;

			m_lcd -= tocommit;
			while (m_lcd <= 0)
				LCD.TimeEvent();

			m_sound -= tocommit;
			while (m_sound <= 0)
			{
				SOUND.TimeEvent();
				// XXX freq
				m_sound += SOUND_PERIOD;
			}

#define COMMIT(dev, obj) \
	if (m_##dev != INT_MAX) \
	{ \
		m_##dev -= tocommit; \
		while (m_##dev <= 0) \
			obj.TimeEvent(); \
	}
			COMMIT(timer[0], TIMER0)
			COMMIT(timer[1], TIMER1)
			COMMIT(timer[2], TIMER2)
			COMMIT(timer[3], TIMER3)
			COMMIT(battery, MEM)
#undef COMMIT

			SetFirst();
		}
	}

	void Clock::WaitForNext ()
	{
		m_cycles = m_first;
		Commit();
	}

#define SETFIRST(dev) \
	if (m_##dev < m_first) \
		m_first = m_##dev
	void Clock::SetFirst ()
	{
		m_first = m_lcd;
		SETFIRST(timer[0]);
		SETFIRST(timer[1]);
		SETFIRST(timer[2]);
		SETFIRST(timer[3]);
		SETFIRST(sound);
		SETFIRST(battery);
	}
#undef SETFIRST

	bool Clock::SaveState (std::ostream& stream)
	{
		SS_WRITE_VAR(m_cycles);
		SS_WRITE_VAR(m_first);
		SS_WRITE_VAR(m_lcd);
		SS_WRITE_VAR(m_sound);
		SS_WRITE_VAR(m_battery);

		SS_WRITE_ARRAY(m_timer);

		return true;
	}

	bool Clock::LoadState (std::istream& stream)
	{
		SS_READ_VAR(m_cycles);
		SS_READ_VAR(m_first);
		SS_READ_VAR(m_lcd);
		SS_READ_VAR(m_sound);
		SS_READ_VAR(m_battery);

		SS_READ_ARRAY(m_timer);

		return true;
	}
}
