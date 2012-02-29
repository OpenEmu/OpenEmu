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

#include "ameteor/audio/sound4.hpp"
#include "../globals.hpp"

namespace AMeteor
{
	namespace Audio
	{
		static bool Noise7Stages[127];
		static bool Noise15Stages[32767];

		void InitNoise ()
		{
			unsigned short i = 0x7f;
			bool* pNoise = Noise7Stages;
			do
			{
				*pNoise++ = i & 1;
				i = (i >> 1) | (((i & 1) << 6) ^ ((i & 2) << 5));
			} while (i != 0x7f);

			i = 0x7fff;
			pNoise = Noise15Stages;
			do
			{
				*pNoise++ = i & 1;
				i = (i >> 1) | (((i & 1) << 14) ^ ((i & 2) << 13));
			} while (i != 0x7fff);
		}

		Sound4::Sound4 (uint16_t& cntl, uint16_t& cnth, uint16_t freq) :
			m_cntl(cntl),
			m_cnth(cnth),
			m_on(false),
			m_posP(0),
			m_posN(0),
			m_posE(0),
			m_sample(0),
			m_speriod(16*1024*1024/freq),
			m_envelope(0),
			m_length(0),
			m_timed(false),
			m_div(4*8/2)
		{
		}

		void Sound4::Reset ()
		{
			m_on = false;
			m_timed = false;
			m_length = 0;
			m_envelope = 0;
			m_posP = m_posE = m_posN = 0;
			m_sample = 0;
			m_div = 4*8/2;
		}

		void Sound4::ResetSound ()
		{
			m_on = true;
			m_timed = (m_cnth & (0x1 << 14));
			m_length = (64 - (m_cntl & 0x3F)) * ((16*1024*1024)/256);
			m_envelope = m_cntl >> 12;
			m_div = ((m_cnth & 0x7) ? 4*8*(m_cnth & 0x7) : 4*8/2);
			m_posE = m_posP = 0;
		}

		void Sound4::SoundTick ()
		{
			// rest is the number of processor clock ticks that were not yet taken by
			// the noise clock divider (if the total divider is 8 and we have 10
			// ticks of the processor clock, only 8 were taken by the noise clock),
			// the rest will be taken by the next call of SoundTick()
			uint16_t rest = m_posP + m_speriod;
			// advance is the number of noise ticks that have passed
			uint16_t advance = m_posP + m_speriod;

			// time of one sound tick in cycles
			uint32_t tick = m_div;
			// if shift is 111X in binary
			if (((m_cnth >> 5) & 0x7) == 0x7)
				// not used
				// assume 13
				tick *= 1 << 14;
			else
				tick *= (2 << ((m_cnth >> 4) & 0xF));

			rest %= tick;
			advance /= tick;

			m_posP = rest;
			m_posN += advance;
			// we have this modulo on posN so that when you switch from 15 stages to
			// 7 stages and then you switch back, you won't restart the 15 stages
			// pattern from the beginning
			// don't know if GBA handle this like that
			m_posN %= 32768;

			m_posE += m_speriod;
			if (m_length > m_speriod)
				m_length -= m_speriod;
			else
			{
				if (m_timed)
					m_on = false;
				m_length = 0;
			}

			uint32_t steptime = ((m_cntl >> 8) & 0x7) * ((16*1024*1024)/64);
			if (steptime && m_posE > steptime)
			{
				if (m_cnth & (0x1 << 11))
				{
					if (m_envelope < 15)
						++m_envelope;
				}
				else
				{
					if (m_envelope > 0)
						--m_envelope;
				}

				m_posE -= steptime;
			}

			if (m_on && m_envelope)
			{
				if (m_cnth & (0x1 << 3))
					m_sample = Noise7Stages[m_posN % 128];
				else
					m_sample = Noise15Stages[m_posN];

				m_sample = m_sample ? 127 : -127;
				m_sample = (((int16_t)m_sample) * m_envelope)/15;
			}
			else
				m_sample = 0;
		}

		bool Sound4::SaveState (std::ostream& stream)
		{
			SS_WRITE_VAR(m_on);
			SS_WRITE_VAR(m_posP);
			SS_WRITE_VAR(m_posE);
			SS_WRITE_VAR(m_posN);
			SS_WRITE_VAR(m_sample);
			SS_WRITE_VAR(m_envelope);
			SS_WRITE_VAR(m_length);
			SS_WRITE_VAR(m_timed);
			SS_WRITE_VAR(m_div);

			return true;
		}

		bool Sound4::LoadState (std::istream& stream)
		{
			SS_READ_VAR(m_on);
			SS_READ_VAR(m_posP);
			SS_READ_VAR(m_posE);
			SS_READ_VAR(m_posN);
			SS_READ_VAR(m_sample);
			SS_READ_VAR(m_envelope);
			SS_READ_VAR(m_length);
			SS_READ_VAR(m_timed);
			SS_READ_VAR(m_div);

			return true;
		}
	}
}
