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
#include "../NstClock.hpp"
#include "../NstSoundPlayer.hpp"
#include "NstBoardJalecoSs88006.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Jaleco
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Ss88006::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						mask = 0xFFFF;
						count = 0;
						latch = 0;
					}
				}

				Ss88006::Ss88006(const Context& c)
				:
				Board (c),
				irq   (*c.cpu),
				sound
				(
					Sound::Player::Create
					(
						*c.apu,
						c.chips,
						L"D7756C",
						board == Type::JALECO_JF24 ? Sound::Player::GAME_TERAO_NO_DOSUKOI_OOZUMOU :
						board == Type::JALECO_JF23 ? Sound::Player::GAME_MOERO_PRO_YAKYUU_88 :
						board == Type::JALECO_JF29 ? Sound::Player::GAME_MOERO_PRO_YAKYUU_88 :
						board == Type::JALECO_JF33 ? Sound::Player::GAME_MOERO_PRO_YAKYUU_88 :
                                                     Sound::Player::GAME_UNKNOWN,
						32
					)
				)
				{}

				Ss88006::~Ss88006()
				{
					Sound::Player::Destroy( sound );
				}

				void Ss88006::SubReset(const bool hard)
				{
					if (hard)
						wrk.Source().SetSecurity( false, false );

					reg = 0;
					irq.Reset( hard, hard ? false : irq.Connected() );

					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0x8000 + i, &Ss88006::Poke_8000 );
						Map( 0x8001 + i, &Ss88006::Poke_8001 );
						Map( 0x8002 + i, &Ss88006::Poke_8002 );
						Map( 0x8003 + i, &Ss88006::Poke_8003 );
						Map( 0x9000 + i, &Ss88006::Poke_9000 );
						Map( 0x9001 + i, &Ss88006::Poke_9001 );
						Map( 0x9002 + i, &Ss88006::Poke_9002 );
						Map( 0xA000 + i, &Ss88006::Poke_A000 );
						Map( 0xA001 + i, &Ss88006::Poke_A001 );
						Map( 0xA002 + i, &Ss88006::Poke_A002 );
						Map( 0xA003 + i, &Ss88006::Poke_A003 );
						Map( 0xB000 + i, &Ss88006::Poke_B000 );
						Map( 0xB001 + i, &Ss88006::Poke_B001 );
						Map( 0xB002 + i, &Ss88006::Poke_B002 );
						Map( 0xB003 + i, &Ss88006::Poke_B003 );
						Map( 0xC000 + i, &Ss88006::Poke_C000 );
						Map( 0xC001 + i, &Ss88006::Poke_C001 );
						Map( 0xC002 + i, &Ss88006::Poke_C002 );
						Map( 0xC003 + i, &Ss88006::Poke_C003 );
						Map( 0xD000 + i, &Ss88006::Poke_D000 );
						Map( 0xD001 + i, &Ss88006::Poke_D001 );
						Map( 0xD002 + i, &Ss88006::Poke_D002 );
						Map( 0xD003 + i, &Ss88006::Poke_D003 );
						Map( 0xE000 + i, &Ss88006::Poke_E000 );
						Map( 0xE001 + i, &Ss88006::Poke_E001 );
						Map( 0xE002 + i, &Ss88006::Poke_E002 );
						Map( 0xE003 + i, &Ss88006::Poke_E003 );
						Map( 0xF000 + i, &Ss88006::Poke_F000 );
						Map( 0xF001 + i, &Ss88006::Poke_F001 );
						Map( 0xF002 + i, NMT_SWAP_HV01    );

						if (sound)
							Map( 0xF003 + i, &Ss88006::Poke_F003 );
					}
				}

				void Ss88006::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'J','S','8'>::V) );

					if (sound)
						sound->Stop();

					if (baseChunk == AsciiId<'J','S','8'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<5> data( state );

									irq.Connect( data[0] & 0x1 );

									if      (data[0] & 0x8) irq.unit.mask = 0x000F;
									else if (data[0] & 0x4) irq.unit.mask = 0x00FF;
									else if (data[0] & 0x2) irq.unit.mask = 0x0FFF;
									else                    irq.unit.mask = 0xFFFF;

									irq.unit.latch = data[1] | data[2] << 8;
									irq.unit.count = data[3] | data[4] << 8;
									break;
								}

								case AsciiId<'R','E','G'>::V:

									NST_VERIFY( sound );
									reg = state.Read8();
									break;
							}

							state.End();
						}
					}
				}

				void Ss88006::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'J','S','8'>::V );

					const byte data[5] =
					{
						(irq.Connected() ? 0x1U : 0x0U) |
						(
							irq.unit.mask == 0x000F ? 0x8U :
							irq.unit.mask == 0x00FF ? 0x4U :
							irq.unit.mask == 0x0FFF ? 0x2U :
                                                      0x0U
						),
						irq.unit.latch & 0xFF,
						irq.unit.latch >> 8,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();

					if (sound)
						state.Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End();

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				template<uint MASK,uint SHIFT>
				void Ss88006::SwapPrg(const uint address,const uint data)
				{
					prg.SwapBank<SIZE_8K>( address, (prg.GetBank<SIZE_8K>(address) & MASK) | (data & 0xF) << SHIFT );
				}

				NES_POKE_D(Ss88006,8000) { SwapPrg<0xF0,0>( 0x0000, data ); }
				NES_POKE_D(Ss88006,8001) { SwapPrg<0x0F,4>( 0x0000, data ); }
				NES_POKE_D(Ss88006,8002) { SwapPrg<0xF0,0>( 0x2000, data ); }
				NES_POKE_D(Ss88006,8003) { SwapPrg<0x0F,4>( 0x2000, data ); }
				NES_POKE_D(Ss88006,9000) { SwapPrg<0xF0,0>( 0x4000, data ); }
				NES_POKE_D(Ss88006,9001) { SwapPrg<0x0F,4>( 0x4000, data ); }

				NES_POKE_D(Ss88006,9002)
				{
					NST_VERIFY( data == 0x3 || data == 0x0 );
					wrk.Source().SetSecurity( data & 0x1, data & 0x2 );
				}

				template<uint MASK,uint SHIFT>
				void Ss88006::SwapChr(const uint address,const uint data) const
				{
					ppu.Update();
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & MASK) | (data & 0xF) << SHIFT );
				}

				NES_POKE_D(Ss88006,A000) { SwapChr<0xF0,0>( 0x0000, data ); }
				NES_POKE_D(Ss88006,A001) { SwapChr<0x0F,4>( 0x0000, data ); }
				NES_POKE_D(Ss88006,A002) { SwapChr<0xF0,0>( 0x0400, data ); }
				NES_POKE_D(Ss88006,A003) { SwapChr<0x0F,4>( 0x0400, data ); }
				NES_POKE_D(Ss88006,B000) { SwapChr<0xF0,0>( 0x0800, data ); }
				NES_POKE_D(Ss88006,B001) { SwapChr<0x0F,4>( 0x0800, data ); }
				NES_POKE_D(Ss88006,B002) { SwapChr<0xF0,0>( 0x0C00, data ); }
				NES_POKE_D(Ss88006,B003) { SwapChr<0x0F,4>( 0x0C00, data ); }
				NES_POKE_D(Ss88006,C000) { SwapChr<0xF0,0>( 0x1000, data ); }
				NES_POKE_D(Ss88006,C001) { SwapChr<0x0F,4>( 0x1000, data ); }
				NES_POKE_D(Ss88006,C002) { SwapChr<0xF0,0>( 0x1400, data ); }
				NES_POKE_D(Ss88006,C003) { SwapChr<0x0F,4>( 0x1400, data ); }
				NES_POKE_D(Ss88006,D000) { SwapChr<0xF0,0>( 0x1800, data ); }
				NES_POKE_D(Ss88006,D001) { SwapChr<0x0F,4>( 0x1800, data ); }
				NES_POKE_D(Ss88006,D002) { SwapChr<0xF0,0>( 0x1C00, data ); }
				NES_POKE_D(Ss88006,D003) { SwapChr<0x0F,4>( 0x1C00, data ); }

				NES_POKE_D(Ss88006,E000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFFF0) | (data & 0xF) << 0;
				}

				NES_POKE_D(Ss88006,E001)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFF0F) | (data & 0xF) << 4;
				}

				NES_POKE_D(Ss88006,E002)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xF0FF) | (data & 0xF) << 8;
				}

				NES_POKE_D(Ss88006,E003)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0x0FFF) | (data & 0xF) << 12;
				}

				NES_POKE(Ss88006,F000)
				{
					irq.Update();
					irq.unit.count = irq.unit.latch;
					irq.ClearIRQ();
				}

				NES_POKE_D(Ss88006,F001)
				{
					irq.Update();

					if      (data & 0x8) irq.unit.mask = 0x000F;
					else if (data & 0x4) irq.unit.mask = 0x00FF;
					else if (data & 0x2) irq.unit.mask = 0x0FFF;
					else                 irq.unit.mask = 0xFFFF;

					irq.Connect( data & 0x1 );
					irq.ClearIRQ();
				}

				NES_POKE_D(Ss88006,F003)
				{
					NST_ASSERT( sound );

					uint tmp = reg;
					reg = data;

					if ((data & 0x2) < (tmp & 0x2) && (data & 0x1D) == (tmp & 0x1D))
						sound->Play( data >> 2 & 0x1F );
				}

				bool Ss88006::Irq::Clock()
				{
					return (count & mask) && !(--count & mask);
				}

				void Ss88006::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
