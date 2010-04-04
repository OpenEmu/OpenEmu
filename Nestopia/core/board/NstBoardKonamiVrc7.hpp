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

#ifndef NST_BOARD_KONAMI_VRC7_H
#define NST_BOARD_KONAMI_VRC7_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Konami
			{
				class Vrc7 : public Board
				{
				public:

					explicit Vrc7(const Context&);

					class Sound : public Apu::Channel
					{
					public:

						explicit Sound(Apu&,bool=true);

						void WriteReg(uint);

						void SaveState(State::Saver&,dword) const;
						void LoadState(State::Loader&);

					protected:

						void Reset();
						bool UpdateSettings();
						Sample GetSample();

					private:

						void ResetClock();
						void Refresh();

						enum
						{
							EG_PHASE_SHIFT = 15,
							EG_MUTE        = 0xFF,
							EG_END         = 0x7F,
							PG_PHASE_SHIFT = 9,
							WAVE_SIZE      = 0x200,
							WAVE_RANGE     = WAVE_SIZE-1,
							PITCH_SHIFT    = 8,
							PITCH_SIZE     = 0x100,
							PITCH_RANGE    = 0xFFFF,
							AMP_SHIFT      = 8,
							AMP_SIZE       = 0x100,
							AMP_RANGE      = 0xFFFF,
							LIN2LOG_SIZE   = 0x80,
							DB2LIN_SIZE    = 0x400,
							TL_SIZE        = 0x40,
							FEEDBACK_SHIFT = 8,
							CLOCK_DIV      = 3579545 / 72,
							CLOCK_RATE     = (1UL << 31) / CLOCK_DIV,
							PG_PHASE_RANGE = (1UL << 18) - 1,
							EG_BEGIN       = 1UL << 22,
							PITCH_RATE     = 64UL * (1UL << 16) / CLOCK_DIV / 10,
							AMP_RATE       = 37UL * (1UL << 16) / CLOCK_DIV / 10
						};

						class Tables
						{
						public:

							Tables();

							inline uint GetAmp(uint) const;
							inline uint GetPitch(uint) const;
							inline uint GetSustainLevel(uint,uint,uint) const;
							inline uint GetTotalLevel(uint,uint,uint,uint) const;
							inline uint GetLog(uint) const;
							inline dword GetAttack(uint,uint) const;
							inline dword GetDecay(uint,uint) const;
							inline dword GetSustain(uint,uint) const;
							inline dword GetRelease(uint,uint) const;
							inline dword GetPhase(uint,uint,uint) const;
							inline Sample GetOutput(uint,uint,uint) const;

						private:

							word  pitch[PITCH_SIZE];
							byte  amp[AMP_SIZE];
							byte  lin2log[LIN2LOG_SIZE];
							dword adr[2][16][16];
							word  wave[2][WAVE_SIZE];
							iword db2lin[DB2LIN_SIZE];
							byte  sl[2][8][2];
							byte  tl[16][8][TL_SIZE][4];
							dword phase[512][8][16];
						};

						enum
						{
							NUM_OPLL_CHANNELS = 6
						};

						class OpllChannel
						{
						public:

							void Reset();
							void Update(const Tables&);
							void SaveState(State::Saver&,dword) const;
							void LoadState(State::Loader&,const Tables&);

							NST_SINGLE_CALL void WriteReg0 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg1 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg2 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg3 (uint);
							NST_SINGLE_CALL void WriteReg4 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg5 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg6 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg7 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg8 (uint,const Tables&);
							NST_SINGLE_CALL void WriteReg9 (uint,const Tables&);
							NST_SINGLE_CALL void WriteRegA (uint,const Tables&);

							NST_SINGLE_CALL Sample GetSample(uint,uint,const Tables&);

						private:

							void UpdatePhase        (const Tables&,uint);
							void UpdateSustainLevel (const Tables&,uint);
							void UpdateTotalLevel   (const Tables&,uint);
							void UpdateEgPhase      (const Tables&,uint);

							enum Mode
							{
								EG_SETTLE,
								EG_ATTACK,
								EG_DECAY,
								EG_HOLD,
								EG_SUSTAIN,
								EG_RELEASE,
								EG_FINISH
							};

							enum
							{
								REG01_MULTIPLE      = 0x0F,
								REG01_RATE          = 0x10,
								REG01_HOLD          = 0x20,
								REG01_USE_VIBRATO   = 0x40,
								REG01_USE_AMP       = 0x80,
								REG2_TOTAL_LEVEL    = 0x3F,
								REG3_FEEDBACK       = 0x07,
								REG3_MODULATED_WAVE = 0x08,
								REG3_CARRIER_WAVE   = 0x10,
								REG45_DECAY         = 0x0F,
								REG45_ATTACK        = 0xF0,
								REG67_RELEASE       = 0x0F,
								REG67_SUSTAIN_LEVEL = 0xF0,
								REG8_FRQ_LO         = 0xFF,
								REG9_FRQ_HI         = 0x01,
								REG9_BLOCK          = 0x0E,
								REG9_KEY            = 0x10,
								REG9_SUSTAIN        = 0x20,
								REGA_VOLUME         = 0x0F,
								REGA_INSTRUMENT     = 0xF0,
								SUSTAIN_LEVEL_MAX   = 0x100
							};

							struct Patch
							{
								enum { CUSTOM };

								uint instrument;
								byte tone[8];
								byte custom[8];

								static const byte preset[15][8];
							};

							enum
							{
								MODULATOR,
								CARRIER,
								NUM_SLOTS
							};

							uint frequency;
							uint key;
							uint sustain;
							uint block;
							uint volume;
							Patch patch;

							struct
							{
								struct
								{
									dword phase;
									dword counter;
								}   pg;

								struct
								{
									Mode mode;
									dword phase;
									dword counter;
								}   eg;

								uint tl;
								uint sl;
								Sample output;
							}   slots[NUM_SLOTS];

							Sample feedback;
						};

						uint output;
						uint regSelect;

						dword sampleRate;
						dword samplePhase;
						dword pitchPhase;
						dword ampPhase;

						Sample prevSample;
						Sample nextSample;

						OpllChannel channels[NUM_OPLL_CHANNELS];
						const Tables tables;

					public:

						void SelectReg(uint data)
						{
							regSelect = data;
						}
					};

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Sync(Event,Input::Controllers*);

					NES_DECL_POKE( 9010 );
					NES_DECL_POKE( 9030 );
					NES_DECL_POKE( E000 );
					NES_DECL_POKE( E008 );
					NES_DECL_POKE( F000 );
					NES_DECL_POKE( F008 );

					Vrc4::Irq irq;
					Sound sound;
				};
			}
		}
	}
}

#endif
