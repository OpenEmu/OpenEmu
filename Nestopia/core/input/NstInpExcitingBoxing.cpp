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

#include "NstInpDevice.hpp"
#include "NstInpExcitingBoxing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			ExcitingBoxing::ExcitingBoxing(const Cpu& c)
			: Device(c,Api::Input::EXCITINGBOXING)
			{
				ExcitingBoxing::Reset();
			}

			void ExcitingBoxing::Reset()
			{
				state = 0x1E;
			}

			void ExcitingBoxing::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'E','B'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void ExcitingBoxing::Poke(const uint data)
			{
				if (input)
				{
					Controllers::ExcitingBoxing::callback( input->excitingBoxing, data & 0x2 );
					state = ~input->excitingBoxing.buttons & 0x1E;
				}
				else
				{
					state = 0x1E;
				}
			}

			uint ExcitingBoxing::Peek(const uint port)
			{
				return port ? state : 0;
			}
		}
	}
}
