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

#include "ameteor/audio/speaker.hpp"
#include "../debug.hpp"

namespace AMeteor
{
	namespace Audio
	{
		Speaker::Speaker (uint16_t& cnt1l, uint16_t& cnt1h, uint16_t& cnt1x,
				uint16_t& cnt2l, uint16_t& cnt2h,
				uint16_t& cnt4l, uint16_t& cnt4h,
				uint16_t& cntl, uint16_t& cnth, uint16_t& cntx, uint16_t& bias) :
			// XXX freq
			m_sound1(cnt1l, cnt1h, cnt1x, 44100),
			m_sound2(cnt2l, cnt2h, 44100),
			m_sound4(cnt4l, cnt4h, 44100),
			m_cntl(cntl),
			m_cnth(cnth),
			m_cntx(cntx),
			m_bias(bias)
		{
		}

		Speaker::~Speaker ()
		{
		}

		void Speaker::Reset ()
		{
			m_sound1.Reset();
			m_sound2.Reset();
			m_sound4.Reset();
			m_dsa.Reset();
			m_dsb.Reset();
		}

		void Speaker::SoundTick ()
		{
			int16_t f[2];

			// if master is enabled
			if (m_cntx & (0x1 << 7))
			{
				m_sound1.SoundTick();
				if (m_sound1.IsOn())
					m_cntx |= 0x0001;
				else
					m_cntx &= 0xFFFE;
				m_sound2.SoundTick();
				if (m_sound2.IsOn())
					m_cntx |= 0x0002;
				else
					m_cntx &= 0xFFFD;
				m_sound4.SoundTick();
				if (m_sound4.IsOn())
					m_cntx |= 0x0008;
				else
					m_cntx &= 0xFFF7;
			}

			// left
			f[0] = MixSample (m_cntl >> 4, m_cnth >> 9);
			// right
			f[1] = MixSample (m_cntl, m_cnth >> 8);

			m_sig_frame(f);
		}

		int16_t Speaker::MixSample (uint16_t cntl, uint8_t cnth)
		{
			int16_t sample;

			// if master is enabled
			if (m_cntx & (0x1 << 7))
			{
				int8_t s1, s2, s4;
				s1 = (cntl & (0x1 <<  8)) ? m_sound1.GetSample() : 0;
				s2 = (cntl & (0x1 <<  9)) ? m_sound2.GetSample() : 0;
				s4 = (cntl & (0x1 << 11)) ? m_sound4.GetSample() : 0;

				int16_t dmg = s1 + s2 + s4;
				dmg = (dmg * (cntl & 0x7)) / 7;
				switch (m_cnth & 0x3)
				{
					case 0: // 25%
						dmg /= 4;
						break;
					case 1: // 50%
						dmg /= 2;
						break;
					case 2: // 100%
						break;
					case 3: // Prohibited
						met_abort("Invalid SOUNDCNT_H sound # 1-4 volume");
						break;
				}

				int16_t sA, sB;
				sA = (cnth & (0x1     )) ? m_dsa.GetSample() : 0;
				sB = (cnth & (0x1 << 4)) ? m_dsb.GetSample() : 0;

				if (!(m_cnth & (0x1 << 2)))
					sA /= 2;
				if (!(m_cnth & (0x1 << 3)))
					sB /= 2;

				// TODO when finished put this all together on one line
				sample = (sA + sB) * 4 + dmg;
			}
			else
				sample = 0;

			sample += m_bias & 0x3FF;
			if (sample < 0)
				sample = 0;
			else if (sample >= 0x400)
				sample = 0x3FF;
			sample -= 0x200;
			sample <<= 6;

			return sample;
		}

		bool Speaker::SaveState (std::ostream& stream)
		{
#define WRITE(var) \
	if (!var.SaveState(stream)) \
		return false
			WRITE(m_sound1);
			WRITE(m_sound2);
			WRITE(m_sound4);
			WRITE(m_dsa);
			WRITE(m_dsb);
#undef WRITE

			return true;
		}

		bool Speaker::LoadState (std::istream& stream)
		{
#define READ(var) \
	if (!var.LoadState(stream)) \
		return false
			READ(m_sound1);
			READ(m_sound2);
			READ(m_sound4);
			READ(m_dsa);
			READ(m_dsb);
#undef READ

			return true;
		}
	}
}
