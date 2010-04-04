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
#include "NstBoardBmc800in1.hpp"
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

				Game800in1::CartSwitches::CartSwitches(const Context& c)
				:
				type (DetectType(c)),
				mode (type == GAME_800IN1 ? 0x6 : type == GAME_70IN1 ? 0xD : 0x0)
				{}

				Game800in1::CartSwitches::Type Game800in1::CartSwitches::DetectType(const Context& c)
				{
					switch (Crc32::Compute(c.prg.Mem(),c.prg.Size()))
					{
						case GAME_800IN1: return GAME_800IN1;
						case GAME_70IN1:  return GAME_70IN1;
					}

					return GAME_XIN1;
				}

				inline void Game800in1::CartSwitches::SetMode(uint value)
				{
					mode = value;
				}

				inline uint Game800in1::CartSwitches::GetMode() const
				{
					return mode;
				}

				uint Game800in1::CartSwitches::GetValue(uint) const
				{
					return mode;
				}

				void Game800in1::CartSwitches::SetValue(uint,uint value)
				{
					mode = value;
				}

				uint Game800in1::CartSwitches::NumDips() const
				{
					return 1;
				}

				uint Game800in1::CartSwitches::NumValues(uint) const
				{
					return 16;
				}

				cstring Game800in1::CartSwitches::GetDipName(uint) const
				{
					return "Mode";
				}

				cstring Game800in1::CartSwitches::GetValueName(uint,uint i) const
				{
					static cstring const names[3][16] =
					{
						{
							"1",  "2",  "3",  "4",
							"5",  "6",  "7",  "8",
							"9",  "10", "11", "12",
							"13", "14", "15", "16"
						},
						{
							"76-in-1",   "150-in-1",     "168-in-1",     "190-in-1",
							"400-in-1",  "500-in-1",     "800-in-1",     "1200-in-1",
							"2000-in-1", "5000-in-1",    "300-in-1",     "1500-in-1",
							"3000-in-1", "1010000-in-1", "5010000-in-1", "10000000-in-1"
						},
						{
							"4-in-1",  "5-in-1",  "6-in-1",  "77-in-1",
							"22-in-1", "38-in-1", "44-in-1", "46-in-1",
							"52-in-1", "55-in-1", "63-in-1", "66-in-1",
							"68-in-1", "70-in-1", "32-in-1", "80-in-1"
						}
					};

					return names[type == GAME_70IN1 ? 2 : type == GAME_800IN1 ? 1 : 0][i];
				}

				Game800in1::Game800in1(const Context& c)
				: Board(c), cartSwitches(c) {}

				Game800in1::Device Game800in1::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return &cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void Game800in1::SubReset(bool)
				{
					mode = 0x0;

					Map( 0x8000U, 0xFFFFU, &Game800in1::Peek_8000, &Game800in1::Poke_8000 );

					NES_DO_POKE(8000,0x8000,0x00);
					NES_DO_POKE(8000,0xC000,0x00);
				}

				void Game800in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','8','1'>::V) );

					if (baseChunk == AsciiId<'B','8','1'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								const uint data = state.Read8();
								mode = data >> 4 & 0x1;
								cartSwitches.SetMode( data & 0xF );
							}

							state.End();
						}
					}
				}

				void Game800in1::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','8','1'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( cartSwitches.GetMode() | (mode << 4) ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Game800in1,8000)
				{
					return prg.Peek( mode ? (address & 0x7FF0) | cartSwitches.GetMode() : address - 0x8000 );
				}

				NES_POKE_A(Game800in1,8000)
				{
					uint banks[2] =
					{
						prg.GetBank<SIZE_16K,0x0000>(),
						prg.GetBank<SIZE_16K,0x4000>()
					};

					if (address < 0xC000)
					{
						ppu.SetMirroring( (address & 0x20) ? Ppu::NMT_H : Ppu::NMT_V );

						if (chr.Source().GetType() == Ram::RAM)
						{
							banks[0] = (banks[0] & 0x7) | (address << 3 & 0x38);
							banks[1] = (banks[1] & 0x7) | (address << 3 & 0x38);
						}
						else
						{
							chr.SwapBank<SIZE_8K,0x0000>( address & 0x7 );
							return;
						}
					}
					else switch (address & 0x30)
					{
						case 0x00: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x7; break;
						case 0x10: mode = 0x1; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x7; break;
						case 0x20: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x6); banks[1] = banks[0] | 0x1; break;
						case 0x30: mode = 0x0; banks[0] = (banks[0] & 0x38) | (address & 0x7); banks[1] = banks[0] | 0x0; break;
					}

					prg.SwapBanks<SIZE_16K,0x0000>( banks[0], banks[1] );
				}
			}
		}
	}
}
