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

#include "ameteor/timer.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

namespace AMeteor
{
	static const uint16_t Prescalers[] = {1, 64, 256, 1024};

	void Timer::Reset ()
	{
		m_reload = 0;
		m_count = 0;
		m_control.w = 0;
	}

	void Timer::Reload ()
	{
		// FIXME to test on hardware, vba update the prescaler even if the timer
		// didn't restart
		// if the timer is at 33, 63 cycles have passed since the last timer
		// increment and prescaler is 1/64, setting TM0CNT even to the same value
		// it had will reset the prescaler part, so the timer would be at 33 and we
		// will have to wait another 64 cycles before having it go to 34
		//
		// the current behaviour for the above example is that it will reach 34 in
		// 1 cycle instead of 64
		if (!m_control.b.start &&
				(IO.DRead16(Io::TM0CNT_H + m_num * Io::TIMER_SIZE)
				 & (0x1 << 7)))
			// if start has changed from 0 to 1
		{
			m_control.w = IO.DRead16(Io::TM0CNT_H + m_num * Io::TIMER_SIZE);
			m_count = 65536 - m_reload;
			if (!m_control.b.countup)
			{
				m_count *= Prescalers[m_control.b.prescaler];

				// here, the str instruction which have triggered this function
				// will be taken in account by this timer
				// in other words, if the str instruction takes 3 cycles, the
				// timer will be incremented by 3 cycles just after its start
				CLOCK.SetTimer(m_num, m_count);
			}
		}
		else
		{
			uint16_t cnt = IO.DRead16(Io::TM0CNT_H + m_num * Io::TIMER_SIZE);
			if (m_control.b.start && (cnt & (0x1 << 7))
					&& m_control.b.prescaler != (cnt & 0x3))
				met_abort("Prescaler changed while timer " << (int)m_num << " was up");

			m_control.w = IO.DRead16(Io::TM0CNT_H + m_num * Io::TIMER_SIZE);

			if (!m_control.b.start)
				CLOCK.DisableTimer(m_num);
		}

		if (m_num == 0 && m_control.b.countup)
			met_abort("Count-up on first timer !");
	}

	uint16_t Timer::GetCount () const
	{
		if (m_control.b.countup)
			return 65536 - m_count;
		else
			return 65536 -
					CLOCK.GetTimer(m_num) / Prescalers[m_control.b.prescaler];
	}

	void Timer::TimeEvent ()
	{
		debug("Timer" << (int)m_num << " overflow");
		SOUND.TimerOverflow(m_num);

		m_count = 65536 - m_reload;
		if (!m_control.b.countup)
		{
			m_count *= Prescalers[m_control.b.prescaler];

			// GetTimer should be zero or less since this function was called
			if (m_count >= (unsigned short)-CLOCK.GetTimer(m_num))
			{
				m_count += CLOCK.GetTimer(m_num);

				CLOCK.SetTimer(m_num, m_count);
			}
			else
			{
				CLOCK.AddTimer(m_num, m_count);
			}
		}

		if (m_control.b.irq)
			CPU.SendInterrupt(0x1 << (3 + m_num));

		if (m_num != 3)
			m_next->Countup();
	}

	void Timer::Countup ()
	{
		if (m_control.b.countup)
		{
			--m_count;
			if (m_count == 0)
				TimeEvent();
		}
	}

	bool Timer::SaveState (std::ostream& stream)
	{
		SS_WRITE_VAR(m_reload);
		SS_WRITE_VAR(m_count);
		SS_WRITE_VAR(m_control);

		return true;
	}

	bool Timer::LoadState (std::istream& stream)
	{
		SS_READ_VAR(m_reload);
		SS_READ_VAR(m_count);
		SS_READ_VAR(m_control);

		return true;
	}
}
