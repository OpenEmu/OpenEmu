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
#include <cmath>
#include "NstBoard.hpp"
#include "../NstClock.hpp"
#include "../NstFpuPrecision.hpp"
#include "NstBoardKonamiVrc4.hpp"
#include "NstBoardKonamiVrc7.hpp"

////////////////////////////////////////////////////////////////////////////////////////
//
// VRC7 Sound Reference:
//
// emu2413.c -- a YM2413 emulator : written by Mitsutaka Okazaki 2001
//
////////////////////////////////////////////////////////////////////////////////////////

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Konami
			{
				const byte Vrc7::Sound::OpllChannel::Patch::preset[15][8] =
				{
					{0x03,0x21,0x04,0x06,0x8D,0xF2,0x42,0x17}, // Violin
					{0x13,0x41,0x05,0x0E,0x99,0x96,0x63,0x12}, // Guitar
					{0x31,0x11,0x10,0x0A,0xF0,0x9C,0x32,0x02}, // Piano
					{0x21,0x61,0x1D,0x07,0x9F,0x64,0x20,0x27}, // Flute
					{0x22,0x21,0x1E,0x06,0xF0,0x76,0x08,0x28}, // Clarinet
					{0x02,0x01,0x06,0x00,0xF0,0xF2,0x03,0x95}, // Oboe
					{0x21,0x61,0x1C,0x07,0x82,0x81,0x16,0x07}, // Trumpet
					{0x23,0x21,0x1A,0x17,0xEF,0x82,0x25,0x15}, // Organ
					{0x25,0x11,0x1F,0x00,0x86,0x41,0x20,0x11}, // Horn
					{0x85,0x01,0x1F,0x0F,0xE4,0xA2,0x11,0x12}, // Synthesizer
					{0x07,0xC1,0x2B,0x45,0xB4,0xF1,0x24,0xF4}, // Harpsichord
					{0x61,0x23,0x11,0x06,0x96,0x96,0x13,0x16}, // Vibraphone
					{0x01,0x02,0xD3,0x05,0x82,0xA2,0x31,0x51}, // Synthesizer Bass
					{0x61,0x22,0x0D,0x02,0xC3,0x7F,0x24,0x05}, // Acoustic Bass
					{0x21,0x62,0x0E,0x00,0xA1,0xA0,0x44,0x17}  // Electric Guitar
				};

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Vrc7::Sound::Tables::Tables()
				{
					FpuPrecision precision;
#pragma unused (precision)

					const double pi2 = 6.2831853071795863;

					for (uint i=0; i < PITCH_SIZE; ++i)
						pitch[i] = AMP_SIZE * std::pow( 2, 13.75 * std::sin( pi2 * i / PITCH_SIZE ) / 1200 );

					for (uint i=0; i < AMP_SIZE; ++i)
						amp[i] = 4.8 / 2 / 0.1875 * (1 + std::sin( pi2 * i / AMP_SIZE ));

					lin2log[0] = 128;

					for (uint i=1; i < LIN2LOG_SIZE; ++i)
						lin2log[i] = 128 - 1 - 128 * std::log( double(i) ) / std::log( 128.0 );

					for (uint i=0; i < 16; ++i)
					{
						for (uint j=0; j < 16; ++j)
						{
							uint rm = i + (j >> 2);
							uint rl = j & 3;

							if (rm > 15)
								rm = 15;

							// Attack

							if (i == 0 || i == 15)
								adr[0][i][j] = 0;
							else
								adr[0][i][j] = (3UL * (rl + 4)) << (rm + 1);

							// Decay & Release

							if (i == 0)
								adr[1][i][j] = 0;
							else
								adr[1][i][j] = (1UL * (rl + 4)) << (rm - 1);
						}
					}

					for (uint i=0; i < WAVE_SIZE/4; ++i)
					{
						uint v = EG_MUTE;
						const double d = std::sin( pi2 * i / WAVE_SIZE );

						if (d > DBL_EPSILON)
						{
							const long m = -(20 * std::log10( d ) / 0.1875);

							if (m < EG_MUTE)
								v = m;
						}

						wave[0][i] = v;
					}

					for (uint i=0; i < WAVE_SIZE/4; ++i)
						wave[0][WAVE_SIZE/2-1 - i] = wave[0][i];

					for (uint i=0; i < WAVE_SIZE/2; ++i)
						wave[0][WAVE_SIZE/2 + i] = (DB2LIN_SIZE/2 + wave[0][i]);

					for (uint i=0; i < WAVE_SIZE/2; ++i)
						wave[1][i] = wave[0][i];

					for (uint i=WAVE_SIZE/2; i < WAVE_SIZE; ++i)
						wave[1][i] = wave[0][0];

					for (uint i=0; i < DB2LIN_SIZE/2; ++i)
					{
						if (i < EG_MUTE)
							db2lin[i] = 0x7FF * std::pow( 10.0, -(i * 0.1875 / 20) );
						else
							db2lin[i] = 0;

						db2lin[DB2LIN_SIZE/2 + i] = -db2lin[i];
					}

					for (uint i=0; i < 2; ++i)
					{
						for (uint j=0; j < 8; ++j)
						{
							for (uint k=0; k < 2; ++k)
							{
								sl[i][j][k] = k ? (j << 1) + i : (j >> 1);
							}
						}
					}

					for (uint i=0; i < 16; ++i)
					{
						for (uint j=0; j < 8; ++j)
						{
							for (uint t=0; t < 64; ++t)
							{
								for (uint k=0; k < 4; ++k)
								{
									uint val = t * 2;

									if (k)
									{
										static const word lut[16] =
										{
											0,     18000, 24000, 27750,
											30000, 32250, 33750, 35250,
											36000, 37500, 38250, 39000,
											39750, 40500, 41250, 42000
										};

										const idword tmp = idword(lut[i]) - 6000 * (7 - idword(j));

										if (tmp > 0)
											val += ((tmp / 1000UL) >> (3-k)) * 1000 / 375;
									}

									tl[i][j][t][k] = val;
								}
							}
						}
					}

					for (uint i=0; i < 512; ++i)
					{
						for (uint j=0; j < 8; ++j)
						{
							for (uint ml=0; ml < 16; ++ml)
							{
								static const byte lut[16] =
								{
                                     1 * 1,  1 * 2,  2 * 2,  3 * 2,
                                     4 * 2,  5 * 2,  6 * 2,  7 * 2,
                                     8 * 2,  9 * 2, 10 * 2, 10 * 2,
									12 * 2, 12 * 2, 15 * 2, 15 * 2
								};

								phase[i][j][ml] = (dword(i * lut[ml]) << j) >> 2;
							}
						}
					}
				}

				Vrc7::Sound::Sound(Apu& a,bool connect)
				: Channel(a)
				{
					Reset();
					bool audible = UpdateSettings();

					if (connect)
						Connect( audible );
				}

				Vrc7::Vrc7(const Context& c)
				:
				Board (c),
				irq   (*c.cpu),
				sound (*c.apu)
				{}

				void Vrc7::SubReset(const bool hard)
				{
					for (dword i=0x8000; i <= 0xFFFF; ++i)
					{
						switch (i & 0xF038)
						{
							case 0x8000: Map( i, PRG_SWAP_8K_0    ); break;
							case 0x8008:
							case 0x8010: Map( i, PRG_SWAP_8K_1    ); break;
							case 0x9000: Map( i, PRG_SWAP_8K_2    ); break;
							case 0x9010:
							case 0x9018: Map( i, &Vrc7::Poke_9010 ); break;
							case 0x9030:
							case 0x9038: Map( i, &Vrc7::Poke_9030 ); break;
							case 0xA000: Map( i, CHR_SWAP_1K_0    ); break;
							case 0xA008:
							case 0xA010: Map( i, CHR_SWAP_1K_1    ); break;
							case 0xB000: Map( i, CHR_SWAP_1K_2    ); break;
							case 0xB008:
							case 0xB010: Map( i, CHR_SWAP_1K_3    ); break;
							case 0xC000: Map( i, CHR_SWAP_1K_4    ); break;
							case 0xC008:
							case 0xC010: Map( i, CHR_SWAP_1K_5    ); break;
							case 0xD000: Map( i, CHR_SWAP_1K_6    ); break;
							case 0xD008:
							case 0xD010: Map( i, CHR_SWAP_1K_7    ); break;
							case 0xE000: Map( i, NMT_SWAP_VH01    ); break;
							case 0xE008:
							case 0xE010: Map( i, &Vrc7::Poke_E008 ); break;
							case 0xF000: Map( i, &Vrc7::Poke_F000 ); break;
							case 0xF008:
							case 0xF010: Map( i, &Vrc7::Poke_F008 ); break;
						}
					}

					irq.Reset( hard, hard ? false : irq.Connected() );

					if (hard)
						prg.SwapBanks<SIZE_8K,0x0000>(0U,0U,0U,~0U);
				}

				void Vrc7::Sound::OpllChannel::Reset()
				{
					frequency = 0;
					key = 0;
					sustain = 0;
					block = 0;
					volume = 0;
					feedback = 0;

					patch.instrument = 0;
					std::memset( patch.tone, 0, sizeof(patch.tone) );
					std::memset( patch.custom, 0, sizeof(patch.tone) );

					for (uint i=0; i < NUM_SLOTS; ++i)
					{
						slots[i].pg.phase = 0;
						slots[i].pg.counter = 0;
						slots[i].eg.mode = EG_SETTLE;
						slots[i].eg.counter = EG_BEGIN;
						slots[i].eg.phase = 0;
						slots[i].tl = 0;
						slots[i].sl = 0;
						slots[i].output = 0;
					}
				}

				void Vrc7::Sound::Reset()
				{
					regSelect = 0;

					for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
						channels[i].Reset();

					ResetClock();
				}

				void Vrc7::Sound::ResetClock()
				{
					sampleRate = 0x80000000UL / GetSampleRate();
					samplePhase = 0;
					prevSample = 0;
					nextSample = 0;
					ampPhase = 0;
					pitchPhase = 0;
				}

				void Vrc7::Sound::Refresh()
				{
					ResetClock();

					for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
						channels[i].Update( tables );
				}

				bool Vrc7::Sound::UpdateSettings()
				{
					output = GetVolume(EXT_VRC7);

					Refresh();

					return output;
				}

				void Vrc7::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'K','V','7'>::V) );

					if (baseChunk == AsciiId<'K','V','7'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'I','R','Q'>::V:

									irq.LoadState( state );
									break;

								case AsciiId<'S','N','D'>::V:

									sound.LoadState( state );
									break;
							}

							state.End();
						}
					}
				}

				void Vrc7::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','V','7'>::V );

					irq.SaveState( state, AsciiId<'I','R','Q'>::V );
					sound.SaveState( state, AsciiId<'S','N','D'>::V );

					state.End();
				}

				void Vrc7::Sound::SaveState(State::Saver& state,const dword baseChunk) const
				{
					state.Begin( baseChunk );

					state.Begin( AsciiId<'R','E','G'>::V ).Write8( regSelect ).End();

					for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
						channels[i].SaveState( state, AsciiId<'C','H','0'>::R(0,0,i) );

					state.End();
				}

				void Vrc7::Sound::LoadState(State::Loader& state)
				{
					Refresh();

					while (dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:

								regSelect = state.Read8();
								break;

							case AsciiId<'C','H','0'>::V:
							case AsciiId<'C','H','1'>::V:
							case AsciiId<'C','H','2'>::V:
							case AsciiId<'C','H','3'>::V:
							case AsciiId<'C','H','4'>::V:
							case AsciiId<'C','H','5'>::V:

								chunk = (chunk >> 16 & 0xFF) - Ascii<'0'>::V;

								if (chunk < NUM_OPLL_CHANNELS)
									channels[chunk].LoadState( state, tables );

								break;
						}

						state.End();
					}
				}

				void Vrc7::Sound::OpllChannel::SaveState(State::Saver& state,const dword chunk) const
				{
					const byte data[11] =
					{
						patch.custom[0],
						patch.custom[1],
						patch.custom[2],
						patch.custom[3],
						patch.custom[4],
						patch.custom[5],
						patch.custom[6],
						patch.custom[7],
						frequency & REG8_FRQ_LO,
						(frequency >> 8) | (block << 1) | (sustain ? REG9_SUSTAIN : 0) | (key ? REG9_KEY : 0),
						(volume >> 2) | (patch.instrument << 4)
					};

					state.Begin( chunk ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void Vrc7::Sound::OpllChannel::LoadState(State::Loader& state,const Tables& tables)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<11> data( state );

							for (uint i=0; i < 8; ++i)
								patch.custom[i] = data[i];

							frequency = (data[8] & REG8_FRQ_LO) | (data[9] & REG9_FRQ_HI) << 8;
							block = (data[9] & REG9_BLOCK) >> 1;
							sustain = data[9] & REG9_SUSTAIN;
							key = data[9] & REG9_KEY;
							volume = (data[10] & REGA_VOLUME) << 2;

							patch.instrument = (data[10] & REGA_INSTRUMENT) >> 4;
							std::memcpy( patch.tone, patch.instrument == Patch::CUSTOM ? patch.custom : Patch::preset[patch.instrument-1], 8 );

							feedback = 0;

							Update( tables );
						}

						state.End();
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Vrc7,9010)
				{
					sound.SelectReg( data );
				}

				NES_POKE_D(Vrc7,9030)
				{
					sound.WriteReg( data );
				}

				NES_POKE_D(Vrc7,E008)
				{
					irq.Update();
					irq.unit.latch = data;
				}

				NES_POKE_D(Vrc7,F000)
				{
					irq.Toggle( data );
				}

				NES_POKE(Vrc7,F008)
				{
					irq.Toggle();
				}

				void Vrc7::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}

				inline uint Vrc7::Sound::Tables::GetAmp(uint i) const
				{
					NST_ASSERT( i < AMP_SIZE );
					return amp[i];
				}

				inline uint Vrc7::Sound::Tables::GetPitch(uint i) const
				{
					NST_ASSERT( i < PITCH_SIZE );
					return pitch[i];
				}

				inline uint Vrc7::Sound::Tables::GetTotalLevel(uint frequency,uint block,uint volume,uint tone) const
				{
					NST_ASSERT( (frequency >> 5) < 16 && block < 8 && volume < TL_SIZE && tone < 4 );
					return tl[frequency >> 5][block][volume][tone];
				}

				inline uint Vrc7::Sound::Tables::GetSustainLevel(uint frequency,uint block,uint rate) const
				{
					NST_ASSERT( (frequency >> 8) < 2 && block < 8 && rate < 2 );
					return sl[frequency >> 8][block][rate];
				}

				inline uint Vrc7::Sound::Tables::GetLog(uint egOut) const
				{
					NST_ASSERT( egOut < LIN2LOG_SIZE );
					return lin2log[egOut];
				}

				inline dword Vrc7::Sound::Tables::GetAttack(uint tone,uint pos) const
				{
					NST_ASSERT( tone < 16 && pos < 16 );
					return adr[0][tone][pos];
				}

				inline dword Vrc7::Sound::Tables::GetDecay(uint tone,uint pos) const
				{
					NST_ASSERT( tone < 16 && pos < 16 );
					return adr[1][tone][pos];
				}

				inline dword Vrc7::Sound::Tables::GetSustain(uint tone,uint pos) const
				{
					return GetDecay( tone, pos );
				}

				inline dword Vrc7::Sound::Tables::GetRelease(uint tone,uint pos) const
				{
					return GetDecay( tone, pos );
				}

				inline dword Vrc7::Sound::Tables::GetPhase(uint frequency,uint block,uint tone) const
				{
					NST_ASSERT( frequency < 512 && block < 8 && tone < 16 );
					return phase[frequency][block][tone];
				}

				inline Vrc7::Sound::Sample Vrc7::Sound::Tables::GetOutput(uint form,uint pgOut,uint egOut) const
				{
					NST_ASSERT( form < 2 && pgOut < WAVE_SIZE && wave[form][pgOut] + egOut < DB2LIN_SIZE );
					return db2lin[wave[form][pgOut] + egOut];
				}

				void Vrc7::Sound::OpllChannel::UpdateEgPhase(const Tables& tables,const uint i)
				{
					NST_ASSERT( i < NUM_SLOTS );

					switch (slots[i].eg.mode)
					{
						case EG_ATTACK:

							slots[i].eg.phase = tables.GetAttack( patch.tone[4+i] >> 4, slots[i].sl );
							break;

						case EG_DECAY:

							slots[i].eg.phase = tables.GetDecay( patch.tone[4+i] & uint(REG45_DECAY), slots[i].sl );
							break;

						case EG_SUSTAIN:

							slots[i].eg.phase = tables.GetSustain( patch.tone[6+i] & uint(REG67_RELEASE), slots[i].sl );
							break;

						case EG_RELEASE:

							if (i != MODULATOR && sustain)
							{
								slots[i].eg.phase = tables.GetRelease( 5, slots[i].sl );
							}
							else if (patch.tone[0+i] & uint(REG01_HOLD))
							{
								slots[i].eg.phase = tables.GetRelease( patch.tone[6+i] & uint(REG67_RELEASE), slots[i].sl );
							}
							else
							{
								slots[i].eg.phase = tables.GetRelease( 7, slots[i].sl );
							}
							break;

						default:

							slots[i].eg.phase = 0;
							break;
					}
				}

				void Vrc7::Sound::OpllChannel::UpdatePhase(const Tables& tables,const uint i)
				{
					NST_ASSERT( i < NUM_SLOTS );
					slots[i].pg.phase = tables.GetPhase( frequency, block, patch.tone[0+i] & uint(REG01_MULTIPLE) );
				}

				void Vrc7::Sound::OpllChannel::UpdateSustainLevel(const Tables& tables,const uint i)
				{
					NST_ASSERT( i < NUM_SLOTS );
					slots[i].sl = tables.GetSustainLevel( frequency, block, (patch.tone[0+i] & uint(REG01_RATE)) >> 4 );
				}

				void Vrc7::Sound::OpllChannel::UpdateTotalLevel(const Tables& tables,const uint i)
				{
					NST_ASSERT( i < NUM_SLOTS );
					slots[i].tl = tables.GetTotalLevel( frequency, block, (i != MODULATOR) ? volume : (patch.tone[2] & uint(REG2_TOTAL_LEVEL)), patch.tone[2+i] >> 6 );
				}

				void Vrc7::Sound::OpllChannel::Update(const Tables& tables)
				{
					for (uint i=0; i < NUM_SLOTS; ++i)
					{
						UpdateSustainLevel( tables, i );
						UpdateTotalLevel( tables, i );
						UpdateEgPhase( tables, i );
						UpdatePhase( tables, i );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg0(const uint data,const Tables& tables)
				{
					patch.custom[0] = data;

					if (patch.instrument == Patch::CUSTOM)
					{
						patch.tone[0] = data;
						UpdateSustainLevel( tables, 0 );
						UpdateEgPhase( tables, 0 );
						UpdatePhase( tables, 0 );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg1(const uint data,const Tables& tables)
				{
					patch.custom[1] = data;

					if (patch.instrument == Patch::CUSTOM)
					{
						patch.tone[1] = data;
						UpdateSustainLevel( tables, 1 );
						UpdateEgPhase( tables, 1 );
						UpdatePhase( tables, 1 );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg2(const uint data,const Tables& tables)
				{
					patch.custom[2] = data;

					if (patch.instrument == Patch::CUSTOM)
					{
						patch.tone[2] = data;
						UpdateTotalLevel( tables, 0 );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg3(const uint data)
				{
					patch.custom[3] = data;

					if (patch.instrument == Patch::CUSTOM)
						patch.tone[3] = data;
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg4(const uint data,const Tables& tables)
				{
					patch.custom[4] = data;

					if (patch.instrument == Patch::CUSTOM)
					{
						patch.tone[4] = data;
						UpdateEgPhase( tables, 0 );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg5(const uint data,const Tables& tables)
				{
					patch.custom[5] = data;

					if (patch.instrument == Patch::CUSTOM)
					{
						patch.tone[5] = data;
						UpdateEgPhase( tables, 1 );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg6(const uint data,const Tables& tables)
				{
					patch.custom[6] = data;

					if (patch.instrument == Patch::CUSTOM)
					{
						patch.tone[6] = data;
						UpdateEgPhase( tables, 0 );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg7(const uint data,const Tables& tables)
				{
					patch.custom[7] = data;

					if (patch.instrument == Patch::CUSTOM)
					{
						patch.tone[7] = data;
						UpdateEgPhase( tables, 1 );
					}
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg8(const uint data,const Tables& tables)
				{
					frequency = (frequency & (uint(REG9_FRQ_HI) << 8)) | data;
					Update( tables );
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteReg9(const uint data,const Tables& tables)
				{
					frequency = (frequency & REG8_FRQ_LO) | ((data & REG9_FRQ_HI) << 8);
					block = (data & REG9_BLOCK) >> 1;
					sustain = data & REG9_SUSTAIN;

					if ((data ^ key) & REG9_KEY)
					{
						key = data & REG9_KEY;

						if (key)
						{
							for (uint i=0; i < NUM_SLOTS; ++i)
							{
								slots[i].eg.mode = EG_ATTACK;
								slots[i].eg.counter = 0;
								slots[i].pg.counter = 0;
							}
						}
						else
						{
							if (slots[CARRIER].eg.mode == EG_ATTACK)
								slots[CARRIER].eg.counter = dword(tables.GetLog( slots[CARRIER].eg.counter >> EG_PHASE_SHIFT )) << EG_PHASE_SHIFT;

							slots[CARRIER].eg.mode = EG_RELEASE;
						}
					}

					Update( tables );
				}

				NST_SINGLE_CALL void Vrc7::Sound::OpllChannel::WriteRegA(uint data,const Tables& tables)
				{
					volume = (data & REGA_VOLUME) << 2;
					data >>= 4;

					if (patch.instrument != data)
					{
						patch.instrument = data;
						std::memcpy( patch.tone, patch.instrument == Patch::CUSTOM ? patch.custom : Patch::preset[patch.instrument-1], 8 );
					}

					Update( tables );
				}

				void Vrc7::Sound::WriteReg(const uint data)
				{
					Update();

					switch (regSelect & 0x3F)
					{
						case 0x00:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg0( data, tables );

							break;

						case 0x01:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg1( data, tables );

							break;

						case 0x02:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg2( data, tables );

							break;

						case 0x03:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg3( data );

							break;

						case 0x04:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg4( data, tables );

							break;

						case 0x05:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg5( data, tables );

							break;

						case 0x06:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg6( data, tables );

							break;

						case 0x07:

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								channels[i].WriteReg7( data, tables );

							break;

						case 0x10:
						case 0x11:
						case 0x12:
						case 0x13:
						case 0x14:
						case 0x15:

							channels[regSelect - 0x10].WriteReg8( data, tables );
							break;

						case 0x20:
						case 0x21:
						case 0x22:
						case 0x23:
						case 0x24:
						case 0x25:

							channels[regSelect - 0x20].WriteReg9( data, tables );
							break;

						case 0x30:
						case 0x31:
						case 0x32:
						case 0x33:
						case 0x34:
						case 0x35:

							channels[regSelect - 0x30].WriteRegA( data, tables );
							break;
					}
				}

				NST_SINGLE_CALL Vrc7::Sound::Sample Vrc7::Sound::OpllChannel::GetSample(const uint pitch,const uint amp,const Tables& tables)
				{
					uint pgOut[NUM_SLOTS], egOut[NUM_SLOTS];

					for (uint i=0; i < NUM_SLOTS; ++i)
					{
						if (patch.tone[i] & uint(REG01_USE_VIBRATO))
							slots[i].pg.counter += (slots[i].pg.phase * pitch) >> AMP_SHIFT;
						else
							slots[i].pg.counter += slots[i].pg.phase;

						slots[i].pg.counter &= PG_PHASE_RANGE;
						pgOut[i] = slots[i].pg.counter >> PG_PHASE_SHIFT;
						egOut[i] = slots[i].eg.counter >> EG_PHASE_SHIFT;

						switch (slots[i].eg.mode)
						{
							case EG_ATTACK:

								egOut[i] = tables.GetLog( egOut[i] );
								slots[i].eg.counter += slots[i].eg.phase;

								if (slots[i].eg.counter >= EG_BEGIN || (patch.tone[4+i] & uint(REG45_ATTACK)) == REG45_ATTACK)
								{
									egOut[i] = 0;
									slots[i].eg.counter = 0;
									slots[i].eg.mode = EG_DECAY;
									UpdateEgPhase( tables, i );
								}
								break;

							case EG_DECAY:
							{
								slots[i].eg.counter += slots[i].eg.phase;

								dword level = patch.tone[6+i] & uint(REG67_SUSTAIN_LEVEL);

								if (level == REG67_SUSTAIN_LEVEL)
									level = SUSTAIN_LEVEL_MAX;

								level <<= (EG_PHASE_SHIFT-1);

								if (slots[i].eg.counter >= level)
								{
									slots[i].eg.counter = level;
									slots[i].eg.mode = (patch.tone[0+i] & uint(REG01_HOLD)) ? EG_HOLD : EG_SUSTAIN;
									UpdateEgPhase( tables, i );
								}
								break;
							}

							case EG_HOLD:

								if (!(patch.tone[0+i] & uint(REG01_HOLD)))
								{
									slots[i].eg.mode = EG_SUSTAIN;
									UpdateEgPhase( tables, i );
								}
								break;

							case EG_SUSTAIN:
							case EG_RELEASE:

								slots[i].eg.counter += slots[i].eg.phase;

								if (egOut[i] <= EG_END)
									break;

								slots[i].eg.mode = EG_FINISH;

							default:

								egOut[i] = EG_END;
								break;
						}

						egOut[i] = (egOut[i] + slots[i].tl) * 2;

						if (patch.tone[i+0] & uint(REG01_USE_AMP))
							egOut[i] += amp;
					}

					if (slots[CARRIER].eg.mode == EG_FINISH)
						return 0;

					Sample output = slots[MODULATOR].output;

					if (egOut[MODULATOR] >= EG_MUTE)
					{
						slots[MODULATOR].output = 0;
					}
					else
					{
						if (const uint fb = (patch.tone[3] & uint(REG3_FEEDBACK)))
							pgOut[MODULATOR] = uint(int(pgOut[MODULATOR]) + signed_shr(feedback,FEEDBACK_SHIFT-fb)) & WAVE_RANGE;

						slots[MODULATOR].output = tables.GetOutput( (patch.tone[3] & uint(REG3_MODULATED_WAVE)) >> 3, pgOut[MODULATOR], egOut[MODULATOR] );
					}

					feedback = (output + slots[MODULATOR].output) / 2;
					output = slots[CARRIER].output;

					if (egOut[CARRIER] >= EG_MUTE)
						slots[CARRIER].output = 0;
					else
						slots[CARRIER].output = tables.GetOutput( (patch.tone[3] & uint(REG3_CARRIER_WAVE)) >> 4, uint(int(pgOut[CARRIER]) + feedback) & WAVE_RANGE, egOut[CARRIER] );

					return (output + slots[CARRIER].output) / 2;
				}

				Vrc7::Sound::Sample Vrc7::Sound::GetSample()
				{
					if (output)
					{
						while (samplePhase < sampleRate)
						{
							samplePhase += CLOCK_RATE;

							pitchPhase = (pitchPhase + PITCH_RATE) & PITCH_RANGE;
							ampPhase = (ampPhase + AMP_RATE) & AMP_RANGE;

							const uint lfo[2] =
							{
								tables.GetPitch( pitchPhase >> PITCH_SHIFT ),
								tables.GetAmp( ampPhase >> AMP_SHIFT )
							};

							prevSample = nextSample;
							nextSample = 0;

							for (uint i=0; i < NUM_OPLL_CHANNELS; ++i)
								nextSample += channels[i].GetSample( lfo[0], lfo[1], tables );
						}

						samplePhase -= sampleRate;

						return signed_shl( (prevSample * idword(samplePhase) + nextSample * idword(CLOCK_RATE - samplePhase)) / idword(CLOCK_RATE), 3 ) * idword(output) / DEFAULT_VOLUME;
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}
}
