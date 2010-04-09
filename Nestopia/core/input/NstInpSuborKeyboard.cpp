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
#include "NstInpSuborKeyboard.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			SuborKeyboard::SuborKeyboard(const Cpu& c)
			: Device(c,Api::Input::SUBORKEYBOARD)
			{
				SuborKeyboard::Reset();
			}

			void SuborKeyboard::Reset()
			{
				scan = 0;
				mode = 0;
			}

			void SuborKeyboard::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'S','K'>::R(0,0,id) ).Write8( mode | (scan << 1) ).End();
			}

			void SuborKeyboard::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'S','K'>::V)
				{
                     const uint data = loader.Read8();

                     mode = data & 0x1;
                     scan = data >> 1 & 0xF;

                     if (scan > 12)
                         scan = 0;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void SuborKeyboard::Poke(const uint data)
			{
				if (data & COMMAND_KEY)
				{
					const uint out = data & COMMAND_SCAN;

					if (mode && !out && ++scan > 12)
						scan = 0;

					mode = out >> 1;

					if (data & COMMAND_RESET)
						scan = 0;
				}
			}

			uint SuborKeyboard::Peek(uint port)
			{
				if (port == 0)
				{
					return 0x00;
				}
				else if (input && scan < 10)
				{
					Controllers::SuborKeyboard::callback( input->suborKeyboard, scan, mode );
					return ~uint(input->suborKeyboard.parts[scan]) & 0x1E;
				}
				else
				{
					return 0x1E;
				}
			}
		}
	}
}
