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
#include "../NstFile.hpp"
#include "NstBoardNamcot163.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Namcot
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				N163::N163(const Context& c)
				:
				Board (c),
				irq   (*c.cpu),
				sound (*c.apu)
				{
				}

				N163::Sound::Sound(Apu& a,bool connect)
				: Channel(a)
				{
					Reset();
					bool audible = UpdateSettings();

					if (connect)
						Connect( audible );
				}

				void N163::SubReset(const bool hard)
				{
					irq.Reset( hard, hard || irq.Connected() );

					Map( 0x4800U, 0x4FFFU, &N163::Peek_4800, &N163::Poke_4800 );
					Map( 0x5000U, 0x57FFU, &N163::Peek_5000, &N163::Poke_5000 );
					Map( 0x5800U, 0x5FFFU, &N163::Peek_5800, &N163::Poke_5800 );
					Map( 0x8000U, 0x87FFU, CHR_SWAP_1K_0 );
					Map( 0x8800U, 0x8FFFU, CHR_SWAP_1K_1 );
					Map( 0x9000U, 0x97FFU, CHR_SWAP_1K_2 );
					Map( 0x9800U, 0x9FFFU, CHR_SWAP_1K_3 );
					Map( 0xA000U, 0xA7FFU, CHR_SWAP_1K_4 );
					Map( 0xA800U, 0xAFFFU, CHR_SWAP_1K_5 );
					Map( 0xB000U, 0xB7FFU, CHR_SWAP_1K_6 );
					Map( 0xB800U, 0xBFFFU, CHR_SWAP_1K_7 );
					Map( 0xC000U, 0xC7FFU, &N163::Poke_C000 );
					Map( 0xC800U, 0xCFFFU, &N163::Poke_C800 );
					Map( 0xD000U, 0xD7FFU, &N163::Poke_D000 );
					Map( 0xD800U, 0xDFFFU, &N163::Poke_D800 );
					Map( 0xE000U, 0xE7FFU, PRG_SWAP_8K_0 );
					Map( 0xE800U, 0xEFFFU, PRG_SWAP_8K_1 );
					Map( 0xF000U, 0xF7FFU, PRG_SWAP_8K_2 );
					Map( 0xF800U, 0xFFFFU, &N163::Poke_F800 );
				}

				void N163::Irq::Reset(const bool hard)
				{
					if (hard)
						count = 0;
				}

				void N163::Sound::Reset()
				{
					exAddress = 0x00;
					exIncrease = 0x01;
					startChannel = NUM_CHANNELS;
					frequency = 0;

					std::memset( wave, 0, sizeof(wave) );
					std::memset( exRam, 0, sizeof(exRam) );

					for (uint i=0; i < NUM_CHANNELS; ++i)
						channels[i].Reset();

					dcBlocker.Reset();
				}

				void N163::Sound::BaseChannel::Reset()
				{
					enabled = false;
					active = false;
					timer = 0;
					frequency = 0;
					phase = 0;
					waveLength = 0;
					waveOffset = 0;
					volume = 0;
				}

				void N163::Load(File& file)
				{
					if (board.HasBattery() && (board == Type::NAMCOT_163_S_0 || board == Type::NAMCOT_163_S_1))
					{
						const File::LoadBlock block[] =
						{
							{ wrk.Source().Mem(), board.GetWram() },
							{ sound.GetExRam(), Sound::EXRAM_SIZE }
						};

						file.Load( File::BATTERY, block );
					}
					else
					{
						Board::Load( file );
					}
				}

				void N163::Save(File& file) const
				{
					if (board.HasBattery() && (board == Type::NAMCOT_163_S_0 || board == Type::NAMCOT_163_S_1))
					{
						const File::SaveBlock block[] =
						{
							{ wrk.Source().Mem(), board.GetWram() },
							{ sound.GetExRam(), Sound::EXRAM_SIZE }
						};

						file.Save( File::BATTERY, block );
					}
					else
					{
						Board::Save( file );
					}
				}

				byte* N163::Sound::GetExRam()
				{
					return exRam;
				}

				const byte* N163::Sound::GetExRam() const
				{
					return exRam;
				}

				void N163::Sound::SaveState(State::Saver& state,const dword baseChunk) const
				{
					state.Begin( baseChunk );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( exAddress | (exIncrease << 7) ).End();
					state.Begin( AsciiId<'R','A','M'>::V ).Compress( exRam ).End();
					state.End();
				}

				void N163::Sound::LoadState(State::Loader& state)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:
							{
								const uint data = state.Read8();
								exAddress = data & 0x7F;
								exIncrease = data >> 7;
								break;
							}

							case AsciiId<'R','A','M'>::V:

								state.Uncompress( exRam );

								for (uint i=0; i < sizeof(exRam); ++i)
								{
									wave[i*2+0] = (exRam[i] & 0xFU) << 2;
									wave[i*2+1] = (exRam[i] >>   4) << 2;
								}

								for (uint i=64; i < sizeof(exRam); i += 8)
								{
									BaseChannel& channel = channels[(i - 64) >> 3];

									channel.Reset();

									channel.SetFrequency  ( FetchFrequency(i) );
									channel.SetWaveLength ( exRam[i+4] );
									channel.SetWaveOffset ( exRam[i+6] );
									channel.SetVolume     ( exRam[i+7] );

									channel.Validate();
								}

								SetChannelState( exRam[0x7F] );
								break;
						}

						state.End();
					}
				}

				void N163::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'N','6','3'>::V) );

					if (baseChunk == AsciiId<'N','6','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<3> data( state );
									irq.unit.count = data[1] | (data[2] << 8 & 0x7F00) | (data[0] << 15 & 0x8000);
									break;
								}

								case AsciiId<'S','N','D'>::V:

									sound.LoadState( state );
									break;
							}

							state.End();
						}
					}
				}

				void N163::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'N','6','3'>::V );

					const byte data[3] =
					{
						irq.unit.count >> 15,
						irq.unit.count >> 0 & 0xFF,
						irq.unit.count >> 8 & 0x7F
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();

					sound.SaveState( state, AsciiId<'S','N','D'>::V );

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				bool N163::Irq::Clock()
				{
					return (count - 0x8000 < 0x7FFF) && (++count == 0xFFFF);
				}

				inline bool N163::Sound::BaseChannel::CanOutput() const
				{
					return volume && frequency && enabled;
				}

				inline void N163::Sound::BaseChannel::Validate()
				{
					active = CanOutput();
				}

				inline void N163::Sound::BaseChannel::SetFrequency(const uint f)
				{
					frequency = f;
				}

				inline void N163::Sound::BaseChannel::SetWaveLength(const uint data)
				{
					const dword length = (0x20UL - (data & REG_WAVELENGTH)) << PHASE_SHIFT;

					if (waveLength != length)
					{
						waveLength = length;
						phase = 0;
					}

					enabled = data >> REG_ENABLE_SHIFT;
				}

				inline void N163::Sound::BaseChannel::SetWaveOffset(const uint data)
				{
					waveOffset = data;
				}

				inline void N163::Sound::BaseChannel::SetVolume(const uint data)
				{
					volume = (data & REG_VOLUME) * VOLUME;
				}

				inline dword N163::Sound::BaseChannel::GetSample
				(
					const Cycle rate,
					const Cycle factor,
					const byte (&wave)[0x100]
				)
				{
					NST_VERIFY( bool(active) == CanOutput() );

					if (active)
					{
						phase = (phase + (timer + rate) / factor * frequency) % waveLength;
						timer = (timer + rate) % factor;

						return wave[(waveOffset + (phase >> PHASE_SHIFT)) & 0xFF] * dword(volume);
					}

					return 0;
				}

				N163::Sound::Sample N163::Sound::GetSample()
				{
					if (output)
					{
						dword sample = 0;

						for (BaseChannel* channel = channels+startChannel; channel != channels+NUM_CHANNELS; ++channel)
							sample += channel->GetSample( rate, frequency, wave );

						return dcBlocker.Apply( sample * output / DEFAULT_VOLUME );
					}
					else
					{
						return 0;
					}
				}

				bool N163::Sound::UpdateSettings()
				{
					output = GetVolume(EXT_N163) * 68U / DEFAULT_VOLUME;

					rate =
					(
						qword(GetModel() == CPU_RP2A03 ? CLK_NTSC : CLK_PAL) * (1UL << SPEED_SHIFT) /
						(GetSampleRate() * 45UL * (GetModel() == CPU_RP2A03 ? CLK_NTSC_DIV : CLK_PAL_DIV))
					);

					dcBlocker.Reset();

					return output;
				}

				inline void N163::Sound::SetChannelState(uint data)
				{
					data = (data >> 4 & 0x7) + 1;
					frequency = dword(data) << SPEED_SHIFT;
					startChannel = NUM_CHANNELS - data;
				}

				inline dword N163::Sound::FetchFrequency(uint address) const
				{
					address &= 0x78;

					return
					(
						(dword(exRam[address+0x0]       ) <<  0) |
						(dword(exRam[address+0x2]       ) <<  8) |
						(dword(exRam[address+0x4] & 0x3U) << 16)
					);
				}

				inline void N163::Sound::WriteWave(const uint data)
				{
					const uint index = exAddress << 1;
					wave[index+0] = (data & 0xF) << 2;
					wave[index+1] = (data >>  4) << 2;
				}

				uint N163::Sound::ReadData()
				{
					const uint data = exRam[exAddress];
					exAddress = (exAddress + exIncrease) & 0x7F;
					return data;
				}

				NES_PEEK(N163,4800)
				{
					return sound.ReadData();
				}

				void N163::Sound::WriteData(const uint data)
				{
					Update();

					WriteWave( data );
					exRam[exAddress] = data;

					if (exAddress >= 0x40)
					{
						BaseChannel& channel = channels[(exAddress - 0x40) >> 3];

						switch (exAddress & 0x7)
						{
							case 0x4:

								channel.SetWaveLength( data );

							case 0x0:
							case 0x2:

								channel.SetFrequency( FetchFrequency(exAddress) );
								break;

							case 0x6:

								channel.SetWaveOffset( data );
								break;

							case 0x7:

								channel.SetVolume( data );

								if (exAddress == 0x7F)
									SetChannelState( data );

								break;
						}

						channel.Validate();
					}

					exAddress = (exAddress + exIncrease) & 0x7F;
				}

				NES_POKE_D(N163,4800)
				{
					sound.WriteData( data );
				}

				NES_PEEK(N163,5000)
				{
					irq.Update();
					return irq.unit.count & 0xFF;
				}

				NES_POKE_D(N163,5000)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0xFF00) | data;
					irq.ClearIRQ();
				}

				NES_PEEK(N163,5800)
				{
					irq.Update();
					return irq.unit.count >> 8;
				}

				NES_POKE_D(N163,5800)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0x00FF) | (data << 8);
					irq.ClearIRQ();
				}

				void N163::Sound::WriteAddress(const uint data)
				{
					NST_COMPILE_ASSERT( EXRAM_INC == 0x80 );

					exAddress = data & 0x7F;
					exIncrease = data >> 7;
				}

				void N163::SwapNmt(const uint address,const uint data) const
				{
					ppu.Update();
					nmt.Source( data < 0xE0 ).SwapBank<SIZE_1K>( address, data );
				}

				NES_POKE_D(N163,C000)
				{
					SwapNmt( 0x0000, data );
				}

				NES_POKE_D(N163,C800)
				{
					SwapNmt( 0x0400, data );
				}

				NES_POKE_D(N163,D000)
				{
					SwapNmt( 0x0800, data );
				}

				NES_POKE_D(N163,D800)
				{
					SwapNmt( 0x0C00, data );
				}

				NES_POKE_D(N163,F800)
				{
					sound.WriteAddress( data );
				}

				void N163::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
