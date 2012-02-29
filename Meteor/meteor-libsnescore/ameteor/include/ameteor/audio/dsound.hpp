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

#ifndef __AUDIO_D_SOUND_H__
#define __AUDIO_D_SOUND_H__

#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	namespace Audio
	{
		class DSound
		{
			public :
				static const int BUFFER_SIZE = 32;

				DSound ();

				void FillFifo (int8_t* buffer);
				void FillFifo (int8_t sample);

				void NextSample ()
				{
					// if the buffer is empty, there is nothing to do
					if (m_size)
						// if this was the last sample, we reset all and send 0 to all next
						// GetSample()s until the buffer is refilled
						if (--m_size == 0)
							Reset();
						// else, we go on to next sample and we go to the first if we got
						// to the last
						else if (++m_rpos >= BUFFER_SIZE)
							m_rpos = 0;
				}

				void Reset ()
				{
					m_buffer[0] = m_size = m_rpos = m_wpos = 0;
				}

				int8_t GetSample()
				{
					return m_buffer[m_rpos];
				}

				uint8_t GetSize ()
				{
					return m_size;
				};

				bool SaveState (std::ostream& stream);
				bool LoadState (std::istream& stream);

			private :
				int8_t m_buffer[BUFFER_SIZE];
				uint8_t m_rpos, m_wpos;
				uint8_t m_size;
		};
	}
}

#endif
