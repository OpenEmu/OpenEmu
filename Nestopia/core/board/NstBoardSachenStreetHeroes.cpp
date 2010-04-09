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
#include "NstBoardMmc3.hpp"
#include "NstBoardSachenStreetHeroes.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sachen
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				StreetHeroes::CartSwitches::CartSwitches()
				: region(0x00) {}

				inline void StreetHeroes::CartSwitches::SetRegion(uint value)
				{
					region = value ? 0xFF : 0x00;
				}

				inline uint StreetHeroes::CartSwitches::GetRegion() const
				{
					return region;
				}

				uint StreetHeroes::CartSwitches::GetValue(uint) const
				{
					return region ? 1 : 0;
				}

				void StreetHeroes::CartSwitches::SetValue(uint,uint value)
				{
					region = value ? 0xFF : 0x00;
				}

				uint StreetHeroes::CartSwitches::NumDips() const
				{
					return 1;
				}

				uint StreetHeroes::CartSwitches::NumValues(uint) const
				{
					return 2;
				}

				cstring StreetHeroes::CartSwitches::GetDipName(uint) const
				{
					return "Region";
				}

				cstring StreetHeroes::CartSwitches::GetValueName(uint,uint i) const
				{
					return i ? "Asia" : "US";
				}

				StreetHeroes::Device StreetHeroes::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return &cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void StreetHeroes::SubReset(const bool hard)
				{
					if (hard)
						exReg = 0;

					Mmc3::SubReset( hard );

					Map( 0x4100U, &StreetHeroes::Peek_4100, &StreetHeroes::Poke_4100 );
				}

				void StreetHeroes::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'S','S','H'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<2> data( state );

								exReg = data[1];
								cartSwitches.SetRegion( data[0] & 0x1 );
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void StreetHeroes::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[2] =
					{
						cartSwitches.GetRegion() ? 0x1 : 0x0,
						exReg
					};

					state.Begin( AsciiId<'S','S','H'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL StreetHeroes::UpdateChr(uint address,uint bank) const
				{
					if (!(exReg & 0x40))
					{
						chr.SwapBank<SIZE_1K>
						(
							address, exReg <<
							(
								address < 0x0800 ? 5 :
								address < 0x1000 ? 6 :
								address < 0x1800 ? 8 : 7
							)   & 0x100 | bank
						);
					}
				}

				NES_POKE_D(StreetHeroes,4100)
				{
					if (exReg != data)
					{
						exReg = data;

						if (exReg & 0x40)
							chr.Source(1).SwapBank<SIZE_8K,0x0000>(0);
						else
							Mmc3::UpdateChr();
					}
				}

				NES_PEEK(StreetHeroes,4100)
				{
					return cartSwitches.GetRegion();
				}
			}
		}
	}
}
