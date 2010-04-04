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

#include "NstBoard.hpp"
#include "../NstDipSwitches.hpp"
#include "NstBoardBmc8157.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				B8157::CartSwitches::CartSwitches()
				: mode(0x100) {}

				inline void B8157::CartSwitches::SetMode(uint value)
				{
					mode = value ? 0x100 : 0x000;
				}

				inline uint B8157::CartSwitches::GetMode() const
				{
					return mode;
				}

				uint B8157::CartSwitches::GetValue(uint) const
				{
					return mode ? 0 : 1;
				}

				void B8157::CartSwitches::SetValue(uint,uint value)
				{
					mode = value ? 0x000 : 0x100;
				}

				uint B8157::CartSwitches::NumDips() const
				{
					return 1;
				}

				uint B8157::CartSwitches::NumValues(uint) const
				{
					return 2;
				}

				cstring B8157::CartSwitches::GetDipName(uint) const
				{
					return "Mode";
				}

				cstring B8157::CartSwitches::GetValueName(uint,uint i) const
				{
					return i ? "20-in-1" : "4-in-1";
				}

				B8157::Device B8157::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return &cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void B8157::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &B8157::Peek_8000, &B8157::Poke_8000 );

					trash = 0x00;

					if (hard)
						NES_DO_POKE(8000,0x8000,0x00);
				}

				void B8157::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','8','1'>::V) );

					if (baseChunk == AsciiId<'B','8','1'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								const uint data = state.Read8();
								trash = (data & 0x2) ? 0xFF : 0x00;
								cartSwitches.SetMode( data & 0x1 );
							}

							state.End();
						}
					}
				}

				void B8157::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','8','1'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( (cartSwitches.GetMode() ? 0x1U : 0x0U) | (trash ? 0x2U : 0x0U) ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(B8157,8000)
				{
					return prg.Peek( address - 0x8000 ) | trash;
				}

				NES_POKE_A(B8157,8000)
				{
					trash = (address & cartSwitches.GetMode()) ? 0xFF : 0x00;

					prg.SwapBanks<SIZE_16K,0x0000>
					(
						(address >> 2 & 0x18) | (address >> 2 & 0x7),
						(address >> 2 & 0x18) | ((address & 0x200) ? 0x7 : 0x0)
					);

					ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_H : Ppu::NMT_V );
				}
			}
		}
	}
}
