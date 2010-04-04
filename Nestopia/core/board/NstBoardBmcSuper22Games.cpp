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
#include "NstBoardBmcSuper22Games.hpp"
#include "../NstCrc32.hpp"

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

				class Super22Games::CartSwitches : public DipSwitches
				{
					enum Type
					{
						SUPER_X_GAMES = 0,
						SUPER_22_GAMES = 0xB27414ED
					};

					uint hiPrg;
					const Type type;

					explicit CartSwitches(Type t)
					: hiPrg(0x00), type(t) {}

				public:

					static CartSwitches* Create(const Context& c)
					{
						if (c.prg.Size() == SIZE_1024K)
							return new CartSwitches( Crc32::Compute(c.prg.Mem(),c.prg.Size()) == SUPER_22_GAMES ? SUPER_22_GAMES : SUPER_X_GAMES );
						else
							return NULL;
					}

					void EnableHiPrg(bool enable)
					{
						hiPrg = enable ? 0x20 : 0x00;
					}

					uint GetHiPrg() const
					{
						return hiPrg;
					}

				private:

					uint GetValue(uint) const
					{
						return hiPrg ? 1 : 0;
					}

					void SetValue(uint,uint value)
					{
						hiPrg = value ? 0x20 : 0x00;
					}

					uint NumDips() const
					{
						return 1;
					}

					uint NumValues(uint) const
					{
						return 2;
					}

					cstring GetDipName(uint) const
					{
						return "Mode";
					}

					cstring GetValueName(uint,uint i) const
					{
						return i ? (type == SUPER_22_GAMES ? "20-in-1" : "2") :
                                   (type == SUPER_22_GAMES ? "22-in-1" : "1");
					}
				};

				Super22Games::Super22Games(const Context& c)
				: Board(c), cartSwitches(CartSwitches::Create(c)) {}

				Super22Games::~Super22Games()
				{
					delete cartSwitches;
				}

				Super22Games::Device Super22Games::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void Super22Games::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &Super22Games::Poke_8000 );

					NES_DO_POKE(8000,0x8000,0x00);
				}

				void Super22Games::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','2','2'>::V) );

					if (baseChunk == AsciiId<'B','2','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								NST_VERIFY( cartSwitches );

								if (cartSwitches)
									cartSwitches->EnableHiPrg( state.Read8() & 0x1 );
							}

							state.End();
						}
					}
				}

				void Super22Games::SubSave(State::Saver& state) const
				{
					if (cartSwitches)
						state.Begin( AsciiId<'B','2','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( cartSwitches->GetHiPrg() ? 0x1 : 0x0 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Super22Games,8000)
				{
					const uint hiPrg = cartSwitches ? cartSwitches->GetHiPrg() : 0x00;

					if (data & 0x20)
						prg.SwapBanks<SIZE_16K,0x0000>( hiPrg | (data & 0x1F), hiPrg | (data & 0x1F) );
					else
						prg.SwapBank<SIZE_32K,0x0000>( (hiPrg >> 1) | (data >> 1 & 0xF) );

					static const byte lut[4][4] =
					{
						{0,0,0,0},
						{0,1,0,1},
						{0,0,1,1},
						{1,1,1,1}
					};

					ppu.SetMirroring( lut[data >> 6] );
				}
			}
		}
	}
}
