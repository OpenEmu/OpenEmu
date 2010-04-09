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

#ifndef NST_BOARD_MMC5_H
#define NST_BOARD_MMC5_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Mmc5 : public Board
			{
			protected:

				explicit Mmc5(const Context&);

			public:

				class Sound : public Apu::Channel
				{
				public:

					explicit Sound(Apu&,bool=true);

					void WriteCtrl(uint);
					uint ReadCtrl() const;

					void WriteSquareReg0(uint,uint);
					void WriteSquareReg1(uint,uint);
					void WriteSquareReg2(uint,uint);

					void WritePcmReg0(uint);
					void WritePcmReg1(uint);

					void SaveState(State::Saver&,dword) const;
					void LoadState(State::Loader&);

				protected:

					void Reset();
					bool UpdateSettings();
					Cycle Clock(Cycle,Cycle,Cycle);
					Sample GetSample();

				private:

					enum
					{
						NUM_SQUARES = 2,
						RP2A03_M2_QUARTER = CLK_NTSC / (240UL * CLK_NTSC_DIV * CPU_RP2A03_CC) * CPU_RP2A03_CC,
						RP2A07_M2_QUARTER = CLK_PAL  / (200UL * CLK_PAL_DIV  * CPU_RP2A07_CC) * CPU_RP2A07_CC
					};

					class Square
					{
					public:

						void Reset();

						NST_SINGLE_CALL dword GetSample(Cycle);

						NST_SINGLE_CALL void WriteReg0(uint);
						NST_SINGLE_CALL void WriteReg1(uint,uint);
						NST_SINGLE_CALL void WriteReg2(uint,uint);

						NST_SINGLE_CALL void Disable(uint);
						NST_SINGLE_CALL void ClockQuarter();
						NST_SINGLE_CALL void ClockHalf();

						inline uint GetLengthCounter() const;

						void UpdateSettings(uint);

						void SaveState(State::Saver&,dword) const;
						void LoadState(State::Loader&,dword);

					private:

						inline bool CanOutput() const;

						enum
						{
							MIN_FRQ              = 0x4,
							REG1_WAVELENGTH_LOW  = 0xFF,
							REG2_WAVELENGTH_HIGH = 0x07,
							DUTY_SHIFT           = 6
						};

						uint   waveLength;
						ibool  active;
						Cycle  frequency;
						idword timer;
						uint   step;
						uint   duty;

						LengthCounter lengthCounter;
						Envelope envelope;
					};

					class Pcm
					{
					public:

						void Reset();

						inline Sample GetSample() const;

						NST_SINGLE_CALL void WriteReg0(uint);
						NST_SINGLE_CALL void WriteReg1(uint);

						void SaveState(State::Saver&,dword) const;
						void LoadState(State::Loader&);

					private:

						enum
						{
							VOLUME = OUTPUT_MUL / 4,
							PCM_DISABLE = 0x1
						};

						ibool enabled;
						Sample sample;
						Sample amp;
					};

					uint output;
					Cycle rate;
					uint fixed;
					uint atHalfClock;
					dword quarterClock;
					Square square[NUM_SQUARES];
					Pcm pcm;
					DcBlocker dcBlocker;
				};

				// Needs to be public because of bug in GCC 3.x.x
				enum FetchType
				{
					NT_CIRAM_0,
					NT_CIRAM_1,
					NT_EXRAM,
					NT_FILL,
					NT_ZERO,
					AT_FILL,
					AT_EXRAM
				};

			private:

				void SubReset(bool);
				void VBlank();
				void HDummy();
				void HActive0();
				void HActiveX();
				void Sync(Event,Input::Controllers*);

				void SubSave(State::Saver&) const;
				void SubLoad(State::Loader&,dword);

				template<uint ADDRESS>
				void SwapPrg8Ex(uint);

				void UpdatePrg();
				void UpdateChrA() const;
				void UpdateChrB() const;
				void UpdateRenderMethod();

				bool ClockSpliter();

				inline void Update();

				uint GetExtPattern(uint) const;
				uint GetSpliterAttribute() const;
				uint GetSpliterPattern(uint) const;

				template<FetchType>
				inline uint FetchByte(uint) const;

				template<FetchType NT>
				inline uint FetchNt(uint) const;

				template<FetchType NT>
				inline uint FetchNtExt(uint);

				template<FetchType NT>
				inline uint FetchNtSplit(uint);

				template<FetchType NT>
				inline uint FetchNtExtSplit(uint);

				template<FetchType AT>
				inline uint FetchAtSplit(uint) const;

				NES_DECL_HOOK( Cpu     );
				NES_DECL_HOOK( HActive );
				NES_DECL_HOOK( HBlank  );

				NES_DECL_ACCESSOR( Nt_CiRam_0         );
				NES_DECL_ACCESSOR( Nt_CiRam_1         );
				NES_DECL_ACCESSOR( Nt_ExRam           );
				NES_DECL_ACCESSOR( Nt_Fill            );
				NES_DECL_ACCESSOR( Nt_Zero            );
				NES_DECL_ACCESSOR( At_Fill            );
				NES_DECL_ACCESSOR( At_ExRam           );
				NES_DECL_ACCESSOR( NtExt_CiRam_0      );
				NES_DECL_ACCESSOR( NtExt_CiRam_1      );
				NES_DECL_ACCESSOR( NtExt_ExRam        );
				NES_DECL_ACCESSOR( NtExt_Fill         );
				NES_DECL_ACCESSOR( NtSplit_CiRam_0    );
				NES_DECL_ACCESSOR( NtSplit_CiRam_1    );
				NES_DECL_ACCESSOR( NtSplit_ExRam      );
				NES_DECL_ACCESSOR( NtSplit_Fill       );
				NES_DECL_ACCESSOR( AtSplit_CiRam_0    );
				NES_DECL_ACCESSOR( AtSplit_CiRam_1    );
				NES_DECL_ACCESSOR( AtSplit_ExRam      );
				NES_DECL_ACCESSOR( AtSplit_Fill       );
				NES_DECL_ACCESSOR( NtExtSplit_CiRam_0 );
				NES_DECL_ACCESSOR( NtExtSplit_CiRam_1 );
				NES_DECL_ACCESSOR( NtExtSplit_ExRam   );
				NES_DECL_ACCESSOR( NtExtSplit_Fill    );

				NES_DECL_ACCESSOR( CRom         );
				NES_DECL_ACCESSOR( CRomExt      );
				NES_DECL_ACCESSOR( CRomSplit    );
				NES_DECL_ACCESSOR( CRomExtSplit );

				NES_DECL_POKE( 2001 );
				NES_DECL_PEEK( 2001 );

				NES_DECL_POKE( 5000 );
				NES_DECL_POKE( 5002 );
				NES_DECL_POKE( 5003 );
				NES_DECL_POKE( 5004 );
				NES_DECL_POKE( 5006 );
				NES_DECL_POKE( 5007 );
				NES_DECL_POKE( 5010 );
				NES_DECL_POKE( 5011 );
				NES_DECL_PEEK( 5015 );
				NES_DECL_POKE( 5015 );
				NES_DECL_POKE( 5100 );
				NES_DECL_POKE( 5101 );
				NES_DECL_POKE( 5102 );
				NES_DECL_POKE( 5103 );
				NES_DECL_POKE( 5104 );
				NES_DECL_POKE( 5105 );
				NES_DECL_POKE( 5106 );
				NES_DECL_POKE( 5107 );
				NES_DECL_POKE( 5113 );
				NES_DECL_POKE( 5114 );
				NES_DECL_POKE( 5120 );
				NES_DECL_POKE( 5128 );
				NES_DECL_POKE( 5130 );
				NES_DECL_POKE( 5200 );
				NES_DECL_POKE( 5201 );
				NES_DECL_POKE( 5202 );
				NES_DECL_POKE( 5203 );
				NES_DECL_PEEK( 5204 );
				NES_DECL_POKE( 5204 );
				NES_DECL_PEEK( 5205 );
				NES_DECL_POKE( 5205 );
				NES_DECL_PEEK( 5206 );
				NES_DECL_POKE( 5206 );
				NES_DECL_PEEK( 5C00 );
				NES_DECL_POKE( 5C00 );
				NES_DECL_PEEK( 6000 );
				NES_DECL_POKE( 6000 );
				NES_DECL_PEEK( 8000 );
				NES_DECL_POKE( 8000 );
				NES_DECL_PEEK( A000 );
				NES_DECL_POKE( A000 );
				NES_DECL_PEEK( C000 );
				NES_DECL_POKE( C000 );

				struct Flow
				{
					void Reset();

					typedef void (Mmc5::*Phase)();

					Cycle cycles;
					Phase phase;
					uint scanline;
				};

				struct Irq
				{
					void Reset();

					enum
					{
						ENABLED    = 0x01,
						FRAME      = 0x40,
						HIT        = 0x80,
						SIGNAL_HIT = ENABLED|HIT
					};

					uint state;
					uint count;
					uint target;
				};

				struct Regs
				{
					void Reset();

					enum
					{
						PRG_MODE            = 0x03,
						PRG_MODE_32K        = 0x00,
						PRG_MODE_16K        = 0x01,
						PRG_MODE_16K_8K     = 0x02,
						PRG_MODE_8K         = 0x03,
						PRG_ROM_SELECT      = 0x80,
						PRG_ROM_BANK        = 0x7F,
						PRG_RAM_BANK        = 0x07,
						CHR_MODE            = 0x03,
						CHR_MODE_8K         = 0x00,
						CHR_MODE_4K         = 0x01,
						CHR_MODE_2K         = 0x02,
						CHR_MODE_1K         = 0x03,
						CHR_HIGH            = 0x03,
						WRK_WRITABLE_A      = 0x02,
						WRK_WRITABLE_B      = 0x01,
						NMT_MODE            = 0x03,
						NMT_CIRAM_0         = 0x00,
						NMT_CIRAM_1         = 0x01,
						NMT_EXRAM           = 0x02,
						NMT_FILL            = 0x03,
						EXRAM_MODE          = 0x03,
						EXRAM_MODE_PPU_NT   = 0x00,
						EXRAM_MODE_PPU_EXT  = 0x01,
						EXRAM_MODE_CPU_RAM  = 0x02,
						EXRAM_MODE_CPU_ROM  = 0x03,
						EXRAM_EXT_CHR_BANK  = 0x3F,
						PPU_CTRL0_SP8X16    = 0x20,
						PPU_CTRL1_ENABLED   = 0x18
					};

					uint prgMode;
					uint chrMode;
					uint exRamMode;
					uint mul[2];
				};

				struct Banks
				{
					explicit Banks(uint);

					void Reset();

					enum
					{
						READABLE_6  = 0x004,
						READABLE_8  = 0x008,
						READABLE_A  = 0x010,
						READABLE_C  = 0x020,
						WRITABLE_6  = 0x040,
						WRITABLE_8  = 0x080,
						WRITABLE_A  = 0x100,
						WRITABLE_C  = 0x200,
						CAN_WRITE_6 = READABLE_6|WRITABLE_6|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B,
						CAN_WRITE_8 = READABLE_8|WRITABLE_8|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B,
						CAN_WRITE_A = READABLE_A|WRITABLE_A|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B,
						CAN_WRITE_C = READABLE_C|WRITABLE_C|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B
					};

					enum FetchMode
					{
						FETCH_MODE_NONE,
						FETCH_MODE_BG,
						FETCH_MODE_SP
					};

					enum LastChr
					{
						LAST_CHR_A = 0,
						LAST_CHR_B = 1
					};

					class Wrk
					{
						byte banks[8];

					public:

						enum
						{
							INVALID = 8
						};

						explicit Wrk(dword);

						inline uint operator [] (uint) const;
					};

					uint nmt;
					word chrA[8];
					word chrB[4];
					dword chrHigh;
					LastChr lastChr;
					FetchMode fetchMode;
					uint security;
					byte prg[4];
					const Wrk wrk;
				};

				struct Filler
				{
					void Reset();

					uint tile;
					uint attribute;

					static const byte squared[4];
				};

				struct Spliter
				{
					void Reset();

					enum
					{
						CTRL_START      = 0x1F,
						CTRL_RIGHT_SIDE = 0x40,
						CTRL_ENABLED    = 0x80
					};

					uint ctrl;
					uint tile;
					ibool inside;
					uint yStart;
					uint chrBank;
					uint x;
					uint y;
				};

				struct ExRam
				{
					void Reset(bool);

					uint tile;
					byte mem[SIZE_1K];
				};

				Flow flow;
				Irq irq;
				Regs regs;
				Banks banks;
				const byte* ciRam[2];
				Filler filler;
				Spliter spliter;
				Io::Port p2001;
				ExRam exRam;
				Sound sound;
			};
		}
	}
}

#endif
