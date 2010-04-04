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
#include "NstBoardBmcVt5201.hpp"
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

				Vt5201::CartSwitches::CartSwitches(const Context& c)
				: mode(0), type(DetectType(c)) {}

				Vt5201::CartSwitches::Type Vt5201::CartSwitches::DetectType(const Context& c)
				{
					switch (const dword crc = Crc32::Compute(c.prg.Mem(),c.prg.Size()))
					{
						case VT5201_6IN1:
						case VT5201_12IN1:
						case VT5201_28IN1:
						case VT5201_54IN1:
						case VT5201_55IN1:
						case VT5201_65IN1:

                             return static_cast<Type>(crc);
					}

					return VT5201_XIN1;
				}

				void Vt5201::CartSwitches::SetMode(uint value)
				{
					mode = value;
				}

				uint Vt5201::CartSwitches::GetMode() const
				{
					return mode;
				}

				uint Vt5201::CartSwitches::GetValue(uint) const
				{
					return mode;
				}

				void Vt5201::CartSwitches::SetValue(uint,uint value)
				{
					mode = value;
				}

				uint Vt5201::CartSwitches::NumDips() const
				{
					return 1;
				}

				uint Vt5201::CartSwitches::NumValues(uint) const
				{
					return 4;
				}

				cstring Vt5201::CartSwitches::GetDipName(uint) const
				{
					return "Mode";
				}

				cstring Vt5201::CartSwitches::GetValueName(uint,uint i) const
				{
					static cstring const names[7][4] =
					{
						{ "1",       "2",       "3",       "4"        },
						{ "6-in-1",  "15-in-1", "35-in-1", "43-in-1"  },
						{ "12-in-1", "66-in-1", "77-in-1", "88-in-1"  },
						{ "28-in-1", "46-in-1", "63-in-1", "118-in-1" },
						{ "54-in-1", "64-in-1", "74-in-1", "84-in-1"  },
						{ "55-in-1", "65-in-1", "75-in-1", "85-in-1"  },
						{ "65-in-1", "75-in-1", "85-in-1", "95-in-1"  }
					};

					uint id = 0;

					switch (type)
					{
						case VT5201_6IN1:  id = 1; break;
						case VT5201_12IN1: id = 2; break;
						case VT5201_28IN1: id = 3; break;
						case VT5201_54IN1: id = 4; break;
						case VT5201_55IN1: id = 5; break;
						case VT5201_65IN1: id = 6; break;
					}

					return names[id][i];
				}

				Vt5201::Vt5201(const Context& c)
				: Board(c), cartSwitches(c) {}

				Vt5201::Device Vt5201::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return &cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void Vt5201::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &Vt5201::Peek_8000, &Vt5201::Poke_8000 );

					if (hard)
						NES_DO_POKE(8000,0x8000,0x00);
				}

				void Vt5201::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','V','T'>::V) );

					if (baseChunk == AsciiId<'B','V','T'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								const uint data = state.Read8();

								cartMode = data << 1 & 0x100;
								cartSwitches.SetMode( data & 0x3 );
							}

							state.End();
						}
					}
				}

				void Vt5201::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','V','T'>::V ) .Begin( AsciiId<'R','E','G'>::V ).Write8( cartSwitches.GetMode() | cartMode >> 1 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Vt5201,8000)
				{
					return !cartMode ? prg.Peek( address - 0x8000 ) : cartSwitches.GetMode();
				}

				NES_POKE_A(Vt5201,8000)
				{
					cartMode = address & 0x100;
					ppu.SetMirroring( (address & 0x8) ? Ppu::NMT_H : Ppu::NMT_V );
					prg.SwapBanks<SIZE_16K,0x0000>( (address >> 4) & ~(~address >> 7 & 0x1), (address >> 4) | (~address >> 7 & 0x1) );
					chr.SwapBank<SIZE_8K,0x0000>( address );
				}
			}
		}
	}
}
