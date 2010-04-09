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
#include "NstBoardTengenRambo1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Tengen
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Rambo1::Irq::Irq(Cpu& cpu,Ppu& ppu)
				:
				a12 ( cpu, ppu, unit ),
				m2  ( cpu, unit )
				{}

				Rambo1::Rambo1(const Context& c)
				:
				Board (c),
				irq   (*c.cpu,*c.ppu)
				{}

				void Rambo1::Irq::Unit::Reset(const bool hard)
				{
					if (hard)
					{
						count = 0;
						reload = false;
						latch = 0;
						enabled = false;
					}
				}

				void Rambo1::Regs::Reset()
				{
					for (uint i=0; i < 8; ++i)
						chr[i] = i;

					for (uint i=0; i < 3; ++i)
						prg[i] = i;

					ctrl = 0;
				}

				void Rambo1::SubReset(const bool hard)
				{
					irq.a12.Reset( hard, !irq.m2.Connected() );
					irq.m2.Reset( hard, irq.m2.Connected() );

					if (hard)
						regs.Reset();

					for (uint i=0x0000; i < 0x1000; i += 0x2)
					{
						Map( 0x8000 + i, &Rambo1::Poke_8000 );
						Map( 0x8001 + i, &Rambo1::Poke_8001 );
						Map( 0xA000 + i, NMT_SWAP_HV        );
						Map( 0xC000 + i, &Rambo1::Poke_C000 );
						Map( 0xC001 + i, &Rambo1::Poke_C001 );
						Map( 0xE000 + i, &Rambo1::Poke_E000 );
						Map( 0xE001 + i, &Rambo1::Poke_E001 );
					}

					UpdateChr();
					UpdatePrg();
				}

				void Rambo1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'T','R','1'>::V) );

					if (baseChunk == AsciiId<'T','R','1'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<1+8+3> data( state );

									regs.ctrl = data[0];

									for (uint i=0; i < 3; ++i)
										regs.prg[i] = data[1+i];

									for (uint i=0; i < 8; ++i)
										regs.chr[i] = data[1+3+i];

									break;
								}

								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<3> data( state );

									irq.unit.enabled = data[0] & 0x1;
									irq.a12.Connect( data[0] & 0x2 );
									irq.m2.Connect( data[0] & 0x2 );
									irq.unit.reload = data[0] & 0x4;
									irq.unit.latch = data[1];
									irq.unit.count = data[2];

									break;
								}
							}

							state.End();
						}
					}
				}

				void Rambo1::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'T','R','1'>::V );

					{
						const byte data[1+8+3] =
						{
							regs.ctrl,
							regs.prg[0],
							regs.prg[1],
							regs.prg[2],
							regs.chr[0],
							regs.chr[1],
							regs.chr[2],
							regs.chr[3],
							regs.chr[4],
							regs.chr[5],
							regs.chr[6],
							regs.chr[7]
						};

						state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					}

					{
						const byte data[3] =
						{
							(irq.unit.enabled   ? 0x1U : 0x0U) |
							(irq.m2.Connected() ? 0x2U : 0x0U) |
							(irq.unit.reload    ? 0x4U : 0x0U),
							irq.unit.latch,
							irq.unit.count & 0xFF
						};

						state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					}

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				bool Rambo1::Irq::Unit::Clock()
				{
					if (!reload)
					{
						if (count)
						{
							return !--count && enabled;
						}
						else
						{
							count = latch;
							return false;
						}
					}
					else
					{
						reload = false;
						count = latch + 1;
						return false;
					}
				}

				void Rambo1::Irq::Update()
				{
					a12.Update();
					m2.Update();
				}

				void Rambo1::UpdatePrg()
				{
					prg.SwapBanks<SIZE_8K,0x0000>
					(
						regs.prg[(regs.ctrl & 0x40U) ? 2 : 0],
						regs.prg[(regs.ctrl & 0x40U) ? 0 : 1],
						regs.prg[(regs.ctrl & 0x40U) ? 1 : 2],
						0xFF
					);
				}

				void Rambo1::UpdateChr() const
				{
					ppu.Update();

					const uint offset = (regs.ctrl & 0x80U) << 5;

					if (regs.ctrl & 0x20U)
						chr.SwapBanks<SIZE_1K>( offset, regs.chr[0], regs.chr[6], regs.chr[1], regs.chr[7] );
					else
						chr.SwapBanks<SIZE_2K>( offset, regs.chr[0] >> 1, regs.chr[1] >> 1 );

					chr.SwapBanks<SIZE_1K>( offset ^ 0x1000, regs.chr[2], regs.chr[3], regs.chr[4], regs.chr[5] );
				}

				NES_POKE_D(Rambo1,8000)
				{
					const uint diff = regs.ctrl ^ data;
					regs.ctrl = data;

					if (diff & 0x40)
						UpdatePrg();

					if (diff & (0x20|0x80))
						UpdateChr();
				}

				NES_POKE_D(Rambo1,8001)
				{
					const uint index = regs.ctrl & 0xFU;

					if (index < 0x6)
					{
						if (regs.chr[index] != data)
						{
							regs.chr[index] = data;
							UpdateChr();
						}
					}
					else switch (index)
					{
						case 0x6:
						case 0x7:

							if (regs.prg[index - 0x6] != data)
							{
								regs.prg[index - 0x6] = data;
								UpdatePrg();
							}
							break;

						case 0x8:
						case 0x9:

							if (regs.chr[index - 0x2] != data)
							{
								regs.chr[index - 0x2] = data;
								UpdateChr();
							}
							break;

						case 0xF:

							if (regs.prg[2] != data)
							{
								regs.prg[2] = data;
								UpdatePrg();
							}
							break;
					}
				}

				NES_POKE_D(Rambo1,C000)
				{
					irq.Update();
					irq.unit.latch = data;
				}

				NES_POKE_D(Rambo1,C001)
				{
					irq.Update();

					irq.unit.reload = true;
					data &= Irq::SOURCE;

					irq.a12.Connect( data == Irq::SOURCE_PPU );
					irq.m2.Connect( data == Irq::SOURCE_CPU );
				}

				NES_POKE(Rambo1,E000)
				{
					irq.Update();
					irq.unit.enabled = false;
					cpu.ClearIRQ();
				}

				NES_POKE(Rambo1,E001)
				{
					irq.Update();
					irq.unit.enabled = true;
				}

				void Rambo1::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
					{
						irq.a12.VSync();
						irq.m2.VSync();
					}

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
