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
#include "NstInpKonamiHyperShot.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			KonamiHyperShot::KonamiHyperShot(const Cpu& c)
			: Device(c,Api::Input::KONAMIHYPERSHOT)
			{
				KonamiHyperShot::Reset();
			}

			void KonamiHyperShot::Reset()
			{
				strobe = 0;
				state = 0;
			}

			void KonamiHyperShot::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'H','S'>::R(0,0,id) ).Write8( strobe ).End();
			}

			void KonamiHyperShot::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'H','S'>::V)
					strobe = loader.Read8() & 0x1;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void KonamiHyperShot::Poke(const uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe && input)
				{
					Controllers::KonamiHyperShot::callback( input->konamiHyperShot );
					state = input->konamiHyperShot.buttons & 0x1E;
					input = NULL;
				}
			}

			uint KonamiHyperShot::Peek(const uint port)
			{
				return port ? state : 0;
			}
		}
	}
}
