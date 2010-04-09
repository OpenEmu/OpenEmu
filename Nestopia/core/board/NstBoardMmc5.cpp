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
#include "NstBoardMmc5.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			const byte Mmc5::Filler::squared[4] = {0x00,0x55,0xAA,0xFF};

			Mmc5::Sound::Sound(Apu& a,bool connect)
			: Channel(a)
			{
				Reset();
				bool audible = UpdateSettings();

				if (connect)
					Connect( audible );
			}

			Mmc5::Banks::Wrk::Wrk(dword s)
			{
				enum {X = INVALID};

				static const byte access[6][8] =
				{
					{X,X,X,X,X,X,X,X},
					{0,0,0,0,X,X,X,X},
					{0,0,0,0,1,1,1,1},
					{0,1,2,3,X,X,X,X},
					{0,1,2,3,4,4,4,4},
					{0,1,2,3,4,5,6,7}
				};

				std::memcpy( banks, access[s==SIZE_16K ? 2 : s==SIZE_32K ? 3 : s==SIZE_40K ? 4 : s==SIZE_64K ? 5 : s ? 1 : 0], 8 );
			}

			Mmc5::Banks::Banks(uint wrkSize)
			: wrk(wrkSize) {}

			Mmc5::Mmc5(const Context& c)
			:
			Board  (c),
			banks  (board.GetWram()),
			sound  (*c.apu)
			{
				nmt.Source(1).Set( Ram::RAM, true, true, sizeof(exRam.mem), exRam.mem );
			}

			void Mmc5::Flow::Reset()
			{
				cycles = 0;
				phase = &Mmc5::VBlank;
				scanline = 240;
			}

			void Mmc5::Irq::Reset()
			{
				state = 0;
				count = 0U-2U;
				target = 0;
			}

			void Mmc5::Regs::Reset()
			{
				prgMode = PRG_MODE_8K;
				chrMode = CHR_MODE_8K;
				exRamMode = EXRAM_MODE_PPU_NT;
				mul[0] = 0;
				mul[1] = 0;
			}

			void Mmc5::Banks::Reset()
			{
				nmt = 0;

				for (uint i=0; i < 8; ++i)
					chrA[i] = i;

				for (uint i=0; i < 4; ++i)
					chrB[i] = i;

				chrHigh = 0;
				lastChr = LAST_CHR_A;
				fetchMode = FETCH_MODE_NONE;
				security = 0;

				for (uint i=0; i < 4; ++i)
					prg[i] = 0xFF;
			}

			void Mmc5::Filler::Reset()
			{
				tile = 0;
				attribute = 0;
			}

			void Mmc5::Spliter::Reset()
			{
				ctrl = 0;
				tile = 0;
				inside = false;
				yStart = 0;
				chrBank = 0;
				x = 0;
				y = 0;
			}

			void Mmc5::ExRam::Reset(bool hard)
			{
				tile = 0;

				if (hard)
					std::memset( mem, 0x00, sizeof(mem) );
			}

			void Mmc5::Sound::Square::Reset()
			{
				waveLength = 0;
				active = false;
				frequency = 0;
				timer = 0;
				step = 0;
				duty = 0;

				lengthCounter.Reset();
				envelope.Reset();
			}

			void Mmc5::Sound::Pcm::Reset()
			{
				sample = 0;
				enabled = false;
				amp = 0;
			}

			void Mmc5::Sound::Reset()
			{
				atHalfClock = 0;

				for (uint i=0; i < NUM_SQUARES; ++i)
					square[i].Reset();

				pcm.Reset();
				dcBlocker.Reset();
			}

			void Mmc5::SubReset(const bool hard)
			{
				cpu.AddHook( Hook(this,&Mmc5::Hook_Cpu) );
				ppu.SetHActiveHook( Hook(this,&Mmc5::Hook_HActive) );
				ppu.SetHBlankHook( Hook(this,&Mmc5::Hook_HBlank) );

				Map( 0x5000U,          &Mmc5::Poke_5000 );
				Map( 0x5002U,          &Mmc5::Poke_5002 );
				Map( 0x5003U,          &Mmc5::Poke_5003 );
				Map( 0x5004U,          &Mmc5::Poke_5004 );
				Map( 0x5006U,          &Mmc5::Poke_5006 );
				Map( 0x5007U,          &Mmc5::Poke_5007 );
				Map( 0x5010U,          &Mmc5::Poke_5010 );
				Map( 0x5011U,          &Mmc5::Poke_5011 );
				Map( 0x5015U,          &Mmc5::Peek_5015, &Mmc5::Poke_5015 );
				Map( 0x5100U,          &Mmc5::Poke_5100 );
				Map( 0x5101U,          &Mmc5::Poke_5101 );
				Map( 0x5102U,          &Mmc5::Poke_5102 );
				Map( 0x5103U,          &Mmc5::Poke_5103 );
				Map( 0x5104U,          &Mmc5::Poke_5104 );
				Map( 0x5105U,          &Mmc5::Poke_5105 );
				Map( 0x5106U,          &Mmc5::Poke_5106 );
				Map( 0x5107U,          &Mmc5::Poke_5107 );
				Map( 0x5113U,          &Mmc5::Poke_5113 );
				Map( 0x5114U, 0x5117U, &Mmc5::Poke_5114 );
				Map( 0x5120U, 0x5127U, &Mmc5::Poke_5120 );
				Map( 0x5128U, 0x512BU, &Mmc5::Poke_5128 );
				Map( 0x5130U,          &Mmc5::Poke_5130 );
				Map( 0x5200U,          &Mmc5::Poke_5200 );
				Map( 0x5201U,          &Mmc5::Poke_5201 );
				Map( 0x5202U,          &Mmc5::Poke_5202 );
				Map( 0x5203U,          &Mmc5::Poke_5203 );
				Map( 0x5204U,          &Mmc5::Peek_5204, &Mmc5::Poke_5204 );
				Map( 0x5205U,          &Mmc5::Peek_5205, &Mmc5::Poke_5205 );
				Map( 0x5206U,          &Mmc5::Peek_5206, &Mmc5::Poke_5206 );
				Map( 0x5C00U, 0x5FFFU, &Mmc5::Peek_5C00, &Mmc5::Poke_5C00 );
				Map( 0x6000U, 0x7FFFU, &Mmc5::Peek_6000, &Mmc5::Poke_6000 );
				Map( 0x8000U, 0x9FFFU, &Mmc5::Peek_8000, &Mmc5::Poke_8000 );
				Map( 0xA000U, 0xBFFFU, &Mmc5::Peek_A000, &Mmc5::Poke_A000 );
				Map( 0xC000U, 0xDFFFU, &Mmc5::Peek_C000, &Mmc5::Poke_C000 );

				p2001 = cpu.Map( 0x2001 );

				for (uint i=0x2001; i < 0x4000; i += 0x8)
					cpu.Map( i ).Set( this, &Mmc5::Peek_2001, &Mmc5::Poke_2001 );

				for (uint i=0; i < 2; ++i)
					ciRam[i] = nmt.Source().Mem(SIZE_1K * i);

				exRam.Reset( hard );
				flow.Reset();
				banks.Reset();
				regs.Reset();
				irq.Reset();
				filler.Reset();
				spliter.Reset();

				UpdatePrg();
				UpdateChrA();
				UpdateRenderMethod();
			}

			inline bool Mmc5::Sound::Square::CanOutput() const
			{
				return lengthCounter.GetCount() && waveLength >= MIN_FRQ;
			}

			void Mmc5::Sound::Square::UpdateSettings(const uint fixed)
			{
				active = CanOutput();
				frequency = (waveLength + 1UL) * fixed * 2;
			}

			bool Mmc5::Sound::UpdateSettings()
			{
				output = GetVolume(EXT_MMC5);

				GetOscillatorClock( rate, fixed );

				for (uint i=0; i < NUM_SQUARES; ++i)
					square[i].UpdateSettings( fixed );

				quarterClock = (GetModel() == CPU_RP2A03 ? RP2A03_M2_QUARTER : RP2A07_M2_QUARTER);

				dcBlocker.Reset();

				return output;
			}

			void Mmc5::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'M','M','5'>::V );

				{
					const byte data[32] =
					{
						regs.prgMode | (regs.chrMode << 2) | (regs.exRamMode << 4),
						banks.prg[0],
						banks.prg[1],
						banks.prg[2],
						banks.prg[3],
						banks.security & (Banks::READABLE_6|Banks::WRITABLE_6|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B),
						banks.nmt,
						banks.chrA[0] & 0xFFU,
						banks.chrA[1] & 0xFFU,
						banks.chrA[2] & 0xFFU,
						banks.chrA[3] & 0xFFU,
						banks.chrA[4] & 0xFFU,
						banks.chrA[5] & 0xFFU,
						banks.chrA[6] & 0xFFU,
						banks.chrA[7] & 0xFFU,
						banks.chrB[0] & 0xFFU,
						banks.chrB[1] & 0xFFU,
						banks.chrB[2] & 0xFFU,
						banks.chrB[3] & 0xFFU,
						uint(banks.chrA[0]) >> 8 | uint(banks.chrA[1]) >> 8 << 2 | uint(banks.chrA[2]) >> 8 << 4 | uint(banks.chrA[3]) >> 8 << 6,
						uint(banks.chrA[4]) >> 8 | uint(banks.chrA[5]) >> 8 << 2 | uint(banks.chrA[6]) >> 8 << 4 | uint(banks.chrA[7]) >> 8 << 6,
						uint(banks.chrB[0]) >> 8 | uint(banks.chrB[1]) >> 8 << 2 | uint(banks.chrB[2]) >> 8 << 4 | uint(banks.chrB[3]) >> 8 << 6,
						(banks.chrHigh >> 6) | (banks.lastChr != Banks::LAST_CHR_A ? 0x80 : 0x00),
						filler.tile,
						(filler.attribute & 0x3) | (spliter.tile >> 2 & 0xF8),
						exRam.tile,
						spliter.ctrl,
						spliter.yStart,
						spliter.chrBank >> 12,
						spliter.tile & 0x1F,
						spliter.x,
						spliter.y
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				}

				{
					const byte data[2] = { regs.mul[0], regs.mul[1] };
					state.Begin( AsciiId<'M','U','L'>::V ).Write( data ).End();
				}

				{
					const byte data[2] =
					{
						irq.state,
						irq.target
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
				}

				state.Begin( AsciiId<'R','A','M'>::V ).Compress( exRam.mem ).End();

				sound.SaveState( state, AsciiId<'S','N','D'>::V );

				state.End();
			}

			void Mmc5::SubLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'M','M','5'>::V) );

				if (baseChunk == AsciiId<'M','M','5'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:
							{
								State::Loader::Data<32> data( state );

								regs.prgMode = data[0] >> 0 & Regs::PRG_MODE;
								regs.chrMode = data[0] >> 2 & Regs::CHR_MODE;
								regs.exRamMode = data[0] >> 4 & Regs::EXRAM_MODE;

								for (uint i=0; i < 4; ++i)
									banks.prg[i] = data[1+i];

								banks.security = data[5] & (Banks::READABLE_6|Banks::WRITABLE_6|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B);
								banks.nmt = data[6];

								for (uint i=0; i < 8; ++i)
									banks.chrA[i] = data[7+i] | (data[19+(i/4)] & Regs::CHR_HIGH) << 8;

								for (uint i=0; i < 4; ++i)
									banks.chrB[i] = data[15+i] | (data[21+(i/4)] & Regs::CHR_HIGH) << 8;

								banks.chrHigh = (data[22] & Regs::CHR_HIGH) << 6;
								banks.lastChr = (data[22] & 0x80) ? Banks::LAST_CHR_B : Banks::LAST_CHR_A;

								filler.tile = data[23];
								filler.attribute = Filler::squared[data[24] & 0x3];

								exRam.tile = data[25];

								spliter.ctrl = data[26];
								spliter.yStart = NST_MIN(data[27],239);
								spliter.chrBank = data[28] << 12;
								spliter.tile = (data[29] & 0x1F) | (data[24] << 2 & 0x3E0);
								spliter.x = data[30] & 0x1F;
								spliter.y = NST_MIN(data[31],239);

								UpdatePrg();

								if (banks.lastChr == Banks::LAST_CHR_A)
									UpdateChrA();
								else
									UpdateChrB();

								UpdateRenderMethod();
								break;
							}

							case AsciiId<'M','U','L'>::V:
							{
								State::Loader::Data<2> data( state );

								regs.mul[0] = data[0];
								regs.mul[1] = data[1];
								break;
							}

							case AsciiId<'I','R','Q'>::V:
							{
								State::Loader::Data<2> data( state );

								NST_VERIFY( !(data[0] & Irq::FRAME) );

								irq.state = data[0] & (Irq::HIT|Irq::ENABLED);
								irq.target = data[1];
								break;
							}

							case AsciiId<'R','A','M'>::V:

								state.Uncompress( exRam.mem );
								break;

							case AsciiId<'S','N','D'>::V:

								sound.LoadState( state );
								break;
						}

						state.End();
					}
				}
			}

			void Mmc5::Sound::SaveState(State::Saver& state,const dword baseChunk) const
			{
				state.Begin( baseChunk );

				state.Begin( AsciiId<'C','L','K'>::V ).Write8( atHalfClock ).End();

				square[0].SaveState( state, AsciiId<'S','Q','0'>::V  );
				square[1].SaveState( state, AsciiId<'S','Q','1'>::V );
				pcm.SaveState( state, AsciiId<'P','C','M'>::V );

				state.End();
			}

			void Mmc5::Sound::LoadState(State::Loader& state)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'C','L','K'>::V:

							atHalfClock = state.Read8() & 0x1;
							break;

						case AsciiId<'S','Q','0'>::V:

							square[0].LoadState( state, fixed );
							break;

						case AsciiId<'S','Q','1'>::V:

							square[1].LoadState( state, fixed );
							break;

						case AsciiId<'P','C','M'>::V:

							pcm.LoadState( state );
							break;
					}

					state.End();
				}
			}

			void Mmc5::Sound::Square::SaveState(State::Saver& state,const dword chunk) const
			{
				state.Begin( chunk );

				{
					const byte data[3] =
					{
						waveLength & 0xFF,
						waveLength >> 8,
						duty
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
				}

				lengthCounter.SaveState( state, AsciiId<'L','E','N'>::V );
				envelope.SaveState( state, AsciiId<'E','N','V'>::V );

				state.End();
			}

			void Mmc5::Sound::Square::LoadState(State::Loader& state,const dword fixed)
			{
				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'R','E','G'>::V:

							waveLength = state.Read16() & 0x7FF;
							duty = state.Read8() & 0x3;
							break;

						case AsciiId<'L','E','N'>::V:

							lengthCounter.LoadState( state );
							break;

						case AsciiId<'E','N','V'>::V:

							envelope.LoadState( state );
							break;
					}

					state.End();
				}

				step = 0;
				timer = 0;
				frequency = (waveLength + 1UL) * fixed * 2;
				active = CanOutput();
			}

			void Mmc5::Sound::Pcm::SaveState(State::Saver& state,const dword chunk) const
			{
				state.Begin( chunk ).Write16( (enabled != 0) | dword(amp / VOLUME) << 8 ).End();
			}

			void Mmc5::Sound::Pcm::LoadState(State::Loader& state)
			{
				const uint data = state.Read16();

				enabled = data & 0x1;
				amp = (data >> 8) * VOLUME;
				sample = enabled ? amp : 0;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Mmc5::VBlank()
			{
				NST_ASSERT( flow.cycles == 0 );

				if (ppu.GetClock() == PPU_RP2C02_CC)
					flow.cycles = PPU_RP2C02_HVINT;
				else
					flow.cycles = PPU_RP2C07_HVINT;

				if (flow.cycles <= cpu.GetCycles())
					HDummy();
				else
					flow.phase = &Mmc5::HDummy;
			}

			void Mmc5::HDummy()
			{
				if (ppu.IsEnabled())
					++irq.count;

				if (ppu.GetClock() == PPU_RP2C02_CC)
					flow.cycles += PPU_RP2C02_HSYNC - (ppu.IsShortFrame() ? PPU_RP2C02_CC : 0);
				else
					flow.cycles += PPU_RP2C07_HSYNC;

				if (flow.cycles <= cpu.GetCycles())
					HActive0();
				else
					flow.phase = &Mmc5::HActive0;
			}

			void Mmc5::HActive0()
			{
				if (ppu.IsEnabled())
				{
					++irq.count;
					irq.state = (irq.state & Irq::ENABLED) | Irq::FRAME;
					cpu.ClearIRQ();
				}

				if (ppu.GetClock() == PPU_RP2C02_CC)
					flow.cycles += PPU_RP2C02_HSYNC;
				else
					flow.cycles += PPU_RP2C07_HSYNC;

				flow.scanline = 0;

				if (flow.cycles <= cpu.GetCycles())
					HActiveX();
				else
					flow.phase = &Mmc5::HActiveX;
			}

			void Mmc5::HActiveX()
			{
				for (;;)
				{
					++flow.scanline;

					if (ppu.IsEnabled())
					{
						if (++irq.count == irq.target && irq.target)
							irq.state |= Irq::HIT;

						if ((irq.state & Irq::SIGNAL_HIT) == Irq::SIGNAL_HIT)
							cpu.DoIRQ( Cpu::IRQ_EXT, flow.cycles );
					}

					if (ppu.GetClock() == PPU_RP2C02_CC)
						flow.cycles += PPU_RP2C02_HSYNC;
					else
						flow.cycles += PPU_RP2C07_HSYNC;

					if (flow.scanline < 240)
					{
						if (flow.cycles > cpu.GetCycles())
							break;
					}
					else
					{
						irq.count = 0U-2U;
						flow.cycles = Cpu::CYCLE_MAX;
						irq.state &= (Irq::ENABLED|Irq::HIT);

						ppu.Update();

						banks.fetchMode = Banks::FETCH_MODE_NONE;
						spliter.inside = false;

						if (banks.lastChr == Banks::LAST_CHR_A)
							UpdateChrA();
						else
							UpdateChrB();

						break;
					}
				}
			}

			inline void Mmc5::Update()
			{
				if (flow.cycles <= cpu.GetCycles())
					(*this.*flow.phase)();
			}

			void Mmc5::Sync(Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_END_FRAME)
				{
					Update();

					flow.cycles = 0;
					flow.phase = &Mmc5::VBlank;
				}

				Board::Sync( event, controllers );
			}

			NES_HOOK(Mmc5,Cpu)
			{
				Update();
			}

			NES_HOOK(Mmc5,HActive)
			{
				banks.fetchMode = Banks::FETCH_MODE_BG;

				if (ppu.IsEnabled())
				{
					spliter.x = 0x1F;

					if (ppu.GetPixelCycles() != ~0U)
					{
						if (spliter.y < 239)
							++spliter.y;
						else
							spliter.y = 0;
					}
					else
					{
						spliter.y = spliter.yStart;
					}

					if (banks.lastChr != Banks::LAST_CHR_A || ppu.GetCtrl0(Regs::PPU_CTRL0_SP8X16))
						UpdateChrB();
					else
						UpdateChrA();
				}
			}

			NES_HOOK(Mmc5,HBlank)
			{
				banks.fetchMode = Banks::FETCH_MODE_SP;
				spliter.inside = false;

				if (ppu.IsEnabled())
				{
					if (banks.lastChr == Banks::LAST_CHR_A || ppu.GetCtrl0(Regs::PPU_CTRL0_SP8X16))
						UpdateChrA();
					else
						UpdateChrB();
				}
			}

			inline uint Mmc5::Banks::Wrk::operator [] (uint i) const
			{
				NST_ASSERT( i < 8 );
				return banks[i];
			}

			template<uint ADDRESS>
			void Mmc5::SwapPrg8Ex(uint bank)
			{
				enum
				{
					ROM = uint(Banks::READABLE_8) << (ADDRESS / SIZE_8K),
					RAM = uint(Banks::WRITABLE_8) << (ADDRESS / SIZE_8K) | ROM
				};

				// GCC goes banana without the explicit cast

				if (bank & Regs::PRG_ROM_SELECT)
				{
					banks.security = banks.security & ~uint(RAM) | ROM;
					static_cast<Prg::SourceProxy>(prg.Source(0)).SwapBank<SIZE_8K,ADDRESS>( bank & Regs::PRG_ROM_BANK );
				}
				else if (Banks::Wrk::INVALID != (bank = banks.wrk[bank & Regs::PRG_RAM_BANK]))
				{
					banks.security |= RAM;
					static_cast<Prg::SourceProxy>(prg.Source(1)).SwapBank<SIZE_8K,ADDRESS>( bank );
				}
				else
				{
					banks.security &= ~uint(RAM);
					NST_DEBUG_MSG("MMC5 open bus PRG bankswitch!");
				}
			}

			void Mmc5::UpdatePrg()
			{
				enum
				{
					ROM_8_A_C = Banks::READABLE_8|Banks::READABLE_A|Banks::READABLE_C,
					ROM_C     = Banks::READABLE_C,
					RAM_8_A_C = Banks::WRITABLE_8|Banks::WRITABLE_A|Banks::WRITABLE_C|ROM_8_A_C,
					RAM_C     = Banks::WRITABLE_C|ROM_C
				};

				switch (regs.prgMode & Regs::PRG_MODE)
				{
					case Regs::PRG_MODE_32K:

						banks.security = banks.security & ~uint(RAM_8_A_C) | ROM_8_A_C;
						prg.SwapBank<SIZE_32K,0x0000>( banks.prg[3] >> 2 );
						break;

					case Regs::PRG_MODE_16K:

						banks.security = banks.security & ~uint(RAM_C) | ROM_C;
						SwapPrg8Ex<0x0000>( banks.prg[1] & 0xFEU );
						SwapPrg8Ex<0x2000>( banks.prg[1] | 0x01U );
						prg.SwapBank<SIZE_16K,0x4000>( banks.prg[3] >> 1 );
						break;

					case Regs::PRG_MODE_16K_8K:

						SwapPrg8Ex<0x0000>( banks.prg[1] & 0xFEU );
						SwapPrg8Ex<0x2000>( banks.prg[1] | 0x01U );
						SwapPrg8Ex<0x4000>( banks.prg[2] );
						prg.SwapBank<SIZE_8K,0x6000>( banks.prg[3] );
						break;

					case Regs::PRG_MODE_8K:

						SwapPrg8Ex<0x0000>( banks.prg[0] );
						SwapPrg8Ex<0x2000>( banks.prg[1] );
						SwapPrg8Ex<0x4000>( banks.prg[2] );
						prg.SwapBank<SIZE_8K,0x6000>( banks.prg[3] );
						break;
				}
			}

			void Mmc5::UpdateChrA() const
			{
				switch (regs.chrMode)
				{
					case Regs::CHR_MODE_8K:

						chr.SwapBank<SIZE_8K,0x0000>( banks.chrA[7] );
						break;

					case Regs::CHR_MODE_4K:

						chr.SwapBanks<SIZE_4K,0x0000>( banks.chrA[3], banks.chrA[7] );
						break;

					case Regs::CHR_MODE_2K:

						chr.SwapBanks<SIZE_2K,0x0000>( banks.chrA[1], banks.chrA[3], banks.chrA[5], banks.chrA[7] );
						break;

					case Regs::CHR_MODE_1K:

						chr.SwapBanks<SIZE_1K,0x0000>( banks.chrA[0], banks.chrA[1], banks.chrA[2], banks.chrA[3], banks.chrA[4], banks.chrA[5], banks.chrA[6], banks.chrA[7] );
						break;
				}
			}

			void Mmc5::UpdateChrB() const
			{
				switch (regs.chrMode)
				{
					case Regs::CHR_MODE_8K:

						chr.SwapBank<SIZE_8K,0x0000>( banks.chrB[3] );
						break;

					case Regs::CHR_MODE_4K:

						chr.SwapBanks<SIZE_4K,0x0000>( banks.chrB[3], banks.chrB[3] );
						break;

					case Regs::CHR_MODE_2K:

						chr.SwapBanks<SIZE_2K,0x0000>( banks.chrB[1], banks.chrB[3], banks.chrB[1], banks.chrB[3] );
						break;

					case Regs::CHR_MODE_1K:

						chr.SwapBanks<SIZE_1K,0x0000>( banks.chrB[0], banks.chrB[1], banks.chrB[2], banks.chrB[3], banks.chrB[0], banks.chrB[1], banks.chrB[2], banks.chrB[3] );
						break;
				}
			}

			bool Mmc5::ClockSpliter()
			{
				NST_ASSERT( spliter.ctrl & Spliter::CTRL_ENABLED );

				if (banks.fetchMode == Banks::FETCH_MODE_BG)
				{
					spliter.x = (spliter.x + 1) & 0x1F;

					if
					(
						(spliter.ctrl & Spliter::CTRL_RIGHT_SIDE) ? (spliter.x >= (spliter.ctrl & Spliter::CTRL_START)) :
																	(spliter.x <  (spliter.ctrl & Spliter::CTRL_START))
					)
					{
						spliter.tile = ((spliter.y & 0xF8) << 2) | spliter.x;
						spliter.inside = true;
						return true;
					}

					spliter.inside = false;
				}

				return false;
			}

			template<>
			inline uint Mmc5::FetchByte<Mmc5::NT_CIRAM_0>(uint address) const
			{
				return ciRam[0][address];
			}

			template<>
			inline uint Mmc5::FetchByte<Mmc5::NT_CIRAM_1>(uint address) const
			{
				return ciRam[1][address];
			}

			template<>
			inline uint Mmc5::FetchByte<Mmc5::NT_EXRAM>(uint address) const
			{
				return exRam.mem[address];
			}

			template<>
			inline uint Mmc5::FetchByte<Mmc5::NT_FILL>(uint) const
			{
				return filler.tile;
			}

			template<>
			inline uint Mmc5::FetchByte<Mmc5::NT_ZERO>(uint) const
			{
				return 0;
			}

			template<>
			inline uint Mmc5::FetchByte<Mmc5::AT_FILL>(uint) const
			{
				return filler.attribute;
			}

			template<>
			inline uint Mmc5::FetchByte<Mmc5::AT_EXRAM>(uint) const
			{
				return Filler::squared[exRam.tile >> 6];
			}

			template<Mmc5::FetchType NT>
			inline uint Mmc5::FetchNt(uint address) const
			{
				return FetchByte<NT>( address );
			}

			template<Mmc5::FetchType NT>
			inline uint Mmc5::FetchNtExt(uint address)
			{
				exRam.tile = exRam.mem[address];
				return FetchByte<NT>( address );
			}

			template<Mmc5::FetchType NT>
			inline uint Mmc5::FetchNtSplit(uint address)
			{
				if (ClockSpliter())
					return exRam.mem[spliter.tile];
				else
					return FetchByte<NT>( address );
			}

			template<Mmc5::FetchType NT>
			inline uint Mmc5::FetchNtExtSplit(uint address)
			{
				if (ClockSpliter())
				{
					return exRam.mem[spliter.tile];
				}
				else
				{
					exRam.tile = exRam.mem[address];
					return FetchByte<NT>( address );
				}
			}

			uint Mmc5::GetSpliterAttribute() const
			{
				return Filler::squared[(exRam.mem[0x3C0 + (spliter.tile >> 4 & 0x38) + (spliter.tile >> 2 & 0x7)] >> ((spliter.tile >> 4 & 0x4) | (spliter.tile & 0x2))) & 0x3];
			}

			template<Mmc5::FetchType AT>
			inline uint Mmc5::FetchAtSplit(uint address) const
			{
				if (spliter.inside)
					return GetSpliterAttribute();
				else
					return FetchByte<AT>( address );
			}

			NES_ACCESSOR( Mmc5, Nt_CiRam_0         ) { return FetchNt         < NT_CIRAM_0 >( address ); }
			NES_ACCESSOR( Mmc5, Nt_CiRam_1         ) { return FetchNt         < NT_CIRAM_1 >( address ); }
			NES_ACCESSOR( Mmc5, Nt_ExRam           ) { return FetchNt         < NT_EXRAM   >( address ); }
			NES_ACCESSOR( Mmc5, Nt_Fill            ) { return FetchNt         < NT_FILL    >( address ); }
			NES_ACCESSOR( Mmc5, Nt_Zero            ) { return FetchNt         < NT_ZERO    >( address ); }
			NES_ACCESSOR( Mmc5, At_Fill            ) { return FetchNt         < AT_FILL    >( address ); }
			NES_ACCESSOR( Mmc5, At_ExRam           ) { return FetchNt         < AT_EXRAM   >( address ); }
			NES_ACCESSOR( Mmc5, NtExt_CiRam_0      ) { return FetchNtExt      < NT_CIRAM_0 >( address ); }
			NES_ACCESSOR( Mmc5, NtExt_CiRam_1      ) { return FetchNtExt      < NT_CIRAM_1 >( address ); }
			NES_ACCESSOR( Mmc5, NtExt_ExRam        ) { return FetchNtExt      < NT_EXRAM   >( address ); }
			NES_ACCESSOR( Mmc5, NtExt_Fill         ) { return FetchNtExt      < NT_FILL    >( address ); }
			NES_ACCESSOR( Mmc5, NtSplit_CiRam_0    ) { return FetchNtSplit    < NT_CIRAM_0 >( address ); }
			NES_ACCESSOR( Mmc5, NtSplit_CiRam_1    ) { return FetchNtSplit    < NT_CIRAM_1 >( address ); }
			NES_ACCESSOR( Mmc5, NtSplit_ExRam      ) { return FetchNtSplit    < NT_EXRAM   >( address ); }
			NES_ACCESSOR( Mmc5, NtSplit_Fill       ) { return FetchNtSplit    < NT_FILL    >( address ); }
			NES_ACCESSOR( Mmc5, AtSplit_CiRam_0    ) { return FetchAtSplit    < NT_CIRAM_0 >( address ); }
			NES_ACCESSOR( Mmc5, AtSplit_CiRam_1    ) { return FetchAtSplit    < NT_CIRAM_1 >( address ); }
			NES_ACCESSOR( Mmc5, AtSplit_ExRam      ) { return FetchAtSplit    < NT_EXRAM   >( address ); }
			NES_ACCESSOR( Mmc5, AtSplit_Fill       ) { return FetchAtSplit    < NT_FILL    >( address ); }
			NES_ACCESSOR( Mmc5, NtExtSplit_CiRam_0 ) { return FetchNtExtSplit < NT_CIRAM_0 >( address ); }
			NES_ACCESSOR( Mmc5, NtExtSplit_CiRam_1 ) { return FetchNtExtSplit < NT_CIRAM_1 >( address ); }
			NES_ACCESSOR( Mmc5, NtExtSplit_ExRam   ) { return FetchNtExtSplit < NT_EXRAM   >( address ); }
			NES_ACCESSOR( Mmc5, NtExtSplit_Fill    ) { return FetchNtExtSplit < NT_FILL    >( address ); }

			NES_ACCESSOR(Mmc5,CRom)
			{
				return chr.Peek( address );
			}

			uint Mmc5::GetSpliterPattern(uint address) const
			{
				return *chr.Source().Mem( spliter.chrBank + (address & 0xFFF) );
			}

			uint Mmc5::GetExtPattern(uint address) const
			{
				return *chr.Source().Mem( (((exRam.tile & Regs::EXRAM_EXT_CHR_BANK) + banks.chrHigh) << 12) + (address & 0xFFF) );
			}

			NES_ACCESSOR(Mmc5,CRomExt)
			{
				if (banks.fetchMode == Banks::FETCH_MODE_BG)
					return GetExtPattern( address );
				else
					return chr.Peek( address );
			}

			NES_ACCESSOR(Mmc5,CRomSplit)
			{
				if (spliter.inside)
					return GetSpliterPattern( address );
				else
					return chr.Peek( address );
			}

			NES_ACCESSOR(Mmc5,CRomExtSplit)
			{
				if (spliter.inside)
				{
					return GetSpliterPattern( address );
				}
				else if (banks.fetchMode == Banks::FETCH_MODE_BG)
				{
					return GetExtPattern( address );
				}
				else
				{
					return chr.Peek( address );
				}
			}

			void Mmc5::UpdateRenderMethod()
			{
				ppu.Update();

				const uint method = regs.exRamMode | (spliter.ctrl >> 5 & 0x4);

				{
					static const Io::Accessor::Type<Mmc5>::Function chrMethods[8] =
					{
						&Mmc5::Access_CRom,         // PPU NT
						&Mmc5::Access_CRomExt,      // PPU EXT
						&Mmc5::Access_CRom,         // CPU EXRAM
						&Mmc5::Access_CRom,         // CPU EXROM
						&Mmc5::Access_CRomSplit,    // PPU NT + SPLIT
						&Mmc5::Access_CRomExtSplit, // PPU EXT + SPLIT
						&Mmc5::Access_CRom,         // CPU EXRAM + denied SPLIT
						&Mmc5::Access_CRom          // CPU EXROM + denied SPLIT
					};

					chr.SetAccessors
					(
						this,
						chrMethods[method],
						chrMethods[method]
					);
				}

				uint bank = banks.nmt;

				{
					static const Io::Accessor::Type<Mmc5>::Function nmtMethods[8][4][2] =
					{
						{   // PPU NT
							{ &Mmc5::Access_Nt_CiRam_0,         &Mmc5::Access_Nt_CiRam_0      },
							{ &Mmc5::Access_Nt_CiRam_1,         &Mmc5::Access_Nt_CiRam_1      },
							{ &Mmc5::Access_Nt_ExRam,           &Mmc5::Access_Nt_ExRam        },
							{ &Mmc5::Access_Nt_Fill,            &Mmc5::Access_At_Fill         }
						},
						{   // PPU EXT
							{ &Mmc5::Access_NtExt_CiRam_0,      &Mmc5::Access_At_ExRam        },
							{ &Mmc5::Access_NtExt_CiRam_1,      &Mmc5::Access_At_ExRam        },
							{ &Mmc5::Access_NtExt_ExRam,        &Mmc5::Access_At_ExRam        },
							{ &Mmc5::Access_NtExt_Fill,         &Mmc5::Access_At_ExRam        }
						},
						{   // CPU EXRAM
							{ &Mmc5::Access_Nt_CiRam_0,         &Mmc5::Access_Nt_CiRam_0      },
							{ &Mmc5::Access_Nt_CiRam_1,         &Mmc5::Access_Nt_CiRam_1      },
							{ &Mmc5::Access_Nt_Zero,            &Mmc5::Access_Nt_Zero         },
							{ &Mmc5::Access_Nt_Fill,            &Mmc5::Access_At_Fill         }
						},
						{   // CPU EXROM
							{ &Mmc5::Access_Nt_CiRam_0,         &Mmc5::Access_Nt_CiRam_0      },
							{ &Mmc5::Access_Nt_CiRam_1,         &Mmc5::Access_Nt_CiRam_1      },
							{ &Mmc5::Access_Nt_Zero,            &Mmc5::Access_Nt_Zero         },
							{ &Mmc5::Access_Nt_Fill,            &Mmc5::Access_At_Fill         }
						},
						{   // PPU NT + SPLIT
							{ &Mmc5::Access_NtSplit_CiRam_0,    &Mmc5::Access_AtSplit_CiRam_0 },
							{ &Mmc5::Access_NtSplit_CiRam_1,    &Mmc5::Access_AtSplit_CiRam_1 },
							{ &Mmc5::Access_NtSplit_ExRam,      &Mmc5::Access_AtSplit_ExRam   },
							{ &Mmc5::Access_NtSplit_Fill,       &Mmc5::Access_AtSplit_Fill    }
						},
						{   // PPU EXT + SPLIT
							{ &Mmc5::Access_NtExtSplit_CiRam_0, &Mmc5::Access_AtSplit_ExRam   },
							{ &Mmc5::Access_NtExtSplit_CiRam_1, &Mmc5::Access_AtSplit_ExRam   },
							{ &Mmc5::Access_NtExtSplit_ExRam,   &Mmc5::Access_AtSplit_ExRam   },
							{ &Mmc5::Access_NtExtSplit_Fill,    &Mmc5::Access_AtSplit_ExRam   }
						},
						{   // CPU EXRAM + denied SPLIT
							{ &Mmc5::Access_Nt_CiRam_0,         &Mmc5::Access_Nt_CiRam_0      },
							{ &Mmc5::Access_Nt_CiRam_1,         &Mmc5::Access_Nt_CiRam_1      },
							{ &Mmc5::Access_Nt_Zero,            &Mmc5::Access_Nt_Zero         },
							{ &Mmc5::Access_Nt_Fill,            &Mmc5::Access_At_Fill         }
						},
						{   // CPU EXROM + denied SPLIT
							{ &Mmc5::Access_Nt_CiRam_0,         &Mmc5::Access_Nt_CiRam_0      },
							{ &Mmc5::Access_Nt_CiRam_1,         &Mmc5::Access_Nt_CiRam_1      },
							{ &Mmc5::Access_Nt_Zero,            &Mmc5::Access_Nt_Zero         },
							{ &Mmc5::Access_Nt_Fill,            &Mmc5::Access_At_Fill         }
						}
					};

					nmt.SetAccessors
					(
						this,
						nmtMethods[method][bank >> 0 & Regs::NMT_MODE],
						nmtMethods[method][bank >> 2 & Regs::NMT_MODE],
						nmtMethods[method][bank >> 4 & Regs::NMT_MODE],
						nmtMethods[method][bank >> 6 & Regs::NMT_MODE]
					);
				}

				for (uint address=0; address < SIZE_4K; address += SIZE_1K, bank >>= 2)
				{
					static const byte securities[4][4][2] =
					{
						{ {0,0}, {0,1}, {1,0}, {0,0} },
						{ {0,0}, {0,1}, {1,0}, {0,0} },
						{ {0,0}, {0,1}, {0,0}, {0,0} },
						{ {0,0}, {0,1}, {0,0}, {0,0} }
					};

					nmt.Source( securities[regs.exRamMode][bank & Regs::NMT_MODE][0] ).SwapBank<SIZE_1K>
					(
						address,
						securities[regs.exRamMode][bank & Regs::NMT_MODE][1]
					);
				}
			}

			NES_POKE_AD(Mmc5,2001)
			{
				Update();

				if (!(data & Regs::PPU_CTRL1_ENABLED))
				{
					irq.count = 0U-2U;
					irq.state &= (Irq::HIT|Irq::ENABLED);
					banks.fetchMode = Banks::FETCH_MODE_NONE;
					spliter.inside = false;
				}

				p2001.Poke( address, data );
			}

			NES_PEEK_A(Mmc5,2001)
			{
				return p2001.Peek( address );
			}

			NES_POKE_D(Mmc5,5000)
			{
				sound.WriteSquareReg0( 0, data );
			}

			NES_POKE_D(Mmc5,5002)
			{
				sound.WriteSquareReg1( 0, data );
			}

			NES_POKE_D(Mmc5,5003)
			{
				sound.WriteSquareReg2( 0, data );
			}

			NES_POKE_D(Mmc5,5004)
			{
				sound.WriteSquareReg0( 1, data );
			}

			NES_POKE_D(Mmc5,5006)
			{
				sound.WriteSquareReg1( 1, data );
			}

			NES_POKE_D(Mmc5,5007)
			{
				sound.WriteSquareReg2( 1, data );
			}

			NES_POKE_D(Mmc5,5010)
			{
				sound.WritePcmReg0( data );
			}

			NES_POKE_D(Mmc5,5011)
			{
				sound.WritePcmReg1( data );
			}

			NES_PEEK(Mmc5,5015)
			{
				return sound.ReadCtrl();
			}

			NES_POKE_D(Mmc5,5015)
			{
				sound.WriteCtrl( data );
			}

			NES_POKE_D(Mmc5,5100)
			{
				data &= Regs::PRG_MODE;

				if (regs.prgMode != data)
				{
					regs.prgMode = data;
					UpdatePrg();
				}
			}

			NES_POKE_D(Mmc5,5101)
			{
				data &= Regs::CHR_MODE;

				if (regs.chrMode != data)
				{
					ppu.Update();

					regs.chrMode = data;

					if (!ppu.GetCtrl0(Regs::PPU_CTRL0_SP8X16) || !ppu.IsActive())
					{
						if (banks.lastChr == Banks::LAST_CHR_A)
							UpdateChrA();
						else
							UpdateChrB();
					}
				}
			}

			NES_POKE_D(Mmc5,5102)
			{
				if (data == Regs::WRK_WRITABLE_A)
					banks.security |= Regs::WRK_WRITABLE_A;
				else
					banks.security &= ~uint(Regs::WRK_WRITABLE_A);
			}

			NES_POKE_D(Mmc5,5103)
			{
				if (data == Regs::WRK_WRITABLE_B)
					banks.security |= Regs::WRK_WRITABLE_B;
				else
					banks.security &= ~uint(Regs::WRK_WRITABLE_B);
			}

			NES_POKE_D(Mmc5,5104)
			{
				data &= Regs::EXRAM_MODE;

				if (regs.exRamMode != data)
				{
					regs.exRamMode = data;
					UpdateRenderMethod();
				}
			}

			NES_POKE_D(Mmc5,5105)
			{
				if (banks.nmt != data)
				{
					banks.nmt = data;
					UpdateRenderMethod();
				}
			}

			NES_POKE_D(Mmc5,5106)
			{
				if (banks.nmt & (banks.nmt << 1))
					ppu.Update();

				filler.tile = data;
			}

			NES_POKE_D(Mmc5,5107)
			{
				if (banks.nmt & (banks.nmt << 1))
					ppu.Update();

				filler.attribute = Filler::squared[data & 0x3];
			}

			NES_POKE_D(Mmc5,5113)
			{
				data = banks.wrk[data & Regs::PRG_RAM_BANK];

				if (data != Banks::Wrk::INVALID)
				{
					banks.security |= Banks::READABLE_6|Banks::WRITABLE_6;
					wrk.SwapBank<SIZE_8K,0x0000>( data );
				}
				else
				{
					banks.security &= ~uint(Banks::READABLE_6|Banks::WRITABLE_6);
				}
			}

			NES_POKE_AD(Mmc5,5114)
			{
				if (banks.prg[address - 0x5114] != data)
				{
					banks.prg[address - 0x5114] = data;
					UpdatePrg();
				}
			}

			NES_POKE_AD(Mmc5,5120)
			{
				data |= banks.chrHigh << 2;
				address &= 0x7;

				if (banks.lastChr != Banks::LAST_CHR_A || banks.chrA[address] != data)
				{
					ppu.Update();

					banks.chrA[address] = data;
					banks.lastChr = Banks::LAST_CHR_A;

					if (!ppu.GetCtrl0(Regs::PPU_CTRL0_SP8X16) || !ppu.IsActive())
						UpdateChrA();
				}
			}

			NES_POKE_AD(Mmc5,5128)
			{
				data |= banks.chrHigh << 2;
				address &= 0x3;

				if (banks.lastChr == Banks::LAST_CHR_A || banks.chrB[address] != data)
				{
					ppu.Update();

					banks.chrB[address] = data;
					banks.lastChr = Banks::LAST_CHR_B;

					if (!ppu.GetCtrl0(Regs::PPU_CTRL0_SP8X16) || !ppu.IsActive())
						UpdateChrB();
				}
			}

			NES_POKE_D(Mmc5,5130)
			{
				data = (data & Regs::CHR_HIGH) << 6;

				if (banks.chrHigh != data)
				{
					ppu.Update();
					banks.chrHigh = data;
				}
			}

			NES_POKE_D(Mmc5,5200)
			{
				if (spliter.ctrl != data)
				{
					ppu.Update();
					spliter.ctrl = data;
					UpdateRenderMethod();
				}
			}

			NES_POKE_D(Mmc5,5201)
			{
				if (data >= 240)
					data -= 16;

				if (spliter.yStart != data)
				{
					ppu.Update();
					spliter.yStart = data;
				}
			}

			NES_POKE_D(Mmc5,5202)
			{
				const dword chrBank = dword(data) << 12;

				if (spliter.chrBank != chrBank)
				{
					ppu.Update();
					spliter.chrBank = chrBank;
				}
			}

			NES_POKE_D(Mmc5,5203)
			{
				Update();
				irq.target = data;
			}

			NES_PEEK(Mmc5,5204)
			{
				Update();

				const uint status = irq.state & (Irq::FRAME|Irq::HIT);
				irq.state &= (Irq::FRAME|Irq::ENABLED);

				cpu.ClearIRQ();

				return status;
			}

			NES_POKE_D(Mmc5,5204)
			{
				Update();

				if (data & 0x80)
				{
					irq.state |= Irq::ENABLED;

					if (irq.state & Irq::HIT)
						cpu.DoIRQ();
				}
				else
				{
					irq.state &= (Irq::HIT|Irq::FRAME);
					cpu.ClearIRQ();
				}
			}

			NES_PEEK(Mmc5,5205)
			{
				return (regs.mul[0] * regs.mul[1]) >> 0 & 0xFF;
			}

			NES_PEEK(Mmc5,5206)
			{
				return (regs.mul[0] * regs.mul[1]) >> 8 & 0xFF;
			}

			NES_POKE_D(Mmc5,5205)
			{
				regs.mul[0] = data;
			}

			NES_POKE_D(Mmc5,5206)
			{
				regs.mul[1] = data;
			}

			NES_PEEK_A(Mmc5,5C00)
			{
				if (regs.exRamMode & Regs::EXRAM_MODE_CPU_RAM)
					return exRam.mem[address - 0x5C00];
				else
					return address >> 8;
			}

			NES_POKE_AD(Mmc5,5C00)
			{
				switch (regs.exRamMode)
				{
					default:

						ppu.Update();
						Update();

						NST_VERIFY( irq.state & Irq::FRAME );

						if (!(irq.state & Irq::FRAME))
							data = 0;

					case Regs::EXRAM_MODE_CPU_RAM:

						exRam.mem[address - 0x5C00] = data;

					case Regs::EXRAM_MODE_CPU_ROM:
						break;
				}
			}

			NES_POKE_AD(Mmc5,6000)
			{
				NST_VERIFY( (banks.security & Banks::CAN_WRITE_6) == Banks::CAN_WRITE_6 );

				if ((banks.security & Banks::CAN_WRITE_6) == Banks::CAN_WRITE_6)
					wrk[0][address - 0x6000] = data;
			}

			NES_POKE_AD(Mmc5,8000)
			{
				NST_VERIFY( (banks.security & Banks::CAN_WRITE_8) == Banks::CAN_WRITE_8 );

				if ((banks.security & Banks::CAN_WRITE_8) == Banks::CAN_WRITE_8)
					prg[0][address - 0x8000] = data;
			}

			NES_POKE_AD(Mmc5,A000)
			{
				NST_VERIFY( (banks.security & Banks::CAN_WRITE_A) == Banks::CAN_WRITE_A );

				if ((banks.security & Banks::CAN_WRITE_A) == Banks::CAN_WRITE_A)
					prg[1][address - 0xA000] = data;
			}

			NES_POKE_AD(Mmc5,C000)
			{
				NST_VERIFY( (banks.security & Banks::CAN_WRITE_C) == Banks::CAN_WRITE_C );

				if ((banks.security & Banks::CAN_WRITE_C) == Banks::CAN_WRITE_C)
					prg[2][address - 0xC000] = data;
			}

			NES_PEEK_A(Mmc5,6000)
			{
				NST_VERIFY( banks.security & Banks::READABLE_6 );
				return (banks.security & Banks::READABLE_6) ? wrk[0][address - 0x6000] : (address >> 8);
			}

			NES_PEEK_A(Mmc5,8000)
			{
				NST_VERIFY( banks.security & Banks::READABLE_8 );
				return (banks.security & Banks::READABLE_8) ? prg[0][address - 0x8000] : (address >> 8);
			}

			NES_PEEK_A(Mmc5,A000)
			{
				NST_VERIFY( banks.security & Banks::READABLE_A );
				return (banks.security & Banks::READABLE_A) ? prg[1][address - 0xA000] : (address >> 8);
			}

			NES_PEEK_A(Mmc5,C000)
			{
				NST_VERIFY( banks.security & Banks::READABLE_C );
				return (banks.security & Banks::READABLE_C) ? prg[2][address - 0xC000] : (address >> 8);
			}

			NST_SINGLE_CALL void Mmc5::Sound::Square::Disable(const uint disable)
			{
				if (disable)
				{
					timer = 0;
					active = 0;
				}

				lengthCounter.Disable( disable );
			}

			void Mmc5::Sound::WriteCtrl(uint data)
			{
				Update();

				data = ~data;

				for (uint i=0; i < NUM_SQUARES; ++i)
					square[i].Disable( data >> i & 0x1 );
			}

			inline uint Mmc5::Sound::Square::GetLengthCounter() const
			{
				return lengthCounter.GetCount();
			}

			uint Mmc5::Sound::ReadCtrl() const
			{
				Update();

				uint data = 0;

				for (uint i=0; i < NUM_SQUARES; ++i)
				{
					if (square[i].GetLengthCounter())
						data |= 0x1U << i;
				}

				return data;
			}

			NST_SINGLE_CALL void Mmc5::Sound::Square::WriteReg0(const uint data)
			{
				envelope.Write( data );
				duty = data >> DUTY_SHIFT;
			}

			NST_SINGLE_CALL void Mmc5::Sound::Square::WriteReg1(const uint data,const uint fixed)
			{
				waveLength &= uint(REG2_WAVELENGTH_HIGH) << 8;
				waveLength |= data;
				frequency = (waveLength + 1UL) * fixed * 2;

				active = CanOutput();
			}

			NST_SINGLE_CALL void Mmc5::Sound::Square::WriteReg2(const uint data,const uint fixed)
			{
				step = 0;
				envelope.ResetClock();

				lengthCounter.Write( data );

				waveLength &= REG1_WAVELENGTH_LOW;
				waveLength |= (data & REG2_WAVELENGTH_HIGH) << 8;
				frequency = (waveLength + 1UL) * fixed * 2;

				active = CanOutput();
			}

			void Mmc5::Sound::WriteSquareReg0(uint index,uint data)
			{
				Update();
				NST_ASSERT( index < NUM_SQUARES );
				square[index].WriteReg0( data );
			}

			void Mmc5::Sound::WriteSquareReg1(uint index,uint data)
			{
				Update();
				NST_ASSERT( index < NUM_SQUARES );
				square[index].WriteReg1( data, fixed );
			}

			void Mmc5::Sound::WriteSquareReg2(uint index,uint data)
			{
				Update();
				NST_ASSERT( index < NUM_SQUARES );
				square[index].WriteReg2( data, fixed );
			}

			NST_SINGLE_CALL void Mmc5::Sound::Pcm::WriteReg0(const uint data)
			{
				enabled = ~data & PCM_DISABLE;
				sample = enabled ? amp : 0;
			}

			NST_SINGLE_CALL void Mmc5::Sound::Pcm::WriteReg1(const uint data)
			{
				amp = data * VOLUME;
				sample = enabled ? amp : 0;
			}

			void Mmc5::Sound::WritePcmReg0(uint data)
			{
				Update();
				pcm.WriteReg0( data );
			}

			void Mmc5::Sound::WritePcmReg1(uint data)
			{
				Update();
				pcm.WriteReg1( data );
			}

			NST_SINGLE_CALL dword Mmc5::Sound::Square::GetSample(const Cycle rate)
			{
				NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );

				if (active)
				{
					dword sum = timer;
					timer -= idword(rate);

					static const byte duties[4][8] =
					{
						{0x1F,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F},
						{0x1F,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F},
						{0x1F,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F},
						{0x00,0x1F,0x1F,0x00,0x00,0x00,0x00,0x00}
					};

					if (timer >= 0)
					{
						return dword(envelope.Volume()) >> duties[duty][step];
					}
					else
					{
						sum >>= duties[duty][step];

						do
						{
							sum += NST_MIN(dword(-timer),frequency) >> duties[duty][step = (step + 1) & 0x7];
							timer += idword(frequency);
						}
						while (timer < 0);

						return (sum * envelope.Volume() + rate/2) / rate;
					}
				}
				else
				{
					return 0;
				}
			}

			inline Mmc5::Sound::Sample Mmc5::Sound::Pcm::GetSample() const
			{
				return sample;
			}

			Mmc5::Sound::Sample Mmc5::Sound::GetSample()
			{
				if (output)
				{
					dword sample = 0;

					for (uint i=0; i < NUM_SQUARES; ++i)
						sample += square[i].GetSample( rate );

					sample += pcm.GetSample();

					return dcBlocker.Apply( sample * 2 * output / DEFAULT_VOLUME );
				}
				else
				{
					return 0;
				}
			}

			NST_SINGLE_CALL void Mmc5::Sound::Square::ClockQuarter()
			{
				envelope.Clock();
			}

			NST_SINGLE_CALL void Mmc5::Sound::Square::ClockHalf()
			{
				if (!envelope.Looping() && lengthCounter.Clock())
					active = false;
			}

			Cycle Mmc5::Sound::Clock(Cycle rateCycles,Cycle rateClock,const Cycle targetCycles)
			{
				rateClock *= quarterClock;

				do
				{
					for (uint i=0; i < NUM_SQUARES; ++i)
						square[i].ClockQuarter();

					if (atHalfClock)
					{
						for (uint i=0; i < NUM_SQUARES; ++i)
							square[i].ClockHalf();
					}

					atHalfClock ^= 1;

					rateCycles += rateClock;
				}
				while (rateCycles <= targetCycles);

				return rateCycles;
			}
		}
	}
}
