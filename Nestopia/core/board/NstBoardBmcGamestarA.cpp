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
#include "NstBoardBmcGamestarA.hpp"
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

				class GamestarA::CartSwitches : public DipSwitches
				{
					enum Type
					{
						GKA_6IN1A   = 0x2A80F48F,
						GKA_6IN1B   = 0xF274BF1F,
						GKA_47IN1   = 0x8DA67F2D,
						GKA_54IN1   = 0x38EB6D5A,
						GKA_103IN1  = 0xB1F9BD94
					};

					uint mode;
					const Type type;

					explicit CartSwitches(Type t)
					: mode(0), type(t) {}

				public:

					static CartSwitches* Create(const Context& c)
					{
						switch (const dword crc = Crc32::Compute(c.prg.Mem(),c.prg.Size()))
						{
							case GKA_6IN1A:
							case GKA_6IN1B:
							case GKA_47IN1:
							case GKA_54IN1:
							case GKA_103IN1:

								return new CartSwitches( static_cast<Type>(crc) );
						}

						return NULL;
					}

					void SetMode(uint value)
					{
						mode = value & 0x3;
					}

					uint GetMode() const
					{
						return mode;
					}

				private:

					uint GetValue(uint) const
					{
						return mode;
					}

					void SetValue(uint,uint value)
					{
						mode = value;
					}

					uint NumDips() const
					{
						return 1;
					}

					uint NumValues(uint) const
					{
						return 4;
					}

					cstring GetDipName(uint) const
					{
						return "Mode";
					}

					cstring GetValueName(uint,uint i) const
					{
						static cstring const names[4][4] =
						{
							{ "6-in-1",   "2-in-1 (1)", "2-in-1 (2)", "2-in-1 (3)" },
							{ "47-in-1",  "57-in-1",    "67-in-1",    "77-in-1"    },
							{ "54-in-1",  "74-in-1",    "84-in-1",    "94-in-1"    },
							{ "103-in-1", "105-in-1",   "106-in-1",   "109-in-1"   }
						};

						switch (type)
						{
							case GKA_6IN1A:
							case GKA_6IN1B:
							default:

								return names[0][i];

							case GKA_47IN1:

								return names[1][i];

							case GKA_54IN1:

								return names[2][i];

							case GKA_103IN1:

								return names[3][i];
						}
					}
				};

				GamestarA::GamestarA(const Context& c)
				: Board(c), cartSwitches(CartSwitches::Create(c)) {}

				GamestarA::~GamestarA()
				{
					delete cartSwitches;
				}

				GamestarA::Device GamestarA::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES && cartSwitches)
						return cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				void GamestarA::SubReset(const bool hard)
				{
					Map( 0x6000U, &GamestarA::Peek_6000 );

					for (uint i=0x0000; i < 0x8000; i += 0x1000)
					{
						Map( 0x8000+i, 0x87FF+i, &GamestarA::Poke_8000 );
						Map( 0x8800+i, 0x8FFF+i, &GamestarA::Poke_8800 );
					}

					if (hard)
					{
						regs[0] = 0;
						regs[1] = 0;

						NES_DO_POKE(8800,0x8800,0x00);
					}
				}

				void GamestarA::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'B','G','A'>::V) );

					if (baseChunk == AsciiId<'B','G','A'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								regs[0] = data[1];
								regs[1] = data[2];

								if (cartSwitches)
									cartSwitches->SetMode( data[0] );
							}

							state.End();
						}
					}
				}

				void GamestarA::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						cartSwitches ? cartSwitches->GetMode() : 0,
						regs[0],
						regs[1]
					};

					state.Begin( AsciiId<'B','G','A'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK(GamestarA,6000)
				{
					return cartSwitches ? cartSwitches->GetMode() : 0x00;
				}

				void GamestarA::UpdateChr() const
				{
					chr.SwapBank<SIZE_8K,0x0000>( (regs[0] >> 1 & 0x8) | (regs[0] & 0x7) | (regs[1] & 0x3) );
				}

				NES_POKE_D(GamestarA,8000)
				{
					regs[1] = data;
					ppu.Update();
					UpdateChr();
				}

				NES_POKE_D(GamestarA,8800)
				{
					regs[0] = data;
					prg.SwapBanks<SIZE_16K,0x0000>( (data >> 5) & ~(data >> 7), (data >> 5) | (data >> 7) );
					ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_H : Ppu::NMT_V );
					UpdateChr();
				}
			}
		}
	}
}
