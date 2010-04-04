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

#include <new>
#include "../NstMachine.hpp"
#include "NstApiMachine.hpp"
#include "NstApiRewinder.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Rewinder::StateCaller Rewinder::stateCallback;

		Result Rewinder::Enable(bool enable) throw()
		{
			try
			{
				return emulator.tracker.EnableRewinder( enable ? &emulator : NULL );
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		bool Rewinder::IsEnabled() const throw()
		{
			return emulator.tracker.IsRewinderEnabled();
		}

		bool Rewinder::IsSoundEnabled() const throw()
		{
			return emulator.tracker.IsRewinderSoundEnabled();
		}

		void Rewinder::EnableSound(bool enable) throw()
		{
			emulator.tracker.EnableRewinderSound( enable );
		}

		Rewinder::Direction Rewinder::GetDirection() const throw()
		{
			return emulator.tracker.IsRewinding() ? BACKWARD : FORWARD;
		}

		Result Rewinder::SetDirection(Direction dir) throw()
		{
			if (emulator.Is(Machine::GAME,Machine::ON))
			{
				if (dir == BACKWARD)
					return emulator.tracker.StartRewinding();
				else
					return emulator.tracker.StopRewinding();
			}

			return RESULT_ERR_NOT_READY;
		}

		void Rewinder::Reset() throw()
		{
			if (emulator.Is(Machine::GAME,Machine::ON))
				emulator.tracker.ResetRewinder();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
