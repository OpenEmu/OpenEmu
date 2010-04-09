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

#include <cstring>
#include "NstBoard.hpp"
#include "../NstClock.hpp"
#include "NstBoardFfe.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Ffe::Trainer::Trainer(const Ram& ram)
			: available(ram.Size() >= SIZE)
			{
				if (available)
					std::memcpy( data, ram.Mem(), SIZE );
				else
					std::memset( data, 0x00, SIZE );
			}

			Ffe::Ffe(const Context& c)
			:
			Board   (c),
			irq     (board == Type::CUSTOM_FFE3 ? NULL : new ClockUnits::M2<Irq>(*c.cpu,0xFFFF)),
			trainer (c.trainer)
			{
			}

			Ffe::~Ffe()
			{
				delete irq;
			}

			void Ffe::Irq::Reset(const bool hard)
			{
				if (hard)
				{
					count = 0;
					enabled = false;
				}
			}

			void Ffe::SubReset(const bool hard)
			{
				if (hard)
					mode = 0;

				if (trainer.available && board.GetWram() >= 0x1000 + Trainer::SIZE)
					std::memcpy( wrk.Source().Mem(0x1000), trainer.data, Trainer::SIZE );

				Map( 0x42FEU, &Ffe::Poke_42FE );
				Map( 0x42FFU, &Ffe::Poke_42FF );

				if (irq)
				{
					irq->Reset( hard, hard || irq->Connected() );

					Map( 0x4501U, &Ffe::Poke_4501 );
					Map( 0x4502U, &Ffe::Poke_4502 );
					Map( 0x4503U, &Ffe::Poke_4503 );
				}

				switch (board.GetId())
				{
					case Type::CUSTOM_FFE3:

						Map( 0x8000U, 0xFFFFU, &Ffe::Poke_Prg_F3 );

						if (hard)
							prg.SwapBank<SIZE_32K,0x0000>(0);

						break;

					case Type::CUSTOM_FFE4:

						Map( 0x8000U, 0xFFFFU, &Ffe::Poke_Prg_F4 );

						if (hard)
							prg.SwapBank<SIZE_16K,0x4000>(7);

						break;

					case Type::CUSTOM_FFE8:

						Map( 0x4504U, PRG_SWAP_8K_0 );
						Map( 0x4505U, PRG_SWAP_8K_1 );
						Map( 0x4506U, PRG_SWAP_8K_2 );
						Map( 0x4507U, PRG_SWAP_8K_3 );
						Map( 0x4510U, CHR_SWAP_1K_0 );
						Map( 0x4511U, CHR_SWAP_1K_1 );
						Map( 0x4512U, CHR_SWAP_1K_2 );
						Map( 0x4513U, CHR_SWAP_1K_3 );
						Map( 0x4514U, CHR_SWAP_1K_4 );
						Map( 0x4515U, CHR_SWAP_1K_5 );
						Map( 0x4516U, CHR_SWAP_1K_6 );
						Map( 0x4517U, CHR_SWAP_1K_7 );

						break;
				}
			}

			void Ffe::SubLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'F','F','E'>::V) );

				if (baseChunk == AsciiId<'F','F','E'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:

								NST_VERIFY( board == Type::CUSTOM_FFE4 );

								if (board == Type::CUSTOM_FFE4)
									mode = state.Read8() & 0x1;

								break;

							case AsciiId<'I','R','Q'>::V:

								NST_VERIFY( irq );

								if (irq)
								{
									State::Loader::Data<3> data( state );

									irq->unit.enabled = data[0] & 0x1;
									irq->unit.count = data[1] | data[2] << 8;
								}
								break;
						}

						state.End();
					}
				}
			}

			void Ffe::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'F','F','E'>::V );

				if (board == Type::CUSTOM_FFE4)
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( mode ).End();

				if (irq)
				{
					const byte data[3] =
					{
						irq->unit.enabled != false,
						irq->unit.count & 0xFF,
						irq->unit.count >> 8
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
				}

				state.End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			bool Ffe::Irq::Clock()
			{
				if (enabled && count++ == clock)
				{
					count = 0;
					enabled = false;
					return true;
				}

				return false;
			}

			NES_POKE_D(Ffe,42FE)
			{
				mode = data >> 7 ^ 0x1;
				ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_1 : Ppu::NMT_0 );
			}

			NES_POKE_D(Ffe,42FF)
			{
				ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_H : Ppu::NMT_V );
			}

			NES_POKE_D(Ffe,4501)
			{
				irq->Update();
				irq->unit.enabled = data & 0x1;
				irq->ClearIRQ();
			}

			NES_POKE_D(Ffe,4502)
			{
				irq->Update();
				irq->unit.count = (irq->unit.count & 0xFF00) | data;
			}

			NES_POKE_D(Ffe,4503)
			{
				irq->Update();
				irq->unit.count = (irq->unit.count & 0x00FF) | (data << 8);
				irq->unit.enabled = true;
				irq->ClearIRQ();
			}

			NES_POKE_D(Ffe,Prg_F3)
			{
				ppu.Update();
				prg.SwapBank<SIZE_16K,0x0000>( data >> 3 );
				chr.SwapBank<SIZE_8K,0x0000>( data & 0x7 );
			}

			NES_POKE_D(Ffe,Prg_F4)
			{
				ppu.Update();

				if (mode || chr.Source(0).Writable())
				{
					prg.SwapBank<SIZE_16K,0x0000>( data >> 2 );
					data &= 0x3;
				}

				chr.Source( mode ).SwapBank<SIZE_8K,0x0000>( data );
			}

			void Ffe::Sync(Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_END_FRAME)
				{
					if (irq)
						irq->VSync();
				}

				Board::Sync( event, controllers );
			}
		}
	}
}
