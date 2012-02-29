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

#include "ameteor/audio/sound2.hpp"
#include "../globals.hpp"
#include <cmath>

namespace AMeteor
{
	namespace Audio
	{
		Sound2::Sound2 (uint16_t& cntl, uint16_t& cnth, uint16_t freq) :
			m_cntl(cntl),
			m_cnth(cnth),
			m_on(false),
			m_posP(0),
			m_posE(0),
			m_sample(0),
			m_speriod(16*1024*1024/freq),
			m_envelope(0),
			m_length(0),
			m_timed(false)
		{
		}

		void Sound2::Reset ()
		{
			m_on = false;
			m_timed = false;
			m_length = 0;
			m_envelope = 0;
			m_posP = m_posE = 0;
			m_sample = 0;
		}

		void Sound2::ResetSound ()
		{
			m_on = true;
			m_timed = (m_cnth & (0x1 << 14));
			m_length = (64 - (m_cntl & 0x3F)) * ((16*1024*1024)/256);
			m_envelope = m_cntl >> 12;
			m_posE = 0;
		}

		void Sound2::SoundTick ()
		{
			// refer at sound1 to know how sound2 works

			m_posP += m_speriod;
			m_posE += m_speriod;
			if (m_length > m_speriod)
				m_length -= m_speriod;
			else
			{
				if (m_timed)
					m_on = false;
				m_length = 0;
			}

			uint32_t period =
				((16*1024*1024) / (128*1024)) * (2048 - (m_cnth & 0x7FF));

			m_posP %= period;

			uint32_t steptime = ((m_cntl >> 8) & 0x7) * ((16*1024*1024)/64);
			if (steptime && m_posE > steptime)
			{
				if (m_cntl & (0x1 << 11))
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
				switch ((m_cntl >> 6) & 0x3)
				{
					case 0: // 12.5%
						m_sample = m_posP < period/8 ? 112 : -16;
						break;
					case 1: // 25%
						m_sample = m_posP < period/4 ? 96 : -32;
						break;
					case 2: // 50%
						m_sample = m_posP < period/2 ? 64 : -64;
						break;
					case 3: // 75%
						m_sample = m_posP < (3*period)/4 ? 32 : -96;
						break;
				}

				m_sample = (((int16_t)m_sample) * m_envelope)/15;
			}
			else
				m_sample = 0;
		}

		bool Sound2::SaveState (std::ostream& stream)
		{
			SS_WRITE_VAR(m_on);
			SS_WRITE_VAR(m_posP);
			SS_WRITE_VAR(m_posE);
			SS_WRITE_VAR(m_sample);
			SS_WRITE_VAR(m_envelope);
			SS_WRITE_VAR(m_length);
			SS_WRITE_VAR(m_timed);

			return true;
		}

		bool Sound2::LoadState (std::istream& stream)
		{
			SS_READ_VAR(m_on);
			SS_READ_VAR(m_posP);
			SS_READ_VAR(m_posE);
			SS_READ_VAR(m_sample);
			SS_READ_VAR(m_envelope);
			SS_READ_VAR(m_length);
			SS_READ_VAR(m_timed);

			return true;
		}
	}
}
