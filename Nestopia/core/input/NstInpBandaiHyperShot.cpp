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
#include "../NstPpu.hpp"
#include "NstInpZapper.hpp"
#include "NstInpBandaiHyperShot.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			BandaiHyperShot::BandaiHyperShot(const Cpu& c,Ppu& p)
			:
			Device (c,Api::Input::BANDAIHYPERSHOT),
			ppu    (p)
			{
				BandaiHyperShot::Reset();
			}

			void BandaiHyperShot::Reset()
			{
				pos = ~0U;
				fire = 0;
				move = 0;
			}

			void BandaiHyperShot::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'B','H'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint BandaiHyperShot::Poll()
			{
				if (input)
				{
					Controllers::BandaiHyperShot& bandaiHyperShot = input->bandaiHyperShot;
					input = NULL;

					if (Controllers::BandaiHyperShot::callback( bandaiHyperShot ))
					{
						fire = (bandaiHyperShot.fire ? 0x10 : 0x00);
						move = (bandaiHyperShot.move ? 0x02 : 0x00);

						if (bandaiHyperShot.y < Video::Screen::HEIGHT && bandaiHyperShot.x < Video::Screen::WIDTH)
							pos = bandaiHyperShot.y * Video::Screen::WIDTH + bandaiHyperShot.x;
						else
							pos = ~0U;
					}
				}

				if (pos < Video::Screen::WIDTH * Video::Screen::HEIGHT)
				{
					ppu.Update();

					uint pixel = ppu.GetPixelCycles();

					if (pos < pixel && pos >= pixel - PHOSPHOR_DECAY)
						return Zapper::GetLightMap( ppu.GetPixel( pos ) );
				}

				return 0;
			}

			uint BandaiHyperShot::Peek(uint)
			{
				uint data = (Poll() >= LIGHT_SENSOR ? 0x0 : 0x8);
				return data | fire | move;
			}
		}
	}
}
