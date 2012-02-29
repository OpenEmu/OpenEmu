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

#include "ameteor/sound.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

namespace AMeteor
{
	Sound::Sound () :
		m_speaker(IO.GetRef16(Io::SOUND1CNT_L), IO.GetRef16(Io::SOUND1CNT_H),
				IO.GetRef16(Io::SOUND1CNT_X),
				IO.GetRef16(Io::SOUND2CNT_L), IO.GetRef16(Io::SOUND2CNT_H),
				IO.GetRef16(Io::SOUND4CNT_L), IO.GetRef16(Io::SOUND4CNT_H),
				IO.GetRef16(Io::SOUNDCNT_L), IO.GetRef16(Io::SOUNDCNT_H),
				IO.GetRef16(Io::SOUNDCNT_X), IO.GetRef16(Io::SOUNDBIAS)),
		m_fATimer(0),
		m_fBTimer(0)
	{
	}

	void Sound::Reset ()
	{
		m_fATimer = m_fBTimer = 0;
		m_speaker.Reset();
	}

	void Sound::UpdateCntH1 (uint8_t val)
	{
		m_fATimer = (val & (0x1 << 10)) ? 1 : 0;
		m_fBTimer = (val & (0x1 << 14)) ? 1 : 0;
		if (val & (0x1 << 3))
			m_speaker.ResetFifoA();
		if (val & (0x1 << 7))
			m_speaker.ResetFifoB();
	}

	void Sound::TimerOverflow (uint8_t timernum)
	{
		// both fifo may be triggered by the same timer
		if (m_fATimer == timernum)
			TimerOverflowA();
		if (m_fBTimer == timernum)
			TimerOverflowB();
	}

	inline void Sound::TimerOverflowA ()
	{
		if (m_speaker.GetSizeA() <= 16)
		{
			DMA.Check(1, Dma::Special);
			if (m_speaker.GetSizeA() <= 16)
			{
				int8_t buf[16] = {0};
				m_speaker.FillFifoA(buf);
			}
		}
		m_speaker.NextSampleA ();
	}

	inline void Sound::TimerOverflowB ()
	{
		if (m_speaker.GetSizeB() <= 16)
		{
			DMA.Check(2, Dma::Special);
			if (m_speaker.GetSizeB() <= 16)
			{
				int8_t buf[16] = {0};
				m_speaker.FillFifoB(buf);
			}
		}
		m_speaker.NextSampleB ();
	}

	bool Sound::SaveState (std::ostream& stream)
	{
		SS_WRITE_VAR(m_fATimer);
		SS_WRITE_VAR(m_fBTimer);

		if (!m_speaker.SaveState(stream))
			return false;

		return true;
	}

	bool Sound::LoadState (std::istream& stream)
	{
		SS_READ_VAR(m_fATimer);
		SS_READ_VAR(m_fBTimer);

		if (!m_speaker.LoadState(stream))
			return false;

		return true;
	}
}
