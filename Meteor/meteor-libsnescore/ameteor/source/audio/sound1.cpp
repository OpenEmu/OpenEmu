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

#include "ameteor/audio/sound1.hpp"
#include "../globals.hpp"
#include <cmath>

namespace AMeteor
{
	namespace Audio
	{
		Sound1::Sound1 (uint16_t& cntl, uint16_t& cnth, uint16_t& cntx,
				uint16_t freq) :
			m_cntl(cntl),
			m_cnth(cnth),
			m_cntx(cntx),
			m_on(false),
			m_posP(0),
			m_posS(0),
			m_posE(0),
			m_sample(0),
			m_speriod(16*1024*1024/freq),
			m_envelope(0),
			m_length(0),
			m_timed(false)
		{
		}

		void Sound1::Reset ()
		{
			m_on = false;
			m_timed = false;
			m_length = 0;
			m_envelope = 0;
			m_posP = m_posE = m_posS = 0;
			m_sample = 0;
		}

		void Sound1::ResetSound ()
		{
			m_on = true;
			m_timed = (m_cntx & (0x1 << 14));
			m_length = (64 - (m_cnth & 0x3F)) * ((16*1024*1024)/256);
			m_envelope = m_cnth >> 12;
			m_posE = m_posS = 0;
		}

		void Sound1::SoundTick ()
		{
			// remember here that the processors runs at 16MHz = 16,777,216 cycles/s
			// and this function is called normally at 44,100 Hz

			m_posP += m_speriod;
			m_posS += m_speriod;
			m_posE += m_speriod;
			if (m_length > m_speriod)
				m_length -= m_speriod;
			else
			{
				if (m_timed)
					m_on = false;
				m_length = 0;
			}

			// sweep time in cycles
			// maximum is 917,504, so we need a 32 bits int
			uint32_t sweeptime = ((m_cntl >> 4) & 0x7) * ((16*1024*1024)/128);
			// period in cycles
			// period = 16M/freq
			// freq = 128K/(2048 - (SOUND1CNT_X & 0x7FF))
			// maximum is 262,144, so we need a 32 bits int
			uint32_t period =
				((16*1024*1024) / (128*1024)) * (2048 - (m_cntx & 0x7FF));
			// frequency as contained in SOUND1CNT_X
			uint16_t freq = m_cntx & 0x7FF;

			// we rewind posP
			m_posP %= period;

			// the envelope now
			// envelope step time in cycles
			uint32_t steptime = ((m_cnth >> 8) & 0x7) * ((16*1024*1024)/64);
			// the envelope can't do two steps between to calls of SoundTick
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

			// if the envelope is null or the sound is finished, no need to calculate
			// anything
			if (m_on && m_envelope)
			{
				// we set the sample according to the position in the current period
				// and the wave duty cycle
				switch ((m_cnth >> 6) & 0x3)
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

			// there can't have been more than one sweep between two call of
			// SoundTick since SoundTick is called at least at a frequency of 4,000Hz
			// (alsa can't output at a lower samplerate on my sound card) and sweeps
			// happen at maximum at a frequency of 128Hz

			// if the channel is on and sweep is enabled and it's time to sweep
			if (m_on && sweeptime && m_posS > sweeptime)
			{
				// n = sweep shifts (in SOUND1CNT_L)
				if (m_cntl & (0x1 << 3))
					// F(t+1) = F(t) - F(t) / 2^n
					freq = freq - freq / (1 << (m_cntl & 0x7));
					// freq won't go under 1 since when freq = 2, freq - freq / 2 (the
					// minimum sweep shift) = 1 and then freq - freq / 2 = 1
					// because 1/2 = 0
				else
				{
					// F(t+1) = F(t) + F(t) / 2^n
					freq = freq + freq / (1 << (m_cntl & 0x7));
					if (freq > 2047)
					{
						m_on = false;
						freq = 2047;
					}
				}

				// we update the frequency in the cntx register
				m_cntx = (m_cntx & 0xF800) | freq;

				// now we rewind posS
				m_posS -= sweeptime;
			}
		}

		bool Sound1::SaveState (std::ostream& stream)
		{
			SS_WRITE_VAR(m_on);
			SS_WRITE_VAR(m_posP);
			SS_WRITE_VAR(m_posS);
			SS_WRITE_VAR(m_posE);
			SS_WRITE_VAR(m_sample);
			SS_WRITE_VAR(m_envelope);
			SS_WRITE_VAR(m_length);
			SS_WRITE_VAR(m_timed);

			return true;
		}

		bool Sound1::LoadState (std::istream& stream)
		{
			SS_READ_VAR(m_on);
			SS_READ_VAR(m_posP);
			SS_READ_VAR(m_posS);
			SS_READ_VAR(m_posE);
			SS_READ_VAR(m_sample);
			SS_READ_VAR(m_envelope);
			SS_READ_VAR(m_length);
			SS_READ_VAR(m_timed);

			return true;
		}
	}
}
