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

#ifndef __AUDIO_SOUND_2_H__
#define __AUDIO_SOUND_2_H__

#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	namespace Audio
	{
		class Sound2
		{
			public :
				Sound2 (uint16_t& cntl, uint16_t& cnth, uint16_t freq);

				void Reset ();

				// call this at the frequence given in constructor
				void SoundTick ();

				void ResetSound ();
				void ResetEnvelope ()
				{
					m_envelope = 0;
				}

				int8_t GetSample () const
				{
					return m_sample;
				}

				bool IsOn () const
				{
					return m_on;
				}

				bool SaveState (std::ostream& stream);
				bool LoadState (std::istream& stream);

			private :
				uint16_t &m_cntl, &m_cnth;
				bool m_on;
				uint32_t m_posP, m_posE;
				int8_t m_sample;
				uint16_t m_speriod;
				uint8_t m_envelope;
				uint32_t m_length;
				bool m_timed;
		};
	}
}

#endif
