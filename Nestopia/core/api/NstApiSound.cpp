////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include "../NstMachine.hpp"
#include "NstApiSound.hpp"

namespace Nes
{
	#ifdef NST_MSVC_OPTIMIZE
	#pragma optimize("s", on)
	#endif

	namespace Core
	{
		namespace Sound
		{
			Output::Locker Output::lockCallback;
			Output::Unlocker Output::unlockCallback;
		}
	}

	namespace Api
	{
		NST_COMPILE_ASSERT
		(
			Sound::CHANNEL_SQUARE1  == 1U << Core::Apu::Channel::APU_SQUARE1  &&
			Sound::CHANNEL_SQUARE2  == 1U << Core::Apu::Channel::APU_SQUARE2  &&
			Sound::CHANNEL_TRIANGLE == 1U << Core::Apu::Channel::APU_TRIANGLE &&
			Sound::CHANNEL_NOISE    == 1U << Core::Apu::Channel::APU_NOISE    &&
			Sound::CHANNEL_DPCM     == 1U << Core::Apu::Channel::APU_DPCM     &&
			Sound::CHANNEL_FDS      == 1U << Core::Apu::Channel::EXT_FDS      &&
			Sound::CHANNEL_MMC5     == 1U << Core::Apu::Channel::EXT_MMC5     &&
			Sound::CHANNEL_VRC6     == 1U << Core::Apu::Channel::EXT_VRC6     &&
			Sound::CHANNEL_VRC7     == 1U << Core::Apu::Channel::EXT_VRC7     &&
			Sound::CHANNEL_N163     == 1U << Core::Apu::Channel::EXT_N163     &&
			Sound::CHANNEL_S5B      == 1U << Core::Apu::Channel::EXT_S5B
		);

		Result Sound::SetSampleRate(ulong rate) throw()
		{
			return emulator.cpu.GetApu().SetSampleRate( rate );
		}

		Result Sound::SetSampleBits(uint bits) throw()
		{
			return emulator.cpu.GetApu().SetSampleBits( bits );
		}

		Result Sound::SetVolume(uint channels,uint volume) throw()
		{
			return emulator.cpu.GetApu().SetVolume( channels, volume );
		}

		Result Sound::SetSpeed(uint speed) throw()
		{
			return emulator.cpu.GetApu().SetSpeed( speed );
		}

		void Sound::SetAutoTranspose(bool enable) throw()
		{
			emulator.cpu.GetApu().SetAutoTranspose( enable );
		}

		void Sound::SetSpeaker(Speaker speaker) throw()
		{
			return emulator.cpu.GetApu().EnableStereo( speaker == SPEAKER_STEREO );
		}

		ulong Sound::GetSampleRate() const throw()
		{
			return emulator.cpu.GetApu().GetSampleRate();
		}

		uint Sound::GetSampleBits() const throw()
		{
			return emulator.cpu.GetApu().GetSampleBits();
		}

		uint Sound::GetVolume(uint channel) const throw()
		{
			return emulator.cpu.GetApu().GetVolume( channel );
		}

		uint Sound::GetSpeed() const throw()
		{
			return emulator.cpu.GetApu().GetSpeed();
		}

		bool Sound::IsAudible() const throw()
		{
			return emulator.cpu.GetApu().IsAudible();
		}

		bool Sound::IsAutoTransposing() const throw()
		{
			return emulator.cpu.GetApu().IsAutoTransposing();
		}

		Sound::Speaker Sound::GetSpeaker() const throw()
		{
			return emulator.cpu.GetApu().InStereo() ? SPEAKER_STEREO : SPEAKER_MONO;
		}

		void Sound::EmptyBuffer() throw()
		{
			return emulator.cpu.GetApu().ClearBuffers();
		}
	}

	#ifdef NST_MSVC_OPTIMIZE
	#pragma optimize("", on)
	#endif
}

