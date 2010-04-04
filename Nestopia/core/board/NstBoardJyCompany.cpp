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

#include "../NstClock.hpp"
#include "../NstDipSwitches.hpp"
#include "NstBoard.hpp"
#include "NstBoardJyCompany.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace JyCompany
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Standard::CartSwitches::CartSwitches(uint d,bool l)
				: data(d), ppuLatched(l) {}

				inline bool Standard::CartSwitches::IsPpuLatched() const
				{
					return ppuLatched;
				}

				Standard::Irq::A12::A12(Irq& irq)
				: base(irq) {}

				Standard::Irq::M2::M2(Irq& irq)
				: base(irq) {}

				#if NST_MSVC >= 1200
				#pragma warning( push )
				#pragma warning( disable : 4355 )
				#endif

				Standard::Irq::Irq(Cpu& cpu,Ppu& ppu)
				: a12(cpu,ppu,*this), m2(cpu,*this) {}

				#if NST_MSVC >= 1200
				#pragma warning( pop )
				#endif

				Standard::Standard(const Context& c)
				:
				Board        (c),
				irq          (*c.cpu,*c.ppu),
				cartSwitches
				(
					board == Type::JYCOMPANY_TYPE_A ? DEFAULT_DIP_NMT_OFF :
					board == Type::JYCOMPANY_TYPE_B ? DEFAULT_DIP_NMT_CONTROLLED :
                                                      DEFAULT_DIP_NMT_ON,
					board == Type::JYCOMPANY_TYPE_B
				)
				{
				}

				void Standard::Regs::Reset()
				{
					mul[0] = 0;
					mul[1] = 0;
					tmp = 0;
					ctrl[0] = 0;
					ctrl[1] = 0;
					ctrl[2] = 0;
					ctrl[3] = 0;
				}

				void Standard::Banks::Reset()
				{
					for (uint i=0; i < 4; ++i)
						prg[i] = 0xFF;

					for (uint i=0; i < 8; ++i)
						chr[i] = 0xFFFF;

					for (uint i=0; i < 4; ++i)
						nmt[i] = 0x00;

					prg6 = NULL;

					chrLatch[0] = 0;
					chrLatch[1] = 4;
				}

				void Standard::Irq::A12::Reset(bool)
				{
				}

				void Standard::Irq::M2::Reset(bool)
				{
				}

				void Standard::Irq::Reset()
				{
					enabled = false;
					mode = 0;
					prescaler = 0;
					scale = 0xFF;
					count = 0;
					flip = 0;

					a12.Reset( true );
					m2.Reset( true, true );
				}

				void Standard::SubReset(bool)
				{
					for (uint i=0x5000; i < 0x5800; i += 0x4)
						Map( i, &Standard::Peek_5000 );

					for (uint i=0x5800; i < 0x6000; i += 0x4)
					{
						cpu.Map( i + 0x0 ).Set( &regs, &Standard::Regs::Peek_5800, &Standard::Regs::Poke_5800 );
						cpu.Map( i + 0x1 ).Set( &regs, &Standard::Regs::Peek_5801, &Standard::Regs::Poke_5801 );
						cpu.Map( i + 0x3 ).Set( &regs, &Standard::Regs::Peek_5803, &Standard::Regs::Poke_5803 );
					}

					Map( 0x6000U, 0x7FFFU, &Standard::Peek_6000 );
					Map( 0x8000U, 0x8FFFU, &Standard::Poke_8000 );
					Map( 0x9000U, 0x9FFFU, &Standard::Poke_9000 );
					Map( 0xA000U, 0xAFFFU, &Standard::Poke_A000 );

					for (uint i=0x0000; i < 0x1000; i += 0x8)
					{
						Map( 0xB000 + i, 0xB003 + i, &Standard::Poke_B000 );
						Map( 0xB004 + i, 0xB007 + i, &Standard::Poke_B004 );

						Map( 0xC000 + i, &Standard::Poke_C000 );
						Map( 0xC001 + i, &Standard::Poke_C001 );
						Map( 0xC002 + i, &Standard::Poke_C002 );
						Map( 0xC003 + i, &Standard::Poke_C003 );
						Map( 0xC004 + i, &Standard::Poke_C004 );
						Map( 0xC005 + i, &Standard::Poke_C005 );
						Map( 0xC006 + i, &Standard::Poke_C006 );
					}

					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0xD000 + i, &Standard::Poke_D000 );
						Map( 0xD001 + i, &Standard::Poke_D001 );
						Map( 0xD002 + i, &Standard::Poke_D002 );
						Map( 0xD003 + i, &Standard::Poke_D003 );
					}

					regs.Reset();
					banks.Reset();
					irq.Reset();

					ppu.SetHActiveHook( Hook(this,&Standard::Hook_HActive) );
					ppu.SetHBlankHook( Hook(this,&Standard::Hook_HBlank) );

					if (cartSwitches.IsPpuLatched())
					{
						chr.SetAccessor( 0, this, &Standard::Access_Chr_0000 );
						chr.SetAccessor( 1, this, &Standard::Access_Chr_1000 );
					}

					UpdatePrg();
					UpdateExChr();
					UpdateChr();
					UpdateNmt();
				}

				void Standard::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'J','Y','C'>::V) );

					if (baseChunk == AsciiId<'J','Y','C'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<35> data( state );

									regs.ctrl[0] = data[0];
									regs.ctrl[1] = data[1];
									regs.ctrl[2] = data[2];
									regs.ctrl[3] = data[3];
									regs.mul[0]  = data[4];
									regs.mul[1]  = data[5];
									regs.tmp     = data[6];
									banks.prg[0] = data[7];
									banks.prg[1] = data[8];
									banks.prg[2] = data[9];
									banks.prg[3] = data[10];
									banks.chr[0] = data[11] | data[12] << 8;
									banks.chr[1] = data[13] | data[14] << 8;
									banks.chr[2] = data[15] | data[16] << 8;
									banks.chr[3] = data[17] | data[18] << 8;
									banks.chr[4] = data[19] | data[20] << 8;
									banks.chr[5] = data[21] | data[22] << 8;
									banks.chr[6] = data[23] | data[24] << 8;
									banks.chr[7] = data[25] | data[26] << 8;
									banks.nmt[0] = data[27] | data[28] << 8;
									banks.nmt[1] = data[29] | data[30] << 8;
									banks.nmt[2] = data[31] | data[32] << 8;
									banks.nmt[3] = data[33] | data[34] << 8;

									UpdatePrg();
									UpdateExChr();
									UpdateChr();
									UpdateNmt();

									break;
								}

								case AsciiId<'L','A','T'>::V:

									NST_VERIFY( cartSwitches.IsPpuLatched() );

									if (cartSwitches.IsPpuLatched())
									{
										banks.chrLatch[0] = state.Read8();
										banks.chrLatch[1] = banks.chrLatch[0] >> 3 & 0x7;
										banks.chrLatch[0] &= 0x7;

										UpdateChr();
									}
									break;

								case AsciiId<'I','R','Q'>::V:
								{
									byte data[5];
									state.Read( data );

									irq.enabled   = data[0] & 0x1U;
									irq.mode      = data[1];
									irq.prescaler = data[2];
									irq.count     = data[3];
									irq.flip      = data[4];

									irq.scale = (irq.mode & Irq::MODE_SCALE_3BIT) ? 0x7 : 0xFF;

									break;
								}
							}

							state.End();
						}
					}
				}

				void Standard::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'J','Y','C'>::V );

					{
						const byte data[35] =
						{
							regs.ctrl[0],
							regs.ctrl[1],
							regs.ctrl[2],
							regs.ctrl[3],
							regs.mul[0],
							regs.mul[1],
							regs.tmp,
							banks.prg[0],
							banks.prg[1],
							banks.prg[2],
							banks.prg[3],
							banks.chr[0] & 0xFF,
							banks.chr[0] >> 8,
							banks.chr[1] & 0xFF,
							banks.chr[1] >> 8,
							banks.chr[2] & 0xFF,
							banks.chr[2] >> 8,
							banks.chr[3] & 0xFF,
							banks.chr[3] >> 8,
							banks.chr[4] & 0xFF,
							banks.chr[4] >> 8,
							banks.chr[5] & 0xFF,
							banks.chr[5] >> 8,
							banks.chr[6] & 0xFF,
							banks.chr[6] >> 8,
							banks.chr[7] & 0xFF,
							banks.chr[7] >> 8,
							banks.nmt[0] & 0xFF,
							banks.nmt[0] >> 8,
							banks.nmt[1] & 0xFF,
							banks.nmt[1] >> 8,
							banks.nmt[2] & 0xFF,
							banks.nmt[2] >> 8,
							banks.nmt[3] & 0xFF,
							banks.nmt[3] >> 8
						};

						state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					}

					if (cartSwitches.IsPpuLatched())
						state.Begin( AsciiId<'L','A','T'>::V ).Write8( banks.chrLatch[0] | banks.chrLatch[1] << 3 ).End();

					{
						const byte data[5] =
						{
							irq.enabled != 0,
							irq.mode,
							irq.prescaler & 0xFF,
							irq.count,
							irq.flip
						};

						state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					}

					state.End();
				}

				uint Standard::CartSwitches::NumDips() const
				{
					return 2;
				}

				uint Standard::CartSwitches::NumValues(uint i) const
				{
					NST_ASSERT( i < 2 );
					return (i == 0) ? 4 : 3;
				}

				cstring Standard::CartSwitches::GetDipName(uint dip) const
				{
					NST_ASSERT( dip < 2 );
					return (dip == 0) ? "Game Select" : "Extended Mirroring";
				}

				cstring Standard::CartSwitches::GetValueName(uint dip,uint value) const
				{
					NST_ASSERT( dip < 2 );

					if (dip == 0)
					{
						NST_ASSERT( value < 4 );
						return (value == 0) ? "1" : (value == 1) ? "2" : (value == 2) ? "3" : "4";
					}
					else
					{
						NST_ASSERT( value < 3 );
						return (value == 0) ? "Off" : (value == 1) ? "Controlled" : "On";
					}
				}

				uint Standard::CartSwitches::GetValue(uint dip) const
				{
					NST_ASSERT( dip < 2 );

					if (dip == 0)
						return data >> 6;
					else
						return data & DIPSWITCH_NMT;
				}

				void Standard::CartSwitches::SetValue(uint dip,uint value)
				{
					NST_ASSERT( dip < 2 );

					if (dip == 0)
					{
						NST_ASSERT( value < 4 );
						data = (data & ~uint(DIPSWITCH_GAME)) | (value << 6);
					}
					else
					{
						NST_ASSERT( value < 3 );
						data = (data & ~uint(DIPSWITCH_NMT)) | (value << 0);
					}
				}

				Standard::Device Standard::QueryDevice(DeviceType type)
				{
					if (type == DEVICE_DIP_SWITCHES)
						return &cartSwitches;
					else
						return Board::QueryDevice( type );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				inline uint Standard::CartSwitches::GetSetting() const
				{
					return data;
				}

				inline void Standard::Irq::Update()
				{
					a12.Update();
					m2.Update();
				}

				bool Standard::Irq::IsEnabled() const
				{
					return enabled &&
					(
						(mode & MODE_COUNT_ENABLE) == MODE_COUNT_DOWN ||
						(mode & MODE_COUNT_ENABLE) == MODE_COUNT_UP
					);
				}

				bool Standard::Irq::IsEnabled(uint checkMode) const
				{
					return (mode & MODE_SOURCE) == checkMode && IsEnabled();
				}

				bool Standard::Irq::Clock()
				{
					NST_ASSERT( IsEnabled() );

					if (mode & MODE_COUNT_DOWN)
						return (--prescaler & scale) == scale && (count-- & 0xFF) == 0x00;
					else
						return (++prescaler & scale) == 0x00 && (++count & 0xFF) == 0x00;
				}

				bool Standard::Irq::A12::Clock()
				{
					return base.IsEnabled(MODE_PPU_A12) && base.Clock();
				}

				bool Standard::Irq::M2::Clock()
				{
					return base.IsEnabled(MODE_M2) && base.Clock();
				}

				uint Standard::Banks::Unscramble(const uint bank)
				{
					return
					(
						(bank & 0x01) << 6 |
						(bank & 0x02) << 4 |
						(bank & 0x04) << 2 |
						(bank & 0x10) >> 2 |
						(bank & 0x20) >> 4 |
						(bank & 0x40) >> 6
					);
				}

				NES_HOOK(Standard,HActive)
				{
					if (irq.IsEnabled(Irq::MODE_PPU_READ) && ppu.IsEnabled())
					{
						for (uint i=0, hit=false; i < (32*4) * 2; i += 2)
						{
							if (irq.Clock() && !hit)
							{
								hit = true;
								cpu.DoIRQ( Cpu::IRQ_EXT, cpu.GetCycles() + ppu.GetClock() * i );
							}
						}
					}
				}

				NES_HOOK(Standard,HBlank)
				{
					if (irq.IsEnabled(Irq::MODE_PPU_READ) && ppu.IsEnabled())
					{
						for (uint i=0, hit=false; i < (8*4+2*4+2) * 2; i += 2)
						{
							if (irq.Clock() && !hit)
							{
								hit = true;
								cpu.DoIRQ( Cpu::IRQ_EXT, cpu.GetCycles() + ppu.GetClock() * i );
							}
						}
					}
				}

				NES_ACCESSOR(Standard,Chr_0000)
				{
					const uint data = chr.Peek( address );
					address &= 0xFF8;

					if (address == 0xFD8 || address == 0xFE8)
					{
						banks.chrLatch[0] = address >> 4 & 0x2;

						if ((regs.ctrl[0] & Regs::CTRL0_CHR_MODE) == Regs::CTRL0_CHR_SWAP_4K)
							UpdateChrLatch();
					}

					return data;
				}

				NES_ACCESSOR(Standard,Chr_1000)
				{
					const uint data = chr.Peek( address );
					address &= 0xFF8;

					if (address == 0xFD8 || address == 0xFE8)
					{
						banks.chrLatch[1] = address >> 4 & (0x2U|0x4U);

						if ((regs.ctrl[0] & Regs::CTRL0_CHR_MODE) == Regs::CTRL0_CHR_SWAP_4K)
							UpdateChrLatch();
					}

					return data;
				}

				NES_PEEK_A(Standard,5000)
				{
					return (cartSwitches.GetSetting() & DIPSWITCH_GAME) | (address >> 8 & ~uint(DIPSWITCH_GAME));
				}

				NES_POKE_D(Standard::Regs,5800) { mul[0] = data; }
				NES_POKE_D(Standard::Regs,5801) { mul[1] = data; }
				NES_POKE_D(Standard::Regs,5803) { tmp = data;  }

				NES_PEEK(Standard::Regs,5800) { return (mul[0] * mul[1]) & 0xFF; }
				NES_PEEK(Standard::Regs,5801) { return (mul[0] * mul[1]) >> 8;   }
				NES_PEEK(Standard::Regs,5803) { return tmp; }

				NES_PEEK_A(Standard,6000)
				{
					NST_VERIFY( banks.prg6 );
					return banks.prg6 ? banks.prg6[address - 0x6000] : (address >> 8);
				}

				NES_POKE_AD(Standard,8000)
				{
					address &= 0x3;
					data &= 0x3F;

					if (banks.prg[address] != data)
					{
						banks.prg[address] = data;
						UpdatePrg();
					}
				}

				NES_POKE_AD(Standard,9000)
				{
					address &= 0x7;
					data |= banks.chr[address] & 0xFF00;

					if (banks.chr[address] != data)
					{
						banks.chr[address] = data;
						UpdateChr();
					}
				}

				NES_POKE_AD(Standard,A000)
				{
					address &= 0x7;
					data = data << 8 | (banks.chr[address] & 0x00FF);

					if (banks.chr[address] != data)
					{
						banks.chr[address] = data;
						UpdateChr();
					}
				}

				NES_POKE_AD(Standard,B000)
				{
					address &= 0x3;
					data |= banks.nmt[address] & 0xFF00;

					if (banks.nmt[address] != data)
					{
						banks.nmt[address] = data;
						UpdateNmt();
					}
				}

				NES_POKE_AD(Standard,B004)
				{
					address &= 0x3;
					data = data << 8 | (banks.nmt[address] & 0x00FF);

					if (banks.nmt[address] != data)
					{
						banks.nmt[address] = data;
						UpdateNmt();
					}
				}

				NES_POKE_D(Standard,C000)
				{
					data &= Irq::TOGGLE;

					if (irq.enabled != data)
					{
						irq.Update();
						irq.enabled = data;

						if (!data)
							cpu.ClearIRQ();
					}
				}

				NES_POKE_D(Standard,C001)
				{
					if (irq.mode != data)
					{
						irq.Update();

						NST_VERIFY
						(
							(data & Irq::MODE_SCALE_ADJUST) == 0 &&
							(
								(data & Irq::MODE_SOURCE) == Irq::MODE_M2 ||
								(data & Irq::MODE_SOURCE) == Irq::MODE_PPU_A12 ||
								(data & Irq::MODE_SOURCE) == Irq::MODE_PPU_READ
							)
						);

						irq.mode = data;
						irq.scale = (data & Irq::MODE_SCALE_3BIT) ? 0x7 : 0xFF;
					}
				}

				NES_POKE(Standard,C002)
				{
					if (irq.enabled)
					{
						irq.Update();
						irq.enabled = false;
						cpu.ClearIRQ();
					}
				}

				NES_POKE(Standard,C003)
				{
					if (!irq.enabled)
					{
						irq.Update();
						irq.enabled = true;
					}
				}

				NES_POKE_D(Standard,C004)
				{
					irq.Update();
					irq.prescaler = data ^ irq.flip;
				}

				NES_POKE_D(Standard,C005)
				{
					irq.Update();
					irq.count = data ^ irq.flip;
				}

				NES_POKE_D(Standard,C006)
				{
					irq.flip = data;
				}

				NES_POKE_D(Standard,D000)
				{
					if (regs.ctrl[0] != data)
					{
						regs.ctrl[0] = data;
						UpdatePrg();
						UpdateExChr();
						UpdateChr();
						UpdateNmt();
					}
				}

				NES_POKE_D(Standard,D001)
				{
					if (regs.ctrl[1] != data)
					{
						regs.ctrl[1] = data;
						UpdateNmt();
					}
				}

				NES_POKE_D(Standard,D002)
				{
					if (regs.ctrl[2] != data)
					{
						regs.ctrl[2] = data;
						UpdateNmt();
					}
				}

				NES_POKE_D(Standard,D003)
				{
					if (regs.ctrl[3] != data)
					{
						regs.ctrl[3] = data;
						UpdatePrg();
						UpdateExChr();
						UpdateChr();
					}
				}

				void Standard::UpdatePrg()
				{
					NST_VERIFY( (regs.ctrl[0] & Regs::CTRL0_PRG_MODE) != Regs::CTRL0_PRG_SWAP_8K_R );

					const uint exPrg = (regs.ctrl[3] & Regs::CTRL3_EX_PRG) << 5;

					if (!(regs.ctrl[0] & Regs::CTRL0_PRG6_ENABLE))
					{
						banks.prg6 = NULL;
					}
					else
					{
						uint bank = banks.prg[3];

						switch (regs.ctrl[0] & Regs::CTRL0_PRG_MODE)
						{
							case Regs::CTRL0_PRG_SWAP_32K:  bank = bank << 2 | 0x3; break;
							case Regs::CTRL0_PRG_SWAP_16K:  bank = bank << 1 | 0x1; break;
							case Regs::CTRL0_PRG_SWAP_8K_R: bank = banks.Unscramble( bank ); break;
						}

						banks.prg6 = prg.Source().Mem( ((bank & 0x3FUL) | exPrg) * SIZE_8K );
					}

					const uint last = (regs.ctrl[0] & Regs::CTRL0_PRG_NOT_LAST) ? banks.prg[3] : 0x3F;

					switch (regs.ctrl[0] & Regs::CTRL0_PRG_MODE)
					{
						case Regs::CTRL0_PRG_SWAP_32K:

							prg.SwapBank<SIZE_32K,0x0000>
							(
								(last & 0xF) | (exPrg >> 2)
							);
							break;

						case Regs::CTRL0_PRG_SWAP_16K:

							prg.SwapBanks<SIZE_16K,0x0000>
							(
								(banks.prg[1] & 0x1F) | (exPrg >> 1),
								(last         & 0x1F) | (exPrg >> 1)
							);
							break;

						case Regs::CTRL0_PRG_SWAP_8K:

							prg.SwapBanks<SIZE_8K,0x0000>
							(
								(banks.prg[0] & 0x3F) | exPrg,
								(banks.prg[1] & 0x3F) | exPrg,
								(banks.prg[2] & 0x3F) | exPrg,
								(last         & 0x3F) | exPrg
							);
							break;

						case Regs::CTRL0_PRG_SWAP_8K_R:

							prg.SwapBanks<SIZE_8K,0x0000>
							(
								(banks.Unscramble( banks.prg[0] ) & 0x3F) | exPrg,
								(banks.Unscramble( banks.prg[1] ) & 0x3F) | exPrg,
								(banks.Unscramble( banks.prg[2] ) & 0x3F) | exPrg,
								(banks.Unscramble( last         ) & 0x3F) | exPrg
							);
							break;
					}
				}

				void Standard::UpdateExChr()
				{
					if (regs.ctrl[3] & Regs::CTRL3_NO_EX_CHR)
					{
						banks.exChr.mask = 0xFFFF;
						banks.exChr.bank = 0x0000;
					}
					else
					{
						const uint mode = (regs.ctrl[0] & Regs::CTRL0_CHR_MODE) >> 3;

						banks.exChr.mask = 0x00FFU >> (mode ^ 0x3);
						banks.exChr.bank = ((regs.ctrl[3] & Regs::CTRL3_EX_CHR_0) | ((regs.ctrl[3] & Regs::CTRL3_EX_CHR_1) >> 2)) << (mode + 5);
					}
				}

				void Standard::UpdateChr() const
				{
					ppu.Update();

					switch (regs.ctrl[0] & Regs::CTRL0_CHR_MODE)
					{
						case Regs::CTRL0_CHR_SWAP_8K:

							chr.SwapBank<SIZE_8K,0x0000>
							(
								(banks.chr[0] & banks.exChr.mask) | banks.exChr.bank
							);
							break;

						case Regs::CTRL0_CHR_SWAP_4K:

							UpdateChrLatch();
							break;

						case Regs::CTRL0_CHR_SWAP_2K:

							chr.SwapBanks<SIZE_2K,0x0000>
							(
								(banks.chr[0] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[2] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[4] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[6] & banks.exChr.mask) | banks.exChr.bank
							);
							break;

						case Regs::CTRL0_CHR_SWAP_1K:

							chr.SwapBanks<SIZE_1K,0x0000>
							(
								(banks.chr[0] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[1] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[2] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[3] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[4] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[5] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[6] & banks.exChr.mask) | banks.exChr.bank,
								(banks.chr[7] & banks.exChr.mask) | banks.exChr.bank
							);
							break;
					}
				}

				void Standard::UpdateChrLatch() const
				{
					NST_ASSERT( (regs.ctrl[0] & Regs::CTRL0_CHR_MODE) == Regs::CTRL0_CHR_SWAP_4K );

					chr.SwapBanks<SIZE_4K,0x0000>
					(
						(banks.chr[banks.chrLatch[0]] & banks.exChr.mask) | banks.exChr.bank,
						(banks.chr[banks.chrLatch[1]] & banks.exChr.mask) | banks.exChr.bank
					);
				}

				void Standard::UpdateNmt()
				{
					if ((regs.ctrl[0] >> 5 & cartSwitches.GetSetting() & 0x1) | (cartSwitches.GetSetting() & 0x2))
					{
						ppu.Update();

						for (uint i=0; i < 4; ++i)
							nmt.Source( (regs.ctrl[0] & Regs::CTRL0_NMT_CHR_ROM) || ((banks.nmt[i] ^ regs.ctrl[2]) & Regs::CTRL2_NMT_USE_RAM) ).SwapBank<SIZE_1K>( i * 0x0400, banks.nmt[i] );
					}
					else
					{
						SetMirroringVH01( regs.ctrl[1] );
					}
				}

				void Standard::Sync(Event event,Input::Controllers* controllers)
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
