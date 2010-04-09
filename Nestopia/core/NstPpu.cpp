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
#include "NstCpu.hpp"
#include "NstPpu.hpp"
#include "NstState.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		const byte Ppu::yuvMaps[4][0x40] =
		{
			{
				0x35, 0x23, 0x16, 0x22, 0x1C, 0x09, 0x2D, 0x15,
				0x20, 0x00, 0x27, 0x05, 0x04, 0x28, 0x08, 0x20,
				0x21, 0x27, 0x07, 0x29, 0x3C, 0x32, 0x36, 0x12,
				0x28, 0x2B, 0x0D, 0x08, 0x10, 0x3D, 0x24, 0x01,
				0x01, 0x31, 0x33, 0x2A, 0x2C, 0x0C, 0x1B, 0x14,
				0x0D, 0x07, 0x34, 0x06, 0x13, 0x02, 0x26, 0x0D,
				0x0D, 0x19, 0x10, 0x0A, 0x39, 0x03, 0x37, 0x17,
				0x09, 0x11, 0x1A, 0x1D, 0x38, 0x25, 0x18, 0x3A
			},
			{
				0x0D, 0x27, 0x18, 0x39, 0x3A, 0x25, 0x1C, 0x31,
				0x16, 0x13, 0x38, 0x34, 0x20, 0x23, 0x3C, 0x1A,
				0x09, 0x21, 0x06, 0x10, 0x1B, 0x29, 0x08, 0x22,
				0x2D, 0x24, 0x01, 0x2B, 0x32, 0x08, 0x0D, 0x03,
				0x04, 0x36, 0x26, 0x33, 0x11, 0x07, 0x10, 0x02,
				0x14, 0x28, 0x00, 0x09, 0x12, 0x0D, 0x28, 0x20,
				0x27, 0x1D, 0x2A, 0x17, 0x0C, 0x01, 0x15, 0x19,
				0x0D, 0x2C, 0x07, 0x37, 0x35, 0x05, 0x0A, 0x3D
			},
			{
				0x14, 0x25, 0x3A, 0x10, 0x1A, 0x20, 0x31, 0x09,
				0x01, 0x0D, 0x36, 0x08, 0x15, 0x10, 0x27, 0x3C,
				0x22, 0x1C, 0x05, 0x12, 0x19, 0x18, 0x17, 0x1B,
				0x00, 0x03, 0x0D, 0x02, 0x16, 0x06, 0x34, 0x35,
				0x23, 0x09, 0x01, 0x37, 0x1D, 0x27, 0x26, 0x20,
				0x29, 0x04, 0x21, 0x24, 0x11, 0x3D, 0x0D, 0x07,
				0x2C, 0x08, 0x39, 0x33, 0x07, 0x2A, 0x28, 0x2D,
				0x0A, 0x0D, 0x32, 0x38, 0x13, 0x2B, 0x28, 0x0C
			},
			{
				0x18, 0x03, 0x1C, 0x28, 0x0D, 0x35, 0x01, 0x17,
				0x10, 0x07, 0x2A, 0x01, 0x36, 0x37, 0x1A, 0x39,
				0x25, 0x08, 0x12, 0x34, 0x0D, 0x2D, 0x06, 0x26,
				0x27, 0x1B, 0x22, 0x19, 0x04, 0x0D, 0x3A, 0x21,
				0x05, 0x0A, 0x07, 0x02, 0x13, 0x14, 0x00, 0x15,
				0x0C, 0x10, 0x11, 0x09, 0x1D, 0x38, 0x3D, 0x24,
				0x33, 0x20, 0x08, 0x16, 0x28, 0x2B, 0x20, 0x3C,
				0x0D, 0x27, 0x23, 0x31, 0x29, 0x32, 0x2C, 0x09
			}
		};

		Ppu::Tiles::Tiles()
		: padding0(0), padding1(0) {}

		Ppu::Oam::Oam()
		: limit(buffer + STD_LINE_SPRITES*4), spriteLimit(true) {}

		Ppu::Output::Output(Video::Screen::Pixel* p)
		: pixels(p) {}

		Ppu::TileLut::TileLut()
		{
			for (uint i=0; i < 0x400; ++i)
			{
				block[i][0] = (i & 0x03) ? (i >> 6 & 0xC) | (i >> 0 & 0x3) : 0;
				block[i][1] = (i & 0x0C) ? (i >> 6 & 0xC) | (i >> 2 & 0x3) : 0;
				block[i][2] = (i & 0x30) ? (i >> 6 & 0xC) | (i >> 4 & 0x3) : 0;
				block[i][3] = (i & 0xC0) ? (i >> 6 & 0xC) | (i >> 6 & 0x3) : 0;
			}
		}

		Ppu::Ppu(Cpu& c)
		:
		cpu    (c),
		output (screen.pixels),
		model  (PPU_RP2C02),
		rgbMap (NULL),
		yuvMap (NULL)
		{
			cycles.one = PPU_RP2C02_CC;
			PowerOff();
		}

		void Ppu::PowerOff()
		{
			Reset( true, false, false );
		}

		void Ppu::Reset(bool hard,bool acknowledged)
		{
			Reset( hard, acknowledged, true );
		}

		void Ppu::Reset(const bool hard,const bool acknowledged,const bool map)
		{
			if (map)
			{
				for (uint i=0x2000; i < 0x4000; i += 0x8)
				{
					cpu.Map( i+0 ).Set( this, i != 0x3000 ? &Ppu::Peek_2xxx : &Ppu::Peek_3000, &Ppu::Poke_2000 );
					cpu.Map( i+1 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2001 );
					cpu.Map( i+2 ).Set( this,               &Ppu::Peek_2002,                   &Ppu::Poke_2xxx );
					cpu.Map( i+3 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2003 );
					cpu.Map( i+4 ).Set( this,               &Ppu::Peek_2004,                   &Ppu::Poke_2004 );
					cpu.Map( i+5 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2005 );
					cpu.Map( i+6 ).Set( this,               &Ppu::Peek_2xxx,                   &Ppu::Poke_2006 );
					cpu.Map( i+7 ).Set( this,               &Ppu::Peek_2007,                   &Ppu::Poke_2007 );
				}

				switch (model)
				{
					case PPU_RC2C05_01:
					case PPU_RC2C05_02:
					case PPU_RC2C05_03:
					case PPU_RC2C05_04:

						if (model == PPU_RC2C05_02)
						{
							for (uint i=0x2002; i < 0x4000; i += 0x8)
								cpu.Map( i ).Set( &Ppu::Peek_2002_RC2C05_02 );
						}
						else if (model == PPU_RC2C05_03)
						{
							for (uint i=0x2002; i < 0x4000; i += 0x8)
								cpu.Map( i ).Set( &Ppu::Peek_2002_RC2C05_03 );
						}
						else
						{
							for (uint i=0x2002; i < 0x4000; i += 0x8)
								cpu.Map( i ).Set( &Ppu::Peek_2002_RC2C05_01_04 );
						}

					case PPU_RC2C05_05:

						for (uint i=0x2000; i < 0x4000; i += 0x8)
						{
							cpu.Map( i+0 ).Set( &Ppu::Poke_2001 );
							cpu.Map( i+1 ).Set( &Ppu::Poke_2000 );
						}
						break;
				}

				cpu.Map( 0x4014U ).Set( this, &Ppu::Peek_4014, &Ppu::Poke_4014 );
			}

			if (hard)
			{
				static const byte powerUpPalette[] =
				{
					0x3F,0x01,0x00,0x01,0x00,0x02,0x02,0x0D,
					0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2C,
					0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,
					0x08,0x3A,0x00,0x02,0x00,0x20,0x2C,0x08
				};

				std::memcpy( palette.ram, powerUpPalette, Palette::SIZE );
				std::memset( oam.ram, Oam::GARBAGE, Oam::SIZE );
				std::memset( nameTable.ram, NameTable::GARBAGE, NameTable::SIZE );

				io.latch = 0;
				io.buffer = Io::BUFFER_GARBAGE;

				regs.status = 0;
				regs.ctrl0 = 0;
				regs.ctrl1 = 0;
				regs.frame = 0;
				regs.oam = 0;

				scroll.latch = 0;
				scroll.xFine = 0;
				scroll.toggle = 0;
				scroll.address = 0;

				output.burstPhase = 0;

				cycles.reset = 0;
				cycles.hClock = HCLOCK_BOOT;
			}
			else if (acknowledged)
			{
				io.buffer = 0;

				regs.status = 0;
				regs.ctrl0 = 0;
				regs.ctrl1 = 0;

				scroll.latch = 0;
				scroll.xFine = 0;
				scroll.toggle = 0;

				cycles.reset = Cpu::CYCLE_MAX;
				cycles.hClock = HCLOCK_BOOT;

				std::memset( oam.ram, Oam::GARBAGE, Oam::SIZE );
			}
			else
			{
				cycles.hClock = HCLOCK_DUMMY;
				cycles.reset = 0;
			}

			if (chr.Source().Empty())
			{
				chr.Source().Set( Ram::RAM, true, false, NameTable::SIZE, nameTable.ram );
				chr.SwapBanks<SIZE_2K,0x0000>(0,0,0,0);
			}

			if (nmt.Source().Empty())
			{
				nmt.Source().Set( Ram::RAM, true, true, NameTable::SIZE, nameTable.ram );
				nmt.SwapBanks<SIZE_2K,0x0000>(0,0);
			}

			chr.ResetAccessors();
			nmt.ResetAccessors();

			cycles.vClock = 0;
			cycles.count = Cpu::CYCLE_MAX;

			scanline = SCANLINE_VBLANK;

			io.address = 0;
			io.pattern = 0;
			io.a12.Unset();

			tiles.pattern[0] = 0;
			tiles.pattern[1] = 0;
			tiles.attribute = 0;
			tiles.index = 8;
			tiles.mask = 0;

			oam.index = 0;
			oam.address = 0;
			oam.latch = 0;
			oam.spriteZeroInLine = false;
			oam.phase = &Ppu::EvaluateSpritesPhase0;
			oam.buffered = oam.buffer;
			oam.visible = oam.output;
			oam.mask = 0;

			output.target = NULL;

			hActiveHook = Hook( this, &Ppu::Hook_Nop );
			hBlankHook = Hook( this, &Ppu::Hook_Nop );

			UpdateStates();

			screen.Clear();
		}

		void Ppu::SetHActiveHook(const Hook& hook)
		{
			hActiveHook = hook;
		}

		void Ppu::SetHBlankHook(const Hook& hook)
		{
			hBlankHook = hook;
		}

		void Ppu::UpdateStates()
		{
			scroll.increase = (regs.ctrl0 & Regs::CTRL0_INC32) ? 32 : 1;
			scroll.pattern = (regs.ctrl0 & Regs::CTRL0_BG_OFFSET) << 8;
			oam.height = (regs.ctrl0 >> 2 & 8) + 8;

			tiles.show[0] = (regs.ctrl1 & Regs::CTRL1_BG_ENABLED) ? 0xFF : 0x00;
			tiles.show[1] = (regs.ctrl1 & Regs::CTRL1_BG_ENABLED_NO_CLIP) == Regs::CTRL1_BG_ENABLED_NO_CLIP ? 0xFF : 0x00;

			oam.show[0] = (regs.ctrl1 & Regs::CTRL1_SP_ENABLED) ? 0xFF : 0x00;
			oam.show[1] = (regs.ctrl1 & Regs::CTRL1_SP_ENABLED_NO_CLIP) == Regs::CTRL1_SP_ENABLED_NO_CLIP ? 0xFF : 0x00;

			UpdatePalette();
		}

		void Ppu::UpdatePalette()
		{
			for (uint i=0, c=Coloring(), e=Emphasis(); i < Palette::SIZE; ++i)
				output.palette[i] = (rgbMap ? rgbMap[palette.ram[i] & uint(Palette::COLOR)] : palette.ram[i]) & c | e;
		}

		void Ppu::SaveState(State::Saver& state,const dword baseChunk) const
		{
			state.Begin( baseChunk );

			{
				const byte data[11] =
				{
					regs.ctrl0,
					regs.ctrl1,
					regs.status,
					scroll.address & 0xFF,
					scroll.address >> 8,
					scroll.latch & 0xFF,
					scroll.latch >> 8,
					scroll.xFine | scroll.toggle << 3,
					regs.oam,
					io.buffer,
					io.latch
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
			}

			state.Begin( AsciiId<'P','A','L'>::V ).Compress( palette.ram   ).End();
			state.Begin( AsciiId<'O','A','M'>::V ).Compress( oam.ram       ).End();
			state.Begin( AsciiId<'N','M','T'>::V ).Compress( nameTable.ram ).End();

			if (model == PPU_RP2C02)
				state.Begin( AsciiId<'F','R','M'>::V ).Write8( (regs.frame & Regs::FRAME_ODD) == 0 ).End();

			if (cycles.hClock == HCLOCK_BOOT)
				state.Begin( AsciiId<'P','O','W'>::V ).Write8( 0x0 ).End();

			state.End();
		}

		void Ppu::LoadState(State::Loader& state)
		{
			cycles.hClock = HCLOCK_DUMMY;
			regs.frame = 0;
			output.burstPhase = 0;

			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case AsciiId<'R','E','G'>::V:
					{
						State::Loader::Data<11> data( state );

						regs.ctrl0     = data[0];
						regs.ctrl1     = data[1];
						regs.status    = data[2] & Regs::STATUS_BITS;
						scroll.address = data[3] | (data[4] << 8 & 0x7F00);
						scroll.latch   = data[5] | (data[6] << 8 & 0x7F00);
						scroll.xFine   = data[7] & 0x7;
						scroll.toggle  = data[7] >> 3 & 0x1;
						regs.oam       = data[8];
						io.buffer      = data[9];
						io.latch       = data[10];

						break;
					}

					case AsciiId<'P','A','L'>::V:

						state.Uncompress( palette.ram );
						break;

					case AsciiId<'O','A','M'>::V:

						state.Uncompress( oam.ram );
						break;

					case AsciiId<'N','M','T'>::V:

						state.Uncompress( nameTable.ram );
						break;

					case AsciiId<'F','R','M'>::V:

						if (model == PPU_RP2C02)
							regs.frame = (state.Read8() & 0x1) ? 0 : Regs::FRAME_ODD;

						break;

					case AsciiId<'P','O','W'>::V:

						cycles.hClock = HCLOCK_BOOT;
						break;
				}

				state.End();
			}

			UpdateStates();
		}

		void Ppu::EnableCpuSynchronization()
		{
			cpu.AddHook( Hook(this,&Ppu::Hook_Sync) );
		}

		void Ppu::ChrMem::ResetAccessors()
		{
			accessors[0].Set( this, &ChrMem::Access_Pattern );
			accessors[1].Set( this, &ChrMem::Access_Pattern );
		}

		void Ppu::ChrMem::SetDefaultAccessor(uint i)
		{
			NST_ASSERT( i < 2 );
			accessors[i].Set( this, &ChrMem::Access_Pattern );
		}

		void Ppu::NmtMem::ResetAccessors()
		{
			accessors[0][0].Set( this, &NmtMem::Access_Name_2000 );
			accessors[0][1].Set( this, &NmtMem::Access_Name_2000 );
			accessors[1][0].Set( this, &NmtMem::Access_Name_2400 );
			accessors[1][1].Set( this, &NmtMem::Access_Name_2400 );
			accessors[2][0].Set( this, &NmtMem::Access_Name_2800 );
			accessors[2][1].Set( this, &NmtMem::Access_Name_2800 );
			accessors[3][0].Set( this, &NmtMem::Access_Name_2C00 );
			accessors[3][1].Set( this, &NmtMem::Access_Name_2C00 );
		}

		void Ppu::NmtMem::SetDefaultAccessor(uint i,uint j)
		{
			NST_ASSERT( i < 4 && j < 2 );

			accessors[i][j].Set
			(
				this,
				i == 0 ? &NmtMem::Access_Name_2000 :
				i == 1 ? &NmtMem::Access_Name_2400 :
				i == 2 ? &NmtMem::Access_Name_2800 :
                         &NmtMem::Access_Name_2C00
			);
		}

		void Ppu::NmtMem::SetDefaultAccessors(uint i)
		{
			SetDefaultAccessor( i, 0 );
			SetDefaultAccessor( i, 1 );
		}

		void Ppu::SetModel(const PpuModel m,const bool yuvConversion)
		{
			if (model != m)
			{
				model = m;
				regs.frame = 0;
				output.burstPhase = 0;

				if (model == PPU_RP2C07)
					cycles.one = PPU_RP2C07_CC;
				else
					cycles.one = PPU_RP2C02_CC;
			}

			const byte* const map =
			(
				model == PPU_RP2C04_0001 ? yuvMaps[0] :
				model == PPU_RP2C04_0002 ? yuvMaps[1] :
				model == PPU_RP2C04_0003 ? yuvMaps[2] :
				model == PPU_RP2C04_0004 ? yuvMaps[3] :
                                           NULL
			);

			const byte* const tmp[2] =
			{
				yuvConversion ? NULL : map,
				yuvConversion ? map : NULL
			};

			if (yuvMap != tmp[0] || rgbMap != tmp[1])
			{
				yuvMap = tmp[0];
				rgbMap = tmp[1];

				UpdatePalette();
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline Cycle Ppu::GetCycles() const
		{
			return (cycles.vClock + cycles.hClock) * cycles.one;
		}

		inline Cycle Ppu::GetLocalCycles(Cycle clock) const
		{
			return cycles.one == PPU_RP2C02_CC ? clock / PPU_RP2C02_CC : (clock+PPU_RP2C07_CC-1) / PPU_RP2C07_CC;
		}

		void Ppu::BeginFrame(bool frameLock)
		{
			NST_ASSERT
			(
				scanline == SCANLINE_VBLANK &&
				(cycles.hClock == HCLOCK_BOOT || cycles.hClock == HCLOCK_DUMMY) &&
				(cpu.GetModel() == CPU_RP2A07) == (model == PPU_RP2C07)
			);

			oam.limit = oam.buffer + ((oam.spriteLimit || frameLock) ? Oam::STD_LINE_SPRITES*4 : Oam::MAX_LINE_SPRITES*4);
			output.target = output.pixels;

			Cycle frame;

			switch (model)
			{
				case PPU_RP2C02:

					regs.frame ^= Regs::FRAME_ODD;

				default:

					if (cycles.hClock == HCLOCK_DUMMY)
					{
						cycles.vClock = PPU_RP2C02_HVINT / PPU_RP2C02_CC - HCLOCK_DUMMY;
						cycles.count = PPU_RP2C02_HVINT;
						frame = PPU_RP2C02_HVSYNC_0;
					}
					else
					{
						cycles.vClock = PPU_RP2C02_HVSYNCBOOT / PPU_RP2C02_CC - HCLOCK_BOOT;
						cycles.count = PPU_RP2C02_HVSYNCBOOT;
						frame = PPU_RP2C02_HVSYNCBOOT;
					}
					break;

				case PPU_RP2C07:

					if (cycles.hClock == HCLOCK_DUMMY)
					{
						cycles.vClock = PPU_RP2C07_HVINT / PPU_RP2C07_CC - HCLOCK_DUMMY;
						cycles.count = PPU_RP2C07_HVINT;
						frame = PPU_RP2C07_HVSYNC;
					}
					else
					{
						cycles.vClock = PPU_RP2C07_HVSYNCBOOT / PPU_RP2C07_CC - HCLOCK_BOOT;
						cycles.count = PPU_RP2C07_HVSYNCBOOT;
						frame = PPU_RP2C07_HVSYNCBOOT;
					}
					break;
			}

			cpu.SetFrameCycles( frame );
		}

		NES_HOOK(Ppu,Nop)
		{
		}

		NES_HOOK(Ppu,Sync)
		{
			const Cycle elapsed = cpu.GetCycles();

			if (cycles.count < elapsed)
			{
				cycles.count = GetLocalCycles( elapsed ) - cycles.vClock;
				Run();
			}
		}

		void Ppu::EndFrame()
		{
			if (cycles.count != Cpu::CYCLE_MAX)
			{
				cycles.count = Cpu::CYCLE_MAX;
				Run();
			}
		}

		void Ppu::Update(Cycle dataSetup)
		{
			dataSetup += cpu.Update();

			if (cycles.count < dataSetup)
			{
				cycles.count = GetLocalCycles( dataSetup ) - cycles.vClock;
				Run();
			}
		}

		void Ppu::SetMirroring(const byte (&banks)[4])
		{
			Update( cycles.one );

			NST_ASSERT( banks[0] < 4 && banks[1] < 4 && banks[2] < 4 && banks[3] < 4 );
			nmt.SwapBanks<SIZE_1K,0x0000>( banks[0], banks[1], banks[2], banks[3] );
		}

		void Ppu::SetMirroring(NmtMirroring mirroring)
		{
			Update( cycles.one );

			nmt.SwapBanks<SIZE_1K,0x0000>
			(
				uint(mirroring) >> 0 & 0x1U,
				uint(mirroring) >> 1 & 0x1U,
				uint(mirroring) >> 2 & 0x1U,
				uint(mirroring) >> 3 & 0x1U
			);
		}

		NES_ACCESSOR(Ppu::ChrMem,Pattern)
		{
			return Peek( address );
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2000)
		{
			return (*this)[0][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2400)
		{
			return (*this)[1][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2800)
		{
			return (*this)[2][address];
		}

		NES_ACCESSOR(Ppu::NmtMem,Name_2C00)
		{
			return (*this)[3][address];
		}

		NST_FORCE_INLINE uint Ppu::Chr::FetchPattern(uint address) const
		{
			address &= 0x1FFF;
			return accessors[address >> 12].Fetch( address );
		}

		NST_FORCE_INLINE uint Ppu::Nmt::FetchName(uint address) const
		{
			const uint offset = address & 0x03FF;
			return accessors[address >> 10 & 0x3][(offset + 0x40) >> 10].Fetch( offset );
		}

		NST_FORCE_INLINE uint Ppu::Nmt::FetchAttribute(uint address) const
		{
			return accessors[address >> 10 & 0x3][1].Fetch( 0x3C0 | (address >> 4 & 0x038) | (address >> 2 & 0x007) );
		}

		NST_FORCE_INLINE uint Ppu::FetchName() const
		{
			return nmt.FetchName( io.address ) << 4 | scroll.address >> 12 | scroll.pattern;
		}

		NST_FORCE_INLINE uint Ppu::FetchAttribute() const
		{
			return nmt.FetchAttribute( io.address ) >> ((scroll.address & 0x2) | (scroll.address >> 4 & 0x4)) & 0x3;
		}

		uint Ppu::GetPixelCycles() const
		{
			return (scanline+1)-1U < 240 ? scanline * 256 + NST_MIN(cycles.hClock,255) : ~0U;
		}

		inline bool Ppu::IsDead() const
		{
			return scanline >= 240 || !(regs.ctrl1 & Regs::CTRL1_BG_SP_ENABLED);
		}

		inline void Ppu::UpdateScrollAddress(const uint newAddress)
		{
			const uint oldAddress = scroll.address;
			scroll.address = newAddress;

			if (io.a12 && (newAddress & 0x1000) > (oldAddress & 0x1000))
				io.a12.Toggle( cpu.GetCycles() );
		}

		inline uint Ppu::Coloring() const
		{
			return (regs.ctrl1 & Regs::CTRL1_MONOCHROME) ? Palette::MONO : Palette::COLOR;
		}

		inline uint Ppu::Emphasis() const
		{
			return (regs.ctrl1 & Regs::CTRL1_EMPHASIS) << 1;
		}

		NES_POKE_D(Ppu,2000)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				scroll.latch = (scroll.latch & 0x73FF) | (data & 0x03) << 10;
				scroll.increase = (data & Regs::CTRL0_INC32) ? 32 : 1;
				scroll.pattern = (data & Regs::CTRL0_BG_OFFSET) << 8;
				oam.height = (data >> 2 & 8) + 8;

				io.latch = data;
				data = regs.ctrl0 ;
				regs.ctrl0 = io.latch;

				if ((regs.ctrl0 & regs.status & Regs::CTRL0_NMI) > data)
				{
					const Cycle clock = cpu.GetCycles() + cycles.one;

					if (clock < (cycles.one == PPU_RP2C02_CC ? PPU_RP2C02_HVINT : PPU_RP2C07_HVINT))
						cpu.DoNMI( clock );
				}
			}
		}

		NES_POKE_D(Ppu,2001)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				tiles.show[0] = (data & Regs::CTRL1_BG_ENABLED) ? 0xFF : 0x00;
				tiles.show[1] = (data & Regs::CTRL1_BG_ENABLED_NO_CLIP) == Regs::CTRL1_BG_ENABLED_NO_CLIP ? 0xFF : 0x00;

				oam.show[0] = (data & Regs::CTRL1_SP_ENABLED) ? 0xFF : 0x00;
				oam.show[1] = (data & Regs::CTRL1_SP_ENABLED_NO_CLIP) == Regs::CTRL1_SP_ENABLED_NO_CLIP ? 0xFF : 0x00;

				const uint pos = (cycles.hClock - 8) >= (256-16);

				tiles.mask = tiles.show[pos];
				oam.mask = oam.show[pos];

				io.latch = data;
				data = (regs.ctrl1 ^ data) & (Regs::CTRL1_EMPHASIS|Regs::CTRL1_MONOCHROME);
				regs.ctrl1 = io.latch;

				if (data)
				{
					const uint ce[] = { Coloring(), Emphasis() };

					const byte* const NST_RESTRICT map = rgbMap;

					if (!map)
					{
						for (uint i=0; i < Palette::SIZE; ++i)
							output.palette[i] = palette.ram[i] & ce[0] | ce[1];
					}
					else
					{
						for (uint i=0; i < Palette::SIZE; ++i)
							output.palette[i] = map[palette.ram[i] & Palette::COLOR] & ce[0] | ce[1];
					}
				}
			}
		}

		NES_PEEK(Ppu,2002)
		{
			Update( cycles.one );

			uint status = regs.status & 0xFF;

			regs.status &= (Regs::STATUS_VBLANK^0xFFU);
			scroll.toggle = 0;
			io.latch = (io.latch & Regs::STATUS_LATCH) | status;

			return io.latch;
		}

		NES_PEEK_A(Ppu,2002_RC2C05_01_04)
		{
			return NES_DO_PEEK(2002,address) & 0xC0 | 0x1B;
		}

		NES_PEEK_A(Ppu,2002_RC2C05_02)
		{
			return NES_DO_PEEK(2002,address) & 0xC0 | 0x3D;
		}

		NES_PEEK_A(Ppu,2002_RC2C05_03)
		{
			return NES_DO_PEEK(2002,address) & 0xC0 | 0x1C;
		}

		NES_POKE_D(Ppu,2003)
		{
			Update( cycles.one );

			regs.oam = data;
			io.latch = data;
		}

		NES_POKE_D(Ppu,2004)
		{
			Update( cycles.one );

			NST_ASSERT( regs.oam < Oam::SIZE );
			NST_VERIFY( IsDead() );

			if (IsDead())
			{
				if ((regs.oam & 0x03) == 0x02)
					data &= 0xE3;
			}
			else
			{
				data = 0xFF;
			}

			byte* const NST_RESTRICT value = oam.ram + regs.oam;
			regs.oam = (regs.oam + 1) & 0xFF;
			io.latch = data;
			*value = data;
		}

		NES_PEEK(Ppu,2004)
		{
			NST_ASSERT( regs.oam <= 0xFF );

			if (!(regs.ctrl1 & Regs::CTRL1_BG_SP_ENABLED) || cpu.GetCycles() - (cpu.GetFrameCycles() - (341 * 241) * cycles.one) >= (341 * 240) * cycles.one)
			{
				io.latch = oam.ram[regs.oam];
			}
			else
			{
				Update( cycles.one );

				io.latch = oam.latch;
			}

			return io.latch;
		}

		NES_POKE_D(Ppu,2005)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				io.latch = data;

				if (scroll.toggle ^= 1)
				{
					scroll.latch = (scroll.latch & 0x7FE0) | (data >> 3);
					scroll.xFine = data & 0x7;
				}
				else
				{
					scroll.latch = (scroll.latch & 0x0C1F) | ((data << 2 | data << 12) & 0x73E0);
				}
			}
		}

		NES_POKE_D(Ppu,2006)
		{
			Update( cycles.one );

			NST_VERIFY( cpu.GetCycles() >= cycles.reset || !data );

			if (cpu.GetCycles() >= cycles.reset)
			{
				io.latch = data;

				if (scroll.toggle ^= 1)
				{
					scroll.latch = (scroll.latch & 0x00FF) | (data & 0x3F) << 8;
				}
				else
				{
					scroll.latch = (scroll.latch & 0x7F00) | data;

					UpdateScrollAddress( scroll.latch );
				}
			}
		}

		NES_POKE_D(Ppu,2007)
		{
			Update( cycles.one * 4 );

			NST_VERIFY( IsDead() );

			uint address = scroll.address;
			UpdateScrollAddress( (scroll.address + scroll.increase) & 0x7FFF );

			io.latch = data;

			if ((address & 0x3F00) == 0x3F00)
			{
				address &= 0x1F;

				const uint final = (!rgbMap ? data : rgbMap[data & Palette::COLOR]) & Coloring() | Emphasis();

				palette.ram[address] = data;
				output.palette[address] = final;

				if (!(address & 0x3))
				{
					palette.ram[address ^ 0x10] = data;
					output.palette[address ^ 0x10] = final;
				}
			}
			else
			{
				address &= 0x3FFF;

				if (address >= 0x2000)
					nmt.Poke( address & 0xFFF, data );
				else
					chr.Poke( address, data );
			}
		}

		NES_PEEK(Ppu,2007)
		{
			Update( cycles.one );

			NST_VERIFY( IsDead() );

			const uint address = scroll.address & 0x3FFF;

			UpdateScrollAddress( (scroll.address + scroll.increase) & 0x7FFF );

			io.latch = (address & 0x3F00) != 0x3F00 ? io.buffer : palette.ram[address & 0x1F] & Coloring();
			io.buffer = (address >= 0x2000 ? nmt.FetchName( address ) : chr.FetchPattern( address ));

			return io.latch;
		}

		NES_POKE_D(Ppu,2xxx)
		{
			io.latch = data;
		}

		NES_PEEK(Ppu,2xxx)
		{
			return io.latch;
		}

		NES_PEEK(Ppu,3000)
		{
			Update( cycles.one );

			return io.latch;
		}

		NES_POKE_D(Ppu,4014)
		{
			if (cpu.IsOddCycle())
				cpu.StealCycles( cpu.GetClock() );

			Update( cycles.one );
			cpu.StealCycles( cpu.GetClock() );

			NST_ASSERT( regs.oam < 0x100 );

			data <<= 8;

			if
			(
				(regs.oam == 0x00 && data < 0x2000) &&
				(!(regs.ctrl1 & Regs::CTRL1_BG_SP_ENABLED) || cpu.GetCycles() <= (cycles.one == PPU_RP2C02_CC ? PPU_RP2C02_HVINT : PPU_RP2C07_HVINT) - cpu.GetClock() * 512)
			)
			{
				cpu.StealCycles( cpu.GetClock() * 512 );

				const byte* const NST_RESTRICT cpuRam = cpu.GetRam() + (data & (Cpu::RAM_SIZE-1));
				byte* const NST_RESTRICT oamRam = oam.ram;

				for (uint i=0x00; i < 0x100; i += 0x4)
				{
					oamRam[i+0x0] = cpuRam[i+0x0];
					oamRam[i+0x1] = cpuRam[i+0x1];
					oamRam[i+0x2] = cpuRam[i+0x2] & 0xE3U;
					oamRam[i+0x3] = cpuRam[i+0x3];
				}

				io.latch = oamRam[0xFF];
			}
			else do
			{
				io.latch = cpu.Peek( data++ );
				cpu.StealCycles( cpu.GetClock() );

				Update( cycles.one );
				cpu.StealCycles( cpu.GetClock() );

				NST_VERIFY( IsDead() );

				if (IsDead())
				{
					if ((regs.oam & 0x03) == 0x02)
						io.latch &= 0xE3;
				}
				else
				{
					io.latch = 0xFF;
				}

				byte* const NST_RESTRICT out = oam.ram + regs.oam;
				regs.oam = (regs.oam + 1) & 0xFF;
				*out = io.latch;
			}
			while (data & 0xFF);
		}

		NES_PEEK(Ppu,4014)
		{
			return 0x40;
		}

		NST_FORCE_INLINE void Ppu::Scroll::ClockX()
		{
			if ((address & X_TILE) != X_TILE)
				address++;
			else
				address ^= (X_TILE|NAME_LOW);
		}

		NST_SINGLE_CALL void Ppu::Scroll::ResetX()
		{
			address = (address & ((X_TILE|NAME_LOW) ^ 0x7FFFU)) | (latch & (X_TILE|NAME_LOW));
		}

		NST_SINGLE_CALL void Ppu::Scroll::ClockY()
		{
			if ((address & Y_FINE) != (7U << 12))
			{
				address += (1U << 12);
			}
			else switch (address & Y_TILE)
			{
				default:         address = (address & (Y_FINE ^ 0x7FFFU)) + (1U << 5); break;
				case (29U << 5): address ^= NAME_HIGH;
				case (31U << 5): address &= (Y_FINE|Y_TILE) ^ 0x7FFFU; break;
			}
		}

		NST_SINGLE_CALL void Ppu::PreLoadTiles()
		{
			const byte* const NST_RESTRICT src[] =
			{
				tileLut.block[(tiles.pattern[0] & 0xAAU) >> 1 | (tiles.pattern[1] & 0xAAU) << 0 | uint(tiles.attribute) << 8],
				tileLut.block[(tiles.pattern[0] & 0x55U) >> 0 | (tiles.pattern[1] & 0x55U) << 1 | uint(tiles.attribute) << 8]
			};

			NST_ASSERT( tiles.index == 8 );

			byte* const NST_RESTRICT dst = tiles.pixels;

			dst[0] = src[0][3];
			dst[1] = src[1][3];
			dst[2] = src[0][2];
			dst[3] = src[1][2];
			dst[4] = src[0][1];
			dst[5] = src[1][1];
			dst[6] = src[0][0];
			dst[7] = src[1][0];
		}

		NST_SINGLE_CALL void Ppu::LoadTiles()
		{
			const byte* const NST_RESTRICT src[] =
			{
				tileLut.block[(tiles.pattern[0] & 0xAAU) >> 1 | (tiles.pattern[1] & 0xAAU) << 0 | uint(tiles.attribute) << 8],
				tileLut.block[(tiles.pattern[0] & 0x55U) >> 0 | (tiles.pattern[1] & 0x55U) << 1 | uint(tiles.attribute) << 8]
			};

			byte* const NST_RESTRICT dst = tiles.pixels + tiles.index;
			tiles.index ^= 8U;

			dst[0] = src[0][3];
			dst[1] = src[1][3];
			dst[2] = src[0][2];
			dst[3] = src[1][2];
			dst[4] = src[0][1];
			dst[5] = src[1][1];
			dst[6] = src[0][0];
			dst[7] = src[1][0];
		}

		NST_FORCE_INLINE void Ppu::EvaluateSpritesEven()
		{
			oam.latch = (cycles.hClock < 64) ? 0xFF : oam.ram[oam.address];
		}

		NST_FORCE_INLINE void Ppu::EvaluateSpritesOdd()
		{
			(*this.*oam.phase)();
		}

		void Ppu::EvaluateSpritesPhase0()
		{
		}

		void Ppu::EvaluateSpritesPhase1()
		{
			oam.index++;

			if (uint(scanline) - oam.latch >= oam.height)
			{
				if (oam.index != 64)
				{
					oam.address = (oam.index != 2 ? oam.address + 4 : 8);
				}
				else
				{
					oam.address = 0;
					oam.phase = &Ppu::EvaluateSpritesPhase9;
				}
			}
			else
			{
				oam.buffered[0] = oam.latch;
				oam.address++;
				oam.phase = &Ppu::EvaluateSpritesPhase2;
			}
		}

		void Ppu::EvaluateSpritesPhase2()
		{
			oam.buffered[1] = oam.latch;
			oam.address++;
			oam.phase = &Ppu::EvaluateSpritesPhase3;
		}

		void Ppu::EvaluateSpritesPhase3()
		{
			oam.buffered[2] = oam.latch;
			oam.address++;
			oam.phase = &Ppu::EvaluateSpritesPhase4;
		}

		void Ppu::EvaluateSpritesPhase4()
		{
			oam.buffered[3] = oam.latch;
			oam.buffered += 4;

			if (oam.index != 64)
			{
				oam.phase = (oam.buffered != oam.limit ? &Ppu::EvaluateSpritesPhase1 : &Ppu::EvaluateSpritesPhase5);

				if (oam.index != 2)
				{
					oam.address++;

					if (oam.index == 1)
						oam.spriteZeroInLine = true;
				}
				else
				{
					oam.address = 8;
				}
			}
			else
			{
				oam.address = 0;
				oam.phase = &Ppu::EvaluateSpritesPhase9;
			}
		}

		void Ppu::EvaluateSpritesPhase5()
		{
			if (uint(scanline) - oam.latch >= oam.height)
			{
				oam.address = ((oam.address + 4) & 0xFC) + ((oam.address + 1) & 0x03);

				if (oam.address <= 5)
				{
					oam.phase = &Ppu::EvaluateSpritesPhase9;
					oam.address &= 0xFC;
				}
			}
			else
			{
				oam.phase = &Ppu::EvaluateSpritesPhase6;
				oam.address = (oam.address + 1) & 0xFF;
				regs.status |= Regs::STATUS_SP_OVERFLOW;
			}
		}

		void Ppu::EvaluateSpritesPhase6()
		{
			oam.phase = &Ppu::EvaluateSpritesPhase7;
			oam.address = (oam.address + 1) & 0xFF;
		}

		void Ppu::EvaluateSpritesPhase7()
		{
			oam.phase = &Ppu::EvaluateSpritesPhase8;
			oam.address = (oam.address + 1) & 0xFF;
		}

		void Ppu::EvaluateSpritesPhase8()
		{
			oam.phase = &Ppu::EvaluateSpritesPhase9;
			oam.address = (oam.address + 1) & 0xFF;

			if ((oam.address & 0x3) == 0x3)
				oam.address++;

			oam.address &= 0xFC;
		}

		void Ppu::EvaluateSpritesPhase9()
		{
			oam.address = (oam.address + 4) & 0xFF;
		}

		void Ppu::LoadSprite(const byte* const NST_RESTRICT buffer,const uint a12rising)
		{
			const uint comparitor = (uint(scanline) - buffer[0]) ^ ((buffer[2] & uint(Oam::Y_FLIP)) ? 0xF : 0x0);

			uint address;

			if (regs.ctrl0 & Regs::CTRL0_SP8X16)
			{
				address =
				(
					((buffer[1] & uint(Oam::TILE_LSB)) << 12) |
					((buffer[1] & (Oam::TILE_LSB ^ 0xFFU)) << 4) |
					((comparitor & Oam::RANGE_MSB) << 1)
				);
			}
			else
			{
				address = (regs.ctrl0 & Regs::CTRL0_SP_OFFSET) << 9 | buffer[1] << 4;
			}

			address |= comparitor & Oam::XFINE;

			if (io.a12 && address & a12rising)
				io.a12.Toggle( GetCycles() );

			uint pattern[2] =
			{
				chr.FetchPattern( address | 0x0 ),
				chr.FetchPattern( address | 0x8 )
			};

			if (pattern[0] | pattern[1])
			{
				address = (buffer[2] & uint(Oam::X_FLIP)) ? 7 : 0;

				uint p =
				(
					(pattern[0] >> 1 & 0x0055) | (pattern[1] << 0 & 0x00AA) |
					(pattern[0] << 8 & 0x5500) | (pattern[1] << 9 & 0xAA00)
				);

				Oam::Output* const NST_RESTRICT entry = oam.visible++;

				entry->pixels[( address^=6 )] = ( p       ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=6 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=7 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=6 )] = ( p >>= 2 ) & 0x3;
				entry->pixels[( address^=2 )] = ( p >>= 2 );

				const uint attribute = buffer[2];

				entry->x       = buffer[3];
				entry->palette = Palette::SPRITE_OFFSET + ((attribute & Oam::COLOR) << 2);
				entry->zero    = (buffer == oam.buffer && oam.spriteZeroInLine) ? 0x3 : 0x0;
				entry->behind  = (attribute & Oam::BEHIND) ? 0x3 : 0x0;
			}
		}

		NST_FORCE_INLINE void Ppu::RenderPixel()
		{
			uint clock;
			uint pixel = tiles.pixels[((clock=cycles.hClock++) + scroll.xFine) & 15] & tiles.mask;

			for (const Oam::Output* NST_RESTRICT sprite=oam.output, *const end=oam.visible; sprite != end; ++sprite)
			{
				uint x = clock - sprite->x;

				if (x > 7)
					continue;

				x = sprite->pixels[x] & oam.mask;

				if (x)
				{
					// first two bits of sprite->zero and sprite->behind booleans
					// are masked if true (for minimizing branching)

					if (pixel & sprite->zero)
						regs.status |= Regs::STATUS_SP_ZERO_HIT;

					if (!(pixel & sprite->behind))
						pixel = sprite->palette + x;

					break;
				}
			}

			Video::Screen::Pixel* const NST_RESTRICT target = output.target++;
			*target = output.palette[pixel];
		}

		NST_SINGLE_CALL void Ppu::RenderPixel255()
		{
			uint pixel = tiles.pixels[(255 + scroll.xFine) & 15] & tiles.mask;

			for (const Oam::Output* NST_RESTRICT sprite=oam.output, *const end=oam.visible; sprite != end; ++sprite)
			{
				uint x = 255 - sprite->x;

				if (x > 7)
					continue;

				x = sprite->pixels[x] & oam.mask;

				if (x)
				{
					if (!(pixel & sprite->behind))
						pixel = sprite->palette + x;

					break;
				}
			}

			Video::Screen::Pixel* const NST_RESTRICT target = output.target++;
			*target = output.palette[pixel];
		}

		NST_NO_INLINE void Ppu::Run()
		{
			NST_VERIFY( cycles.count != cycles.hClock );

			if (regs.ctrl1 & Regs::CTRL1_BG_SP_ENABLED)
			{
				switch (cycles.hClock)
				{
					case 0:
					case 8:
					case 16:
					case 24:
					case 32:
					case 40:
					case 48:
					case 56:
					case 64:
					case 72:
					case 80:
					case 88:
					case 96:
					case 104:
					case 112:
					case 120:
					case 128:
					case 136:
					case 144:
					case 152:
					case 160:
					case 168:
					case 176:
					case 184:
					case 192:
					case 200:
					case 208:
					case 216:
					case 224:
					case 232:
					case 240:
					case 248:
					HActive:

						if (cycles.hClock == 64)
						{
							NST_VERIFY( regs.oam == 0 );
							oam.address = regs.oam & Oam::OFFSET_TO_0_1;
							oam.phase = &Ppu::EvaluateSpritesPhase1;
						}

					HActiveBegin:

						LoadTiles();
						io.address = scroll.address;
						EvaluateSpritesEven();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 1:
					case 9:
					case 17:
					case 25:
					case 33:
					case 41:
					case 49:
					case 57:
					case 65:
					case 73:
					case 81:
					case 89:
					case 97:
					case 105:
					case 113:
					case 121:
					case 129:
					case 137:
					case 145:
					case 153:
					case 161:
					case 169:
					case 177:
					case 185:
					case 193:
					case 201:
					case 209:
					case 217:
					case 225:
					case 233:
					case 241:
					case 249:

						io.pattern = FetchName();
						EvaluateSpritesOdd();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 2:
					case 10:
					case 18:
					case 26:
					case 34:
					case 42:
					case 50:
					case 58:
					case 66:
					case 74:
					case 82:
					case 90:
					case 98:
					case 106:
					case 114:
					case 122:
					case 130:
					case 138:
					case 146:
					case 154:
					case 162:
					case 170:
					case 178:
					case 186:
					case 194:
					case 202:
					case 210:
					case 218:
					case 226:
					case 234:
					case 242:
					case 250:

						io.address = scroll.address;
						EvaluateSpritesEven();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 3:
					case 11:
					case 19:
					case 27:
					case 35:
					case 43:
					case 51:
					case 59:
					case 67:
					case 75:
					case 83:
					case 91:
					case 99:
					case 107:
					case 115:
					case 123:
					case 131:
					case 139:
					case 147:
					case 155:
					case 163:
					case 171:
					case 179:
					case 187:
					case 195:
					case 203:
					case 211:
					case 219:
					case 227:
					case 235:
					case 243:
					case 251:

						tiles.attribute = FetchAttribute();
						EvaluateSpritesOdd();

						if (cycles.hClock == 251)
							scroll.ClockY();

						scroll.ClockX();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 4:
					case 12:
					case 20:
					case 28:
					case 36:
					case 44:
					case 52:
					case 60:
					case 68:
					case 76:
					case 84:
					case 92:
					case 100:
					case 108:
					case 116:
					case 124:
					case 132:
					case 140:
					case 148:
					case 156:
					case 164:
					case 172:
					case 180:
					case 188:
					case 196:
					case 204:
					case 212:
					case 220:
					case 228:
					case 236:
					case 244:
					case 252:

						io.address = io.pattern | 0x0;

						if (io.a12 && scroll.pattern)
							io.a12.Toggle( GetCycles() );

						EvaluateSpritesEven();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 5:
					case 13:
					case 21:
					case 29:
					case 37:
					case 45:
					case 53:
					case 61:
					case 69:
					case 77:
					case 85:
					case 93:
					case 101:
					case 109:
					case 117:
					case 125:
					case 133:
					case 141:
					case 149:
					case 157:
					case 165:
					case 173:
					case 181:
					case 189:
					case 197:
					case 205:
					case 213:
					case 221:
					case 229:
					case 237:
					case 245:
					case 253:

						tiles.pattern[0] = chr.FetchPattern( io.address );
						EvaluateSpritesOdd();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

					case 6:
					case 14:
					case 22:
					case 30:
					case 38:
					case 46:
					case 54:
					case 62:
					case 70:
					case 78:
					case 86:
					case 94:
					case 102:
					case 110:
					case 118:
					case 126:
					case 134:
					case 142:
					case 150:
					case 158:
					case 166:
					case 174:
					case 182:
					case 190:
					case 198:
					case 206:
					case 214:
					case 222:
					case 230:
					case 238:
					case 246:
					case 254:

						io.address = io.pattern | 0x8;
						EvaluateSpritesEven();
						RenderPixel();

						if (cycles.count <= cycles.hClock)
							break;

						if (cycles.hClock == 255)
							goto HActiveEnd;

					case 7:
					case 15:
					case 23:
					case 31:
					case 39:
					case 47:
					case 55:
					case 63:
					case 71:
					case 79:
					case 87:
					case 95:
					case 103:
					case 111:
					case 119:
					case 127:
					case 135:
					case 143:
					case 151:
					case 159:
					case 167:
					case 175:
					case 183:
					case 191:
					case 199:
					case 207:
					case 215:
					case 223:
					case 231:
					case 239:
					case 247:

						tiles.pattern[1] = chr.FetchPattern( io.address );
						EvaluateSpritesOdd();
						RenderPixel();

						tiles.mask = tiles.show[0];
						oam.mask = oam.show[0];

						if (cycles.count <= cycles.hClock)
							break;

						goto HActive;

					case 255:
					HActiveEnd:

						tiles.pattern[1] = chr.FetchPattern( io.address );
						EvaluateSpritesOdd();
						cycles.hClock = 256;
						RenderPixel255();

						if (cycles.count <= 256)
							break;

					case 256:

						oam.latch = 0xFF;
						cycles.hClock = 257;

						if (cycles.count <= 257)
							break;

					case 257:

						hBlankHook.Execute();

						scroll.ResetX();

						oam.visible = oam.output;

						cycles.hClock = 260;

						if (cycles.count <= 260)
							break;

					case 260:
					case 268:
					case 276:
					case 284:
					case 292:
					case 300:
					case 308:
					case 316:

						for (const byte* buffer = oam.buffer + ((cycles.hClock - 260) >> 1); ; buffer += 4)
						{
							if (buffer < oam.buffered)
							{
								LoadSprite( buffer );

								if (cycles.hClock != 316)
								{
									cycles.hClock += 8;

									if (cycles.count <= cycles.hClock)
										goto End;
								}
								else
								{
									// extended +9 sprites

									for (buffer += 4; buffer < oam.buffered; buffer += 4)
										LoadSprite( buffer, 0 );

									break;
								}
							}
							else
							{
								if (io.a12 && regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16))
								{
									for (;;)
									{
										io.a12.Toggle( GetCycles() );

										if (cycles.hClock == 316)
											break;

										cycles.hClock += 8;

										if (cycles.count <= cycles.hClock)
											goto End;
									}
								}

								break;
							}
						}

						if (scanline == 238)
							regs.oam = 0;

						cycles.hClock = 320;

						if (cycles.count <= 320)
							break;

					case 320:
					HBlank:

						hActiveHook.Execute();

						oam.latch = oam.ram[0];
						oam.buffered = oam.buffer;
						oam.spriteZeroInLine = false;
						oam.index = 0;
						oam.phase = &Ppu::EvaluateSpritesPhase0;
						io.address = scroll.address;
						cycles.hClock = 321;

						if (cycles.count <= 321)
							break;

					case 321:

						io.pattern = FetchName();
						cycles.hClock = 322;

						if (cycles.count <= 322)
							break;

					case 322:

						io.address = scroll.address;
						cycles.hClock = 323;

						if (cycles.count <= 323)
							break;

					case 323:

						tiles.attribute = FetchAttribute();
						scroll.ClockX();
						cycles.hClock = 324;

						if (cycles.count <= 324)
							break;

					case 324:

						io.address = io.pattern | 0x0;

						if (io.a12 && scroll.pattern)
							io.a12.Toggle( GetCycles() );

						cycles.hClock = 325;

						if (cycles.count <= 325)
							break;

					case 325:

						tiles.pattern[0] = chr.FetchPattern( io.address );
						cycles.hClock = 326;

						if (cycles.count <= 326)
							break;

					case 326:

						io.address = io.pattern | 0x8;
						cycles.hClock = 327;

						if (cycles.count <= 327)
							break;

					case 327:

						tiles.pattern[1] = chr.FetchPattern( io.address );
						cycles.hClock = 328;

						if (cycles.count <= 328)
							break;

					case 328:

						PreLoadTiles();
						io.address = scroll.address;
						cycles.hClock = 329;

						if (cycles.count <= 329)
							break;

					case 329:

						io.pattern = FetchName();
						cycles.hClock = 330;

						if (cycles.count <= 330)
							break;

					case 330:

						io.address = scroll.address;
						cycles.hClock = 331;

						if (cycles.count <= 331)
							break;

					case 331:

						tiles.attribute = FetchAttribute();
						scroll.ClockX();
						cycles.hClock = 332;

						if (cycles.count <= 332)
							break;

					case 332:

						io.address = io.pattern | 0x0;

						if (io.a12 && scroll.pattern)
							io.a12.Toggle( GetCycles() );

						cycles.hClock = 333;

						if (cycles.count <= 333)
							break;

					case 333:

						tiles.pattern[0] = chr.FetchPattern( io.address );
						cycles.hClock = 334;

						if (cycles.count <= 334)
							break;

					case 334:

						io.address = io.pattern | 0x8;
						cycles.hClock = 335;

						if (cycles.count <= 335)
							break;

					case 335:

						tiles.pattern[1] = chr.FetchPattern( io.address );
						cycles.hClock = 337;

						if (cycles.count <= 337)
							break;

					case 337:

						if (scanline++ != 239)
						{
							tiles.mask = tiles.show[1];
							oam.mask = oam.show[1];

							cycles.hClock = 0;

							if (scanline == 0 && model == PPU_RP2C02)
							{
								if (regs.frame)
								{
									output.burstPhase = (output.burstPhase + 2) % 3;
									cpu.SetFrameCycles( PPU_RP2C02_HVSYNC_1 );

									cycles.vClock += 340;

									if (cycles.count <= 340)
										break;

									cycles.count -= 340;

									goto HActiveBegin;
								}
								else
								{
									output.burstPhase = (output.burstPhase + 1) % 3;
								}
							}

							cycles.vClock += 341;

							if (cycles.count <= 341)
								break;

							cycles.count -= 341;

							goto HActiveBegin;
						}
						else
						{
							cycles.hClock = HCLOCK_VBLANK_0;

							if (cycles.count <= HCLOCK_VBLANK_0)
								break;
						}

					case HCLOCK_VBLANK_0:
					VBlank0:

						regs.status |= Regs::STATUS_VBLANKING;
						cycles.hClock = HCLOCK_VBLANK_1;

						if (cycles.count <= HCLOCK_VBLANK_1)
							break;

					case HCLOCK_VBLANK_1:
					VBlank1:

						regs.status = (regs.status & 0xFF) | (regs.status >> 1 & Regs::STATUS_VBLANK);
						scanline = SCANLINE_VBLANK;
						oam.visible = oam.output;
						cycles.hClock = HCLOCK_VBLANK_2;

						if (cycles.count <= HCLOCK_VBLANK_2)
							break;

					case HCLOCK_VBLANK_2:
					VBlank2:

						cycles.hClock = HCLOCK_DUMMY;
						cycles.count = Cpu::CYCLE_MAX;
						cycles.reset = 0;

						if (regs.ctrl0 & regs.status & Regs::CTRL0_NMI)
							cpu.DoNMI( cpu.GetFrameCycles() );

						return;

					case HCLOCK_BOOT:
						goto Boot;

					case HCLOCK_DUMMY+0:

						regs.status = 0;
						scanline = SCANLINE_HDUMMY;
						cycles.hClock = HCLOCK_DUMMY+4;

						if (cycles.count <= HCLOCK_DUMMY+4)
							break;

					case HCLOCK_DUMMY+4:
					case HCLOCK_DUMMY+12:
					case HCLOCK_DUMMY+20:
					case HCLOCK_DUMMY+28:
					case HCLOCK_DUMMY+36:
					case HCLOCK_DUMMY+44:
					case HCLOCK_DUMMY+52:
					case HCLOCK_DUMMY+60:
					case HCLOCK_DUMMY+68:
					case HCLOCK_DUMMY+76:
					case HCLOCK_DUMMY+84:
					case HCLOCK_DUMMY+92:
					case HCLOCK_DUMMY+100:
					case HCLOCK_DUMMY+108:
					case HCLOCK_DUMMY+116:
					case HCLOCK_DUMMY+124:
					case HCLOCK_DUMMY+132:
					case HCLOCK_DUMMY+140:
					case HCLOCK_DUMMY+148:
					case HCLOCK_DUMMY+156:
					case HCLOCK_DUMMY+164:
					case HCLOCK_DUMMY+172:
					case HCLOCK_DUMMY+180:
					case HCLOCK_DUMMY+188:
					case HCLOCK_DUMMY+196:
					case HCLOCK_DUMMY+204:
					case HCLOCK_DUMMY+212:
					case HCLOCK_DUMMY+220:
					case HCLOCK_DUMMY+228:
					case HCLOCK_DUMMY+236:
					case HCLOCK_DUMMY+244:
					case HCLOCK_DUMMY+252:

						if (io.a12 && scroll.pattern)
						{
							do
							{
								io.a12.Toggle( GetCycles() );
								cycles.hClock += 8;

								if (cycles.count <= cycles.hClock)
									goto End;
							}
							while (cycles.hClock != HCLOCK_DUMMY+252);
						}
						else
						{
							if (cycles.count <= HCLOCK_DUMMY+260)
							{
								cycles.hClock = HCLOCK_DUMMY+4 + ((cycles.count - (HCLOCK_DUMMY+4-7)) & ~7U);
								break;
							}

							cycles.hClock = HCLOCK_DUMMY+260;
						}

					case HCLOCK_DUMMY+260:
					case HCLOCK_DUMMY+268:
					case HCLOCK_DUMMY+276:
					case HCLOCK_DUMMY+284:
					case HCLOCK_DUMMY+292:

						if (io.a12 && regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16))
						{
							do
							{
								io.a12.Toggle( GetCycles() );
								cycles.hClock += 8;

								if (cycles.count <= cycles.hClock)
									goto End;
							}
							while (cycles.hClock != HCLOCK_DUMMY+300);
						}
						else
						{
							if (cycles.count <= HCLOCK_DUMMY+300)
							{
								cycles.hClock = HCLOCK_DUMMY+260 + ((cycles.count - (HCLOCK_DUMMY+260-7)) & ~7U);
								break;
							}

							cycles.hClock = HCLOCK_DUMMY+300;
						}

					case HCLOCK_DUMMY+300:

						if (io.a12 && regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16))
							io.a12.Toggle( GetCycles() );

						cycles.hClock = HCLOCK_DUMMY+304;

						if (cycles.count <= HCLOCK_DUMMY+304)
							break;

					case HCLOCK_DUMMY+304:

						scroll.address = scroll.latch;
						cycles.hClock = HCLOCK_DUMMY+308;

						if (cycles.count <= HCLOCK_DUMMY+308)
							break;

					case HCLOCK_DUMMY+308:

						if (io.a12 && regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16))
							io.a12.Toggle( GetCycles() );

						cycles.hClock = HCLOCK_DUMMY+316;

						if (cycles.count <= HCLOCK_DUMMY+316)
							break;

					case HCLOCK_DUMMY+316:

						if (io.a12 && regs.ctrl0 & (Regs::CTRL0_SP_OFFSET|Regs::CTRL0_SP8X16))
							io.a12.Toggle( GetCycles() );

						cycles.hClock = 320;
						cycles.vClock += HCLOCK_DUMMY;
						cycles.count -= HCLOCK_DUMMY;

						if (cycles.count <= 320)
							break;

						goto HBlank;

					default:

						NST_UNREACHABLE();
				}
			}
			else
			{
				switch (cycles.hClock)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
					case 16:
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
					case 25:
					case 26:
					case 27:
					case 28:
					case 29:
					case 30:
					case 31:
					case 32:
					case 33:
					case 34:
					case 35:
					case 36:
					case 37:
					case 38:
					case 39:
					case 40:
					case 41:
					case 42:
					case 43:
					case 44:
					case 45:
					case 46:
					case 47:
					case 48:
					case 49:
					case 50:
					case 51:
					case 52:
					case 53:
					case 54:
					case 55:
					case 56:
					case 57:
					case 58:
					case 59:
					case 60:
					case 61:
					case 62:
					case 63:
					case 64:
					case 65:
					case 66:
					case 67:
					case 68:
					case 69:
					case 70:
					case 71:
					case 72:
					case 73:
					case 74:
					case 75:
					case 76:
					case 77:
					case 78:
					case 79:
					case 80:
					case 81:
					case 82:
					case 83:
					case 84:
					case 85:
					case 86:
					case 87:
					case 88:
					case 89:
					case 90:
					case 91:
					case 92:
					case 93:
					case 94:
					case 95:
					case 96:
					case 97:
					case 98:
					case 99:
					case 100:
					case 101:
					case 102:
					case 103:
					case 104:
					case 105:
					case 106:
					case 107:
					case 108:
					case 109:
					case 110:
					case 111:
					case 112:
					case 113:
					case 114:
					case 115:
					case 116:
					case 117:
					case 118:
					case 119:
					case 120:
					case 121:
					case 122:
					case 123:
					case 124:
					case 125:
					case 126:
					case 127:
					case 128:
					case 129:
					case 130:
					case 131:
					case 132:
					case 133:
					case 134:
					case 135:
					case 136:
					case 137:
					case 138:
					case 139:
					case 140:
					case 141:
					case 142:
					case 143:
					case 144:
					case 145:
					case 146:
					case 147:
					case 148:
					case 149:
					case 150:
					case 151:
					case 152:
					case 153:
					case 154:
					case 155:
					case 156:
					case 157:
					case 158:
					case 159:
					case 160:
					case 161:
					case 162:
					case 163:
					case 164:
					case 165:
					case 166:
					case 167:
					case 168:
					case 169:
					case 170:
					case 171:
					case 172:
					case 173:
					case 174:
					case 175:
					case 176:
					case 177:
					case 178:
					case 179:
					case 180:
					case 181:
					case 182:
					case 183:
					case 184:
					case 185:
					case 186:
					case 187:
					case 188:
					case 189:
					case 190:
					case 191:
					case 192:
					case 193:
					case 194:
					case 195:
					case 196:
					case 197:
					case 198:
					case 199:
					case 200:
					case 201:
					case 202:
					case 203:
					case 204:
					case 205:
					case 206:
					case 207:
					case 208:
					case 209:
					case 210:
					case 211:
					case 212:
					case 213:
					case 214:
					case 215:
					case 216:
					case 217:
					case 218:
					case 219:
					case 220:
					case 221:
					case 222:
					case 223:
					case 224:
					case 225:
					case 226:
					case 227:
					case 228:
					case 229:
					case 230:
					case 231:
					case 232:
					case 233:
					case 234:
					case 235:
					case 236:
					case 237:
					case 238:
					case 239:
					case 240:
					case 241:
					case 242:
					case 243:
					case 244:
					case 245:
					case 246:
					case 247:
					case 248:
					case 249:
					case 250:
					case 251:
					case 252:
					case 253:
					case 254:
					case 255:
					HActiveOff:
					{
						const uint pixel = output.palette[(scroll.address & 0x3F00) == 0x3F00 ? (scroll.address & 0x001F) : 0];

						uint i = cycles.hClock;
						const uint hClock = NST_MIN(cycles.count,256);
						NST_ASSERT( i < hClock );

						cycles.hClock = hClock;
						tiles.index = (hClock - 1) & 8;

						byte* const NST_RESTRICT tile = tiles.pixels;
						Video::Screen::Pixel* NST_RESTRICT target = output.target;

						do
						{
							tile[i++ & 15] = 0;
							*target++ = pixel;
						}
						while (i != hClock);

						output.target = target;

						if (cycles.count <= 256)
							break;
					}

					case 256:

						cycles.hClock = 257;

						if (cycles.count <= 257)
							break;

					case 257:

						hBlankHook.Execute();

						oam.visible = oam.output;

						cycles.hClock = 260;

						if (cycles.count <= 260)
							break;

					case 260:
					case 268:
					case 276:
					case 284:
					case 292:
					case 300:
					case 308:

						if (cycles.count <= 316)
						{
							cycles.hClock = 260 + ((cycles.count - (260-7)) & ~7U);
							break;
						}

					case 316:

						cycles.hClock = 320;

						if (cycles.count <= 320)
							break;

					case 320:
					HBlankOff:

						hActiveHook.Execute();

						oam.buffered = oam.buffer;
						oam.spriteZeroInLine = false;
						oam.index = 0;
						oam.phase = &Ppu::EvaluateSpritesPhase0;

						cycles.hClock = 321;

						if (cycles.count <= 321)
							break;

					case 321:
					case 322:
					case 323:
					case 324:
					case 325:
					case 326:
					case 327:
					case 328:
					case 329:
					case 330:
					case 331:
					case 332:
					case 333:
					case 334:

						if (cycles.count <= 335)
						{
							cycles.hClock = cycles.count;
							break;
						}

					case 335:

						if (cycles.count <= 337)
						{
							cycles.hClock = 337;
							break;
						}

					case 337:

						if (scanline++ != 239)
						{
							tiles.mask = tiles.show[1];
							oam.mask = oam.show[1];

							if (scanline == 0 && model == PPU_RP2C02)
								output.burstPhase = (output.burstPhase + 1) % 3;

							cycles.vClock += 341;
							cycles.hClock = 0;

							if (cycles.count <= 341)
								break;

							cycles.count -= 341;

							goto HActiveOff;
						}
						else
						{
							cycles.hClock = HCLOCK_VBLANK_0;

							if (cycles.count <= HCLOCK_VBLANK_0)
								break;
						}

					case HCLOCK_VBLANK_0:
						goto VBlank0;

					case HCLOCK_VBLANK_1:
						goto VBlank1;

					case HCLOCK_VBLANK_2:
						goto VBlank2;

					case HCLOCK_BOOT:
					Boot:

						regs.status |= Regs::STATUS_VBLANK;
						cycles.hClock = HCLOCK_DUMMY;
						cycles.count = Cpu::CYCLE_MAX;

						if (cycles.reset)
						{
							if (cycles.one == PPU_RP2C02_CC)
								cycles.reset = PPU_RP2C02_HVREGBOOT - PPU_RP2C02_HVSYNCBOOT;
							else
								cycles.reset = PPU_RP2C07_HVREGBOOT - PPU_RP2C07_HVSYNCBOOT;
						}
						return;

					case HCLOCK_DUMMY+0:

						regs.status = 0;
						scanline = SCANLINE_HDUMMY;
						cycles.hClock = HCLOCK_DUMMY+4;

						if (cycles.count <= HCLOCK_DUMMY+4)
							break;

					case HCLOCK_DUMMY+4:
					case HCLOCK_DUMMY+12:
					case HCLOCK_DUMMY+20:
					case HCLOCK_DUMMY+28:
					case HCLOCK_DUMMY+36:
					case HCLOCK_DUMMY+44:
					case HCLOCK_DUMMY+52:
					case HCLOCK_DUMMY+60:
					case HCLOCK_DUMMY+68:
					case HCLOCK_DUMMY+76:
					case HCLOCK_DUMMY+84:
					case HCLOCK_DUMMY+92:
					case HCLOCK_DUMMY+100:
					case HCLOCK_DUMMY+108:
					case HCLOCK_DUMMY+116:
					case HCLOCK_DUMMY+124:
					case HCLOCK_DUMMY+132:
					case HCLOCK_DUMMY+140:
					case HCLOCK_DUMMY+148:
					case HCLOCK_DUMMY+156:
					case HCLOCK_DUMMY+164:
					case HCLOCK_DUMMY+172:
					case HCLOCK_DUMMY+180:
					case HCLOCK_DUMMY+188:
					case HCLOCK_DUMMY+196:
					case HCLOCK_DUMMY+204:
					case HCLOCK_DUMMY+212:
					case HCLOCK_DUMMY+220:
					case HCLOCK_DUMMY+228:
					case HCLOCK_DUMMY+236:
					case HCLOCK_DUMMY+244:
					case HCLOCK_DUMMY+252:
					case HCLOCK_DUMMY+260:
					case HCLOCK_DUMMY+268:
					case HCLOCK_DUMMY+276:
					case HCLOCK_DUMMY+284:
					case HCLOCK_DUMMY+292:

						if (cycles.count <= HCLOCK_DUMMY+300)
						{
							cycles.hClock = HCLOCK_DUMMY+4 + ((cycles.count - (HCLOCK_DUMMY+4-7)) & ~7U);
							break;
						}

					case HCLOCK_DUMMY+300:

						cycles.hClock = HCLOCK_DUMMY+304;

						if (cycles.count <= HCLOCK_DUMMY+304)
							break;

					case HCLOCK_DUMMY+304:

						cycles.hClock = HCLOCK_DUMMY+308;

						if (cycles.count <= HCLOCK_DUMMY+308)
							break;

					case HCLOCK_DUMMY+308:

						cycles.hClock = HCLOCK_DUMMY+316;

						if (cycles.count <= HCLOCK_DUMMY+316)
							break;

					case HCLOCK_DUMMY+316:

						cycles.hClock = 320;
						cycles.vClock += HCLOCK_DUMMY;
						cycles.count -= HCLOCK_DUMMY;

						if (cycles.count <= 320)
							break;

						goto HBlankOff;

					default:

						NST_UNREACHABLE();
				}
			}
			End:

			cycles.count = GetCycles();
		}
	}
}
