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
#include "NstBoardMmc3.hpp"
#include "NstBoardSomeriTeam.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace SomeriTeam
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Sl12::Sl12(const Context& c)
				:
				Board (c),
				irq   (*c.cpu,*c.ppu,false)
				{}

				void Sl12::SubReset(const bool hard)
				{
					irq.Reset( hard );

					if (hard)
					{
						mode = 0;

						vrc2.prg[0] = 0x0;
						vrc2.prg[1] = 0x1;
						vrc2.nmt = 0;

						for (uint i=0; i < 8; ++i)
							vrc2.chr[i] = i;

						mmc3.ctrl = 0;
						mmc3.nmt = 0;

						mmc3.banks[0] = 0x0;
						mmc3.banks[1] = 0x1;
						mmc3.banks[2] = 0x4;
						mmc3.banks[3] = 0x5;
						mmc3.banks[4] = 0x6;
						mmc3.banks[5] = 0x7;

						mmc3.banks[6] = 0x3C;
						mmc3.banks[7] = 0x3D;
						mmc3.banks[8] = 0xFE;
						mmc3.banks[9] = 0xFF;

						mmc1.buffer = 0;
						mmc1.shifter = 0;

						mmc1.regs[0] = 0x4U|0x8U;
						mmc1.regs[1] = 0;
						mmc1.regs[2] = 0;
						mmc1.regs[3] = 0;
					}

					for (uint i=0x4100; i < 0x6000; i += 0x200)
						Map( i + 0x00, i + 0xFF, &Sl12::Poke_4100 );

					Map( 0x8000U, 0x8FFFU, &Sl12::Poke_8000 );
					Map( 0x9000U, 0x9FFFU, &Sl12::Poke_9000 );
					Map( 0xA000U, 0xAFFFU, &Sl12::Poke_A000 );
					Map( 0xB000U, 0xBFFFU, &Sl12::Poke_B000 );
					Map( 0xC000U, 0xCFFFU, &Sl12::Poke_C000 );
					Map( 0xD000U, 0xDFFFU, &Sl12::Poke_D000 );
					Map( 0xE000U, 0xEFFFU, &Sl12::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, &Sl12::Poke_F000 );

					UpdatePrg();
					UpdateNmt();
					UpdateChr();
				}

				void Sl12::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','1','2'>::V) );

					if (baseChunk == AsciiId<'S','1','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									mode = state.Read8();
									break;

								case AsciiId<'V','R','2'>::V:

									state.Read( vrc2.chr );
									state.Read( vrc2.prg );
									vrc2.nmt = state.Read8();
									break;

								case AsciiId<'M','M','3'>::V:

									state.Read( mmc3.banks );
									mmc3.ctrl = state.Read8();
									mmc3.nmt = state.Read8();
									break;

								case AsciiId<'M','M','1'>::V:

									state.Read( mmc1.regs );
									mmc1.buffer = state.Read8();
									mmc1.shifter = state.Read8();
									break;

								case AsciiId<'I','R','Q'>::V:

									irq.unit.LoadState( state );
									break;
							}

							state.End();
						}
					}

					UpdatePrg();
					UpdateNmt();
					UpdateChr();
				}

				void Sl12::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'S','1','2'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( mode ).End();
					state.Begin( AsciiId<'V','R','2'>::V ).Write( vrc2.chr ).Write( vrc2.prg ).Write8( vrc2.nmt ).End();
					state.Begin( AsciiId<'M','M','3'>::V ).Write( mmc3.banks ).Write8( mmc3.ctrl ).Write8( mmc3.nmt ).End();
					state.Begin( AsciiId<'M','M','1'>::V ).Write( mmc1.regs ).Write8( mmc1.buffer ).Write8( mmc1.shifter ).End();
					irq.unit.SaveState( state, AsciiId<'I','R','Q'>::V );
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void Sl12::UpdatePrg()
				{
					switch (mode & 0x3)
					{
						case 0x0:

							prg.SwapBanks<SIZE_8K,0x0000>( vrc2.prg[0], vrc2.prg[1], 0x1E, 0x1F );
							break;

						case 0x1:
						{
							const uint i = mmc3.ctrl >> 5 & 0x2U;
							prg.SwapBanks<SIZE_8K,0x0000>( mmc3.banks[6+i], mmc3.banks[6+1], mmc3.banks[6+(i^2)], mmc3.banks[6+3] );
							break;
						}

						case 0x2:
						{
							const uint bank = mmc1.regs[3] & 0xFU;

							if (mmc1.regs[0] & 0x8U)
								prg.SwapBanks<SIZE_16K,0x0000>( (mmc1.regs[0] & 0x4U) ? bank : 0x0, (mmc1.regs[0] & 0x4U) ? 0xF : bank );
							else
								prg.SwapBank<SIZE_32K,0x0000>( bank >> 1 );

							break;
						}
					}
				}

				void Sl12::UpdateChr() const
				{
					const uint base = (mode & 0x4) << 6;

					switch (mode & 0x3)
					{
						case 0x0:

							chr.SwapBanks<SIZE_1K,0x0000>( base|vrc2.chr[0], base|vrc2.chr[1], base|vrc2.chr[2], base|vrc2.chr[3], base|vrc2.chr[4], base|vrc2.chr[5], base|vrc2.chr[6], base|vrc2.chr[7] );
							break;

						case 0x1:
						{
							const uint swap = (mmc3.ctrl & 0x80U) << 5;
							chr.SwapBanks<SIZE_2K>( 0x0000 ^ swap, base >> 1 | mmc3.banks[0], base >> 1 | mmc3.banks[1] );
							chr.SwapBanks<SIZE_1K>( 0x1000 ^ swap, base|mmc3.banks[2], base|mmc3.banks[3], base|mmc3.banks[4], base|mmc3.banks[5] );
							break;
						}

						case 0x2:

							chr.SwapBanks<SIZE_4K,0x0000>( (mmc1.regs[0] & 0x10U) ? mmc1.regs[1] : mmc1.regs[1] & 0x1EU, (mmc1.regs[0] & 0x10U) ? mmc1.regs[2] : mmc1.regs[1] | 0x01U );
							break;
					}
				}

				void Sl12::UpdateNmt() const
				{
					Ppu::NmtMirroring nmtCtrl;

					switch (mode & 0x3)
					{
						case 0x0:

							nmtCtrl = (vrc2.nmt & 0x1U) ? Ppu::NMT_H : Ppu::NMT_V;
							break;

						case 0x1:

							nmtCtrl = (mmc3.nmt & 0x1U) ? Ppu::NMT_H : Ppu::NMT_V;
							break;

						case 0x2:

							switch (mmc1.regs[0] & 0x3U)
							{
								case 0x0: nmtCtrl = Ppu::NMT_0; break;
								case 0x1: nmtCtrl = Ppu::NMT_1; break;
								case 0x2: nmtCtrl = Ppu::NMT_V; break;
								default:  nmtCtrl = Ppu::NMT_H; break;
							}
							break;

						default: return;
					}

					ppu.SetMirroring( nmtCtrl );
				}

				void Sl12::Poke_Vrc2_8000(uint address,uint data)
				{
					NST_ASSERT( (mode & 0x3) == 0 );

					data &= 0x1F;
					address = address >> 13 & 0x1;

					if (vrc2.prg[address] != data)
					{
						vrc2.prg[address] = data;
						UpdatePrg();
					}
				}

				void Sl12::Poke_Vrc2_9000(uint,uint data)
				{
					NST_ASSERT( (mode & 0x3) == 0 );

					data &= 0x1;

					if (vrc2.nmt != data)
					{
						vrc2.nmt = data;
						UpdateNmt();
					}
				}

				void Sl12::Poke_Vrc2_B000(uint address,uint data)
				{
					NST_ASSERT( (mode & 0x3) == 0 );

					data = (data & 0xF) << (address << 1 & 0x4);
					address = ((address - 0xB000) >> 11 & 0x6) | (address & 0x1);

					if (vrc2.chr[address] != data)
					{
						vrc2.chr[address] = data;
						ppu.Update();
						UpdateChr();
					}
				}

				void Sl12::Poke_Mmc3_8000(uint address,uint data)
				{
					NST_ASSERT( (mode & 0x3) == 1 );

					if (address & 0x1)
					{
						address = mmc3.ctrl & 0x7U;

						if (address < 2)
							data >>= 1;

						if (mmc3.banks[address] != data)
						{
							mmc3.banks[address] = data;

							if (address < 6)
							{
								ppu.Update();
								UpdateChr();
							}
							else
							{
								UpdatePrg();
							}
						}
					}
					else
					{
						address = mmc3.ctrl ^ data;
						mmc3.ctrl = data;

						if (address & 0x40)
							UpdatePrg();

						if (address & (0x80U|0x07U))
						{
							ppu.Update();
							UpdateChr();
						}
					}
				}

				void Sl12::Poke_Mmc3_A000(uint address,uint data)
				{
					NST_ASSERT( (mode & 0x3) == 1 );

					if (!(address & 0x1))
					{
						if (mmc3.nmt != data)
						{
							mmc3.nmt = data;
							UpdateNmt();
						}
					}
				}

				void Sl12::Poke_Mmc3_C000(uint address,uint data)
				{
					NST_ASSERT( (mode & 0x3) == 1 );

					irq.Update();

					if (address & 0x1)
						irq.unit.Reload();
					else
						irq.unit.SetLatch( data );
				}

				void Sl12::Poke_Mmc3_E000(uint address,uint)
				{
					NST_ASSERT( (mode & 0x3) == 1 );

					irq.Update();

					if (address & 0x1)
						irq.unit.Enable();
					else
						irq.unit.Disable( cpu );
				}

				void Sl12::Poke_Mmc1_8000(uint address,uint data)
				{
					NST_ASSERT( (mode & 0x3) == 2 );

					if (!(data & 0x80))
					{
						mmc1.buffer |= (data & 0x1) << mmc1.shifter++;

						if (mmc1.shifter != 5)
							return;

						mmc1.shifter = 0;
						data = mmc1.buffer;
						mmc1.buffer = 0;

						address = address >> 13 & 0x3;

						if (mmc1.regs[address] != data)
						{
							mmc1.regs[address] = data;

							UpdatePrg();
							UpdateNmt();
							UpdateChr();
						}
					}
					else
					{
						mmc1.buffer = 0;
						mmc1.shifter = 0;

						if ((mmc1.regs[0] & (0x4U|0x8U)) != (0x4U|0x8U))
						{
							mmc1.regs[0] |= (0x4U|0x8U);

							UpdatePrg();
							UpdateNmt();
							UpdateChr();
						}
					}
				}

				NES_POKE_D(Sl12,4100)
				{
					if (mode != data)
					{
						mode = data;

						if ((data & 0x3) != 1)
							irq.unit.Disable( cpu );

						UpdatePrg();
						UpdateNmt();
						UpdateChr();
					}
				}

				NES_POKE_AD(Sl12,8000)
				{
					switch (mode & 0x3)
					{
						case 0x0: Poke_Vrc2_8000( address, data ); break;
						case 0x1: Poke_Mmc3_8000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				NES_POKE_AD(Sl12,9000)
				{
					switch (mode & 0x3)
					{
						case 0x0: Poke_Vrc2_9000( address, data ); break;
						case 0x1: Poke_Mmc3_8000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				NES_POKE_AD(Sl12,A000)
				{
					switch (mode & 0x3)
					{
						case 0x0: Poke_Vrc2_8000( address, data ); break;
						case 0x1: Poke_Mmc3_A000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				NES_POKE_AD(Sl12,B000)
				{
					switch (mode & 0x3)
					{
						case 0x0: Poke_Vrc2_B000( address, data ); break;
						case 0x1: Poke_Mmc3_A000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				NES_POKE_AD(Sl12,C000)
				{
					switch (mode & 0x3)
					{
						case 0x0: Poke_Vrc2_B000( address, data ); break;
						case 0x1: Poke_Mmc3_C000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				NES_POKE_AD(Sl12,D000)
				{
					switch (mode & 0x3)
					{
						case 0x0: Poke_Vrc2_B000( address, data ); break;
						case 0x1: Poke_Mmc3_C000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				NES_POKE_AD(Sl12,E000)
				{
					switch (mode & 0x3)
					{
						case 0x0: Poke_Vrc2_B000( address, data ); break;
						case 0x1: Poke_Mmc3_E000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				NES_POKE_AD(Sl12,F000)
				{
					switch (mode & 0x3)
					{
						case 0x0: break;
						case 0x1: Poke_Mmc3_E000( address, data ); break;
						case 0x2: Poke_Mmc1_8000( address, data ); break;
					}
				}

				void Sl12::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
