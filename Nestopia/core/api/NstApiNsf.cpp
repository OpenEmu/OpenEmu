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
#include "../NstNsf.hpp"
#include "NstApiMachine.hpp"
#include "NstApiNsf.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Nsf::EventCaller Nsf::eventCallback;

		const char* Nsf::GetName() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetName();

			return "";
		}

		const char* Nsf::GetArtist() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetArtist();

			return "";
		}

		const char* Nsf::GetCopyright() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetCopyright();

			return "";
		}

		uint Nsf::GetChips() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetChips();

			return 0;
		}

		Nsf::TuneMode Nsf::GetMode() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<TuneMode>(static_cast<const Core::Nsf*>(emulator.image)->GetTuneMode());

			return TUNE_MODE_NTSC;
		}

		uint Nsf::GetNumSongs() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->NumSongs();

			return 0;
		}

		int Nsf::GetCurrentSong() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->CurrentSong();

			return NO_SONG;
		}

		int Nsf::GetStartingSong() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->StartingSong();

			return NO_SONG;
		}

		uint Nsf::GetInitAddress() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetInitAddress();

			return 0x0000;
		}

		uint Nsf::GetLoadAddress() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetLoadAddress();

			return 0x0000;
		}

		uint Nsf::GetPlayAddress() const throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<const Core::Nsf*>(emulator.image)->GetPlayAddress();

			return 0x0000;
		}

		Result Nsf::SelectSong(uint song) throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<Core::Nsf*>(emulator.image)->SelectSong( song );

			return RESULT_ERR_NOT_READY;
		}

		Result Nsf::PlaySong() throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<Core::Nsf*>(emulator.image)->PlaySong();

			return RESULT_ERR_NOT_READY;
		}

		Result Nsf::StopSong() throw()
		{
			if (emulator.Is(Machine::SOUND))
				return static_cast<Core::Nsf*>(emulator.image)->StopSong();

			return RESULT_ERR_NOT_READY;
		}

		Result Nsf::SelectNextSong() throw()
		{
			if (emulator.Is(Machine::SOUND))
			{
				return static_cast<Core::Nsf*>(emulator.image)->SelectSong
				(
					static_cast<const Core::Nsf*>(emulator.image)->CurrentSong() + 1U
				);
			}

			return RESULT_ERR_NOT_READY;
		}

		Result Nsf::SelectPrevSong() throw()
		{
			if (emulator.Is(Machine::SOUND))
			{
				return static_cast<Core::Nsf*>(emulator.image)->SelectSong
				(
					static_cast<const Core::Nsf*>(emulator.image)->CurrentSong() - 1U
				);
			}

			return RESULT_ERR_NOT_READY;
		}

		bool Nsf::IsPlaying() const throw()
		{
			return emulator.Is(Machine::SOUND) && static_cast<Core::Nsf*>(emulator.image)->IsPlaying();
		}

		bool Nsf::UsesBankSwitching() const throw()
		{
			return emulator.Is(Machine::SOUND) && static_cast<Core::Nsf*>(emulator.image)->UsesBankSwitching();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
