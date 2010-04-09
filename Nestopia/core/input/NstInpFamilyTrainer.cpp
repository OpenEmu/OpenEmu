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
#include "NstInpFamilyTrainer.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			FamilyTrainer::FamilyTrainer(const Cpu& c)
			: Device(c,Api::Input::FAMILYTRAINER)
			{
				FamilyTrainer::Reset();
			}

			void FamilyTrainer::Reset()
			{
				output = 0;
				state = ~0U;
			}

			void FamilyTrainer::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'F','T'>::R(0,0,id) ).Write8( output ).End();
			}

			void FamilyTrainer::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'F','T'>::V)
					output = loader.Read8() & 0x1E;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void FamilyTrainer::Poll()
			{
				Controllers::FamilyTrainer& trainer = input->familyTrainer;
				input = NULL;

				if (Controllers::FamilyTrainer::callback( trainer ))
				{
					static const word lut[Controllers::FamilyTrainer::NUM_SIDE_A_BUTTONS] =
					{
						1U <<  1 ^ 0x1FFF,
						1U <<  2 ^ 0x1FFF,
						1U <<  3 ^ 0x1FFF,
						1U <<  4 ^ 0x1FFF,
						1U <<  5 ^ 0x1FFF,
						1U <<  6 ^ 0x1FFF,
						1U <<  7 ^ 0x1FFF,
						1U <<  8 ^ 0x1FFF,
						1U <<  9 ^ 0x1FFF,
						1U << 10 ^ 0x1FFF,
						1U << 11 ^ 0x1FFF,
						1U << 12 ^ 0x1FFF
					};

					uint bits = ~0U;

					for (uint i=0; i < Controllers::FamilyTrainer::NUM_SIDE_A_BUTTONS; ++i)
					{
						if (trainer.sideA[i])
							bits &= lut[i];
					}

					static const byte index[Controllers::FamilyTrainer::NUM_SIDE_B_BUTTONS] =
					{
						2,1,7,6,5,4,10,9
					};

					for (uint i=0; i < Controllers::FamilyTrainer::NUM_SIDE_B_BUTTONS; ++i)
					{
						if (trainer.sideB[i])
							bits &= lut[index[i]];
					}

					state = bits;
				}
			}

			uint FamilyTrainer::Peek(uint port)
			{
				return port == 1 ? output : 0;
			}

			void FamilyTrainer::Poke(const uint data)
			{
				if (input)
					Poll();

				if ((data & 0x1) == 0)
				{
					output = state >> 8 & 0x1E;
				}
				else if ((data & 0x2) == 0)
				{
					output = state >> 4 & 0x1E;
				}
				else if ((data & 0x4) == 0)
				{
					output = state >> 0 & 0x1E;
				}
				else
				{
					output = 0;
				}
			}
		}
	}
}
