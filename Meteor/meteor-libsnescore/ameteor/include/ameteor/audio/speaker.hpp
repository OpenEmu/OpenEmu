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

#ifndef __AUDIO_SPEAKER_H__
#define __AUDIO_SPEAKER_H__

#include <stdint.h>
#include <istream>
#include <ostream>
#include "signal.hpp"
#include "sound1.hpp"
#include "sound2.hpp"
#include "sound4.hpp"
#include "dsound.hpp"

namespace AMeteor
{
	namespace Audio
	{
		class Speaker
		{
			public :
				typedef syg::slot1<void, const int16_t*> FrameSlot;

				Speaker (uint16_t& cnt1l, uint16_t& cnt1h, uint16_t& cnt1x,
						uint16_t& cnt2l, uint16_t& cnt2h,
						uint16_t& cnt4l, uint16_t& cnt4h,
						uint16_t& cntl, uint16_t& cnth, uint16_t& cntx, uint16_t& bias);
				~Speaker ();

				inline void SetFrameSlot (const FrameSlot& slot);

				void Reset ();

				inline void ResetSound1 ();
				inline void ResetSound2 ();
				inline void ResetSound4 ();
				inline void ResetSound1Envelope ();
				inline void ResetSound2Envelope ();
				inline void ResetSound4Envelope ();

				inline void FillFifoA (int8_t* buffer);
				inline void FillFifoB (int8_t* buffer);
				inline void FillFifoA (int8_t sample);
				inline void FillFifoB (int8_t sample);

				inline void ResetFifoA ();
				inline void ResetFifoB ();

				inline void NextSampleA ();
				inline void NextSampleB ();

				inline uint8_t GetSizeA();
				inline uint8_t GetSizeB();

				void SoundTick ();

				bool SaveState (std::ostream& stream);
				bool LoadState (std::istream& stream);

			private :
				Sound1 m_sound1;
				Sound2 m_sound2;
				Sound4 m_sound4;
				DSound m_dsa, m_dsb;
				uint16_t &m_cntl, &m_cnth, &m_cntx, &m_bias;

				FrameSlot m_sig_frame;

				int16_t MixSample (uint16_t cntl, uint8_t cnth);
		};

		inline void Speaker::SetFrameSlot (const FrameSlot& slot)
		{
			m_sig_frame = slot;
		}

		inline void Speaker::ResetSound1 ()
		{
			m_sound1.ResetSound ();
		}

		inline void Speaker::ResetSound2 ()
		{
			m_sound2.ResetSound ();
		}

		inline void Speaker::ResetSound4 ()
		{
			m_sound4.ResetSound ();
		}

		inline void Speaker::ResetSound1Envelope ()
		{
			m_sound1.ResetEnvelope();
		}

		inline void Speaker::ResetSound2Envelope ()
		{
			m_sound2.ResetEnvelope();
		}

		inline void Speaker::ResetSound4Envelope ()
		{
			m_sound4.ResetEnvelope();
		}

		inline void Speaker::FillFifoA (int8_t* buffer)
		{
			m_dsa.FillFifo(buffer);
		}

		inline void Speaker::FillFifoB (int8_t* buffer)
		{
			m_dsb.FillFifo(buffer);
		}

		inline void Speaker::FillFifoA (int8_t sample)
		{
			m_dsa.FillFifo(sample);
		}

		inline void Speaker::FillFifoB (int8_t sample)
		{
			m_dsb.FillFifo(sample);
		}

		inline void Speaker::ResetFifoA ()
		{
			m_dsa.Reset();
		}

		inline void Speaker::ResetFifoB ()
		{
			m_dsb.Reset();
		}

		inline void Speaker::NextSampleA ()
		{
			m_dsa.NextSample();
		}

		inline void Speaker::NextSampleB ()
		{
			m_dsb.NextSample();
		}

		inline uint8_t Speaker::GetSizeA()
		{
			return m_dsa.GetSize();
		}

		inline uint8_t Speaker::GetSizeB()
		{
			return m_dsb.GetSize();
		}
	}
}

#endif
