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

#ifndef __TIMER_H__
#define __TIMER_H__

#include "clock.hpp"

#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Timer
	{
		public :
			Timer (int8_t num, Timer* next) :
				m_num(num),
				m_reload(0),
				m_count(0),
				m_control(0),
				m_next(next)
			{
			}

			void Reset ();

			void SetReload (uint16_t rel)
			{
				m_reload = rel;
			}
			void Reload ();

			uint16_t GetCount () const;

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		private :
			union Control
			{
				Control(uint16_t v) :
					w(v)
				{ }

				uint16_t w;
				struct
				{
					unsigned int prescaler : 2;
					bool countup           : 1;
					unsigned int unused1   : 3;
					bool irq               : 1;
					bool start             : 1;
					unsigned int unused2   : 8;
				} b;
			};

			const int8_t m_num;
			uint16_t m_reload;
			uint32_t m_count;
			Control m_control;

			Timer* m_next;

			void TimeEvent ();
			void Countup ();

			friend void Clock::Commit();
	};
}

#endif
