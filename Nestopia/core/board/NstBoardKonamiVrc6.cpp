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
#include "../NstClock.hpp"
#include "NstBoardKonamiVrc4.hpp"
#include "NstBoardKonamiVrc6.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Konami
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Vrc6::Sound::Sound(Apu& a,bool connect)
				: Channel(a)
				{
					Reset();
					bool audible = UpdateSettings();

					if (connect)
						Connect( audible );
				}

				uint Vrc6::GetPrgLineShift(const Context& c,const uint pin,const uint def)
				{
					if (const Chips::Type* const vrc6 = c.chips.Find(L"Konami VRC VI"))
					{
						const uint line = *vrc6->Pin(pin).C(L"PRG").A();
						NST_VERIFY( line < 8 );

						if (line < 8)
							return line;
					}

					return def;
				}

				Vrc6::Vrc6(const Context& c)
				:
				Board    (c),
				irq      (*c.cpu),
				sound    (*c.apu),
				prgLineA (GetPrgLineShift(c, 9,1)),
				prgLineB (GetPrgLineShift(c,10,0))
				{}

				void Vrc6::Sound::BaseChannel::Reset()
				{
					enabled = false;
					waveLength = 1;
					active = false;
					timer = 0;
					frequency = 0;
					step = 0;
				}

				void Vrc6::Sound::Square::Reset()
				{
					BaseChannel::Reset();

					duty = 1;
					volume = 0;
					digitized = false;
				}

				void Vrc6::Sound::Saw::Reset()
				{
					BaseChannel::Reset();

					phase = 0;
					amp = 0;
					frequency = 0;
				}

				void Vrc6::Sound::Reset()
				{
					for (uint i=0; i < 2; ++i)
						square[i].Reset();

					saw.Reset();
					dcBlocker.Reset();
				}

				void Vrc6::SubReset(const bool hard)
				{
					irq.Reset( hard, hard ? false : irq.Connected() );

					Map( 0x8000U, 0x8FFFU, PRG_SWAP_16K_0 );
					Map( 0xC000U, 0xCFFFU, PRG_SWAP_8K_2  );

					for (dword i=0x9000, a=9-prgLineA, b=8-prgLineB; i <= 0xFFFF; ++i)
					{
						switch ((i & 0xF000) | (i << a & 0x0200) | (i << b & 0x0100))
						{
							case 0x9000: Map( i, &Vrc6::Poke_9000 ); break;
							case 0x9100: Map( i, &Vrc6::Poke_9001 ); break;
							case 0x9200: Map( i, &Vrc6::Poke_9002 ); break;
							case 0xA000: Map( i, &Vrc6::Poke_A000 ); break;
							case 0xA100: Map( i, &Vrc6::Poke_A001 ); break;
							case 0xA200: Map( i, &Vrc6::Poke_A002 ); break;
							case 0xB000: Map( i, &Vrc6::Poke_B000 ); break;
							case 0xB100: Map( i, &Vrc6::Poke_B001 ); break;
							case 0xB200: Map( i, &Vrc6::Poke_B002 ); break;
							case 0xB300: Map( i, &Vrc6::Poke_B003 ); break;
							case 0xD000: Map( i, CHR_SWAP_1K_0    ); break;
							case 0xD100: Map( i, CHR_SWAP_1K_1    ); break;
							case 0xD200: Map( i, CHR_SWAP_1K_2    ); break;
							case 0xD300: Map( i, CHR_SWAP_1K_3    ); break;
							case 0xE000: Map( i, CHR_SWAP_1K_4    ); break;
							case 0xE100: Map( i, CHR_SWAP_1K_5    ); break;
							case 0xE200: Map( i, CHR_SWAP_1K_6    ); break;
							case 0xE300: Map( i, CHR_SWAP_1K_7    ); break;
							case 0xF000: Map( i, &Vrc6::Poke_F000 ); break;
							case 0xF100: Map( i, &Vrc6::Poke_F001 ); break;
							case 0xF200: Map( i, &Vrc6::Poke_F002 ); break;
						}
					}
				}

				bool Vrc6::Sound::Square::CanOutput() const
				{
					return volume && enabled && !digitized && waveLength >= MIN_FRQ;
				}

				bool Vrc6::Sound::Saw::CanOutput() const
				{
					return enabled && phase && waveLength >= MIN_FRQ;
				}

				void Vrc6::Sound::Square::UpdateSettings(const uint fixed)
				{
					active = CanOutput();
					frequency = (waveLength + 1U) * fixed;
				}

				void Vrc6::Sound::Saw::UpdateSettings(const uint fixed)
				{
					active = CanOutput();
					frequency = ((waveLength + 1UL) << FRQ_SHIFT) * fixed;
				}

				bool Vrc6::Sound::UpdateSettings()
				{
					output = GetVolume(EXT_VRC6);

					GetOscillatorClock( rate, fixed );

					for (uint i=0; i < 2; ++i)
						square[i].UpdateSettings( fixed );

					saw.UpdateSettings( fixed );

					dcBlocker.Reset();

					return output;
				}

				void Vrc6::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'K','V','6'>::V) );

					if (baseChunk == AsciiId<'K','V','6'>::V)
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

				void Vrc6::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','V','6'>::V );

					irq.SaveState( state, AsciiId<'I','R','Q'>::V );
					sound.SaveState( state, AsciiId<'S','N','D'>::V );

					state.End();
				}

				void Vrc6::Sound::SaveState(State::Saver& state,const dword baseChunk) const
				{
					state.Begin( baseChunk );

					square[0].SaveState( state, AsciiId<'S','Q','0'>::V );
					square[1].SaveState( state, AsciiId<'S','Q','1'>::V );
					saw.SaveState( state, AsciiId<'S','A','W'>::V );

					state.End();
				}

				void Vrc6::Sound::LoadState(State::Loader& state)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'S','Q','0'>::V:

								square[0].LoadState( state, fixed );
								break;

							case AsciiId<'S','Q','1'>::V:

								square[1].LoadState( state, fixed );
								break;

							case AsciiId<'S','A','W'>::V:

								saw.LoadState( state, fixed );
								break;
						}

						state.End();
					}
				}

				void Vrc6::Sound::Square::SaveState(State::Saver& state,const dword chunk) const
				{
					const byte data[4] =
					{
						(enabled ? 0x1U : 0x0U) | (digitized ? 0x2U : 0x0U),
						waveLength & 0xFF,
						waveLength >> 8,
						(duty - 1) | ((volume / VOLUME) << 3)
					};

					state.Begin( chunk ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void Vrc6::Sound::Square::LoadState(State::Loader& state,const uint fixed)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<4> data( state );

							enabled = data[0] & 0x1;
							digitized = data[0] & 0x2;
							waveLength = data[1] | (data[2] << 8 & 0xF00);
							duty = (data[3] & 0x7) + 1;
							volume = (data[3] >> 3 & 0xF) * VOLUME;

							timer = 0;
							step = 0;

							UpdateSettings( fixed );
						}

						state.End();
					}
				}

				void Vrc6::Sound::Saw::SaveState(State::Saver& state,const dword chunk) const
				{
					const byte data[3] =
					{
						(enabled != 0) | (phase << 1),
						waveLength & 0xFF,
						waveLength >> 8
					};

					state.Begin( chunk ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void Vrc6::Sound::Saw::LoadState(State::Loader& state,const uint fixed)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<3> data( state );

							enabled = data[0] & 0x1;
							phase = data[0] >> 1 & 0x3F;
							waveLength = data[1] | (data[2] << 8 & 0xF00);

							timer = 0;
							step = 0;
							amp = 0;

							UpdateSettings( fixed );
						}

						state.End();
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NST_SINGLE_CALL void Vrc6::Sound::Square::WriteReg0(const uint data)
				{
					volume = (data & REG0_VOLUME) * VOLUME;
					duty = ((data & REG0_DUTY) >> REG0_DUTY_SHIFT) + 1;
					digitized = data & REG0_DIGITIZED;
					NST_VERIFY( !digitized );
					active = CanOutput();
				}

				NST_SINGLE_CALL void Vrc6::Sound::Square::WriteReg1(const uint data,const dword fixed)
				{
					waveLength &= uint(REG2_WAVELENGTH_HIGH) << 8;
					waveLength |= data;
					frequency = (waveLength + 1U) * fixed;
					active = CanOutput();
				}

				NST_SINGLE_CALL void Vrc6::Sound::Square::WriteReg2(const uint data,const dword fixed)
				{
					waveLength &= REG1_WAVELENGTH_LOW;
					waveLength |= (data & REG2_WAVELENGTH_HIGH) << 8;
					frequency = (waveLength + 1U) * fixed;
					enabled = data & REG2_ENABLE;
					active = CanOutput();
				}

				NST_SINGLE_CALL void Vrc6::Sound::Saw::WriteReg0(const uint data)
				{
					phase = data & REG0_PHASE;
					active = CanOutput();
				}

				NST_SINGLE_CALL void Vrc6::Sound::Saw::WriteReg1(const uint data,const dword fixed)
				{
					waveLength &= uint(REG2_WAVELENGTH_HIGH) << 8;
					waveLength |= data;
					frequency = ((waveLength + 1UL) << FRQ_SHIFT) * fixed;
					active = CanOutput();
				}

				NST_SINGLE_CALL void Vrc6::Sound::Saw::WriteReg2(const uint data,const dword fixed)
				{
					waveLength &= REG1_WAVELENGTH_LOW;
					waveLength |= (data & REG2_WAVELENGTH_HIGH) << 8;
					frequency = ((waveLength + 1UL) << FRQ_SHIFT) * fixed;
					enabled = data & REG2_ENABLE;
					active = CanOutput();
				}

				void Vrc6::Sound::WriteSquareReg0(uint i,uint data)
				{
					Update();
					square[i].WriteReg0( data );
				}

				void Vrc6::Sound::WriteSquareReg1(uint i,uint data)
				{
					Update();
					square[i].WriteReg1( data, fixed );
				}

				void Vrc6::Sound::WriteSquareReg2(uint i,uint data)
				{
					Update();
					square[i].WriteReg2( data, fixed );
				}

				void Vrc6::Sound::WriteSawReg0(uint data)
				{
					Update();
					saw.WriteReg0( data );
				}

				void Vrc6::Sound::WriteSawReg1(uint data)
				{
					Update();
					saw.WriteReg1( data, fixed );
				}

				void Vrc6::Sound::WriteSawReg2(uint data)
				{
					Update();
					saw.WriteReg2( data, fixed );
				}

				NES_POKE_D(Vrc6,9000) { sound.WriteSquareReg0 ( 0, data ); }
				NES_POKE_D(Vrc6,9001) { sound.WriteSquareReg1 ( 0, data ); }
				NES_POKE_D(Vrc6,9002) { sound.WriteSquareReg2 ( 0, data ); }
				NES_POKE_D(Vrc6,A000) { sound.WriteSquareReg0 ( 1, data ); }
				NES_POKE_D(Vrc6,A001) { sound.WriteSquareReg1 ( 1, data ); }
				NES_POKE_D(Vrc6,A002) { sound.WriteSquareReg2 ( 1, data ); }
				NES_POKE_D(Vrc6,B000) { sound.WriteSawReg0    (    data ); }
				NES_POKE_D(Vrc6,B001) { sound.WriteSawReg1    (    data ); }
				NES_POKE_D(Vrc6,B002) { sound.WriteSawReg2    (    data ); }

				NST_SINGLE_CALL dword Vrc6::Sound::Square::GetSample(const Cycle rate)
				{
					NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );

					if (active)
					{
						dword sum = timer;
						timer -= idword(rate);

						if (timer >= 0)
						{
							return step < duty ? volume : 0;
						}
						else
						{
							if (step >= duty)
								sum = 0;

							do
							{
								step = (step + 1) & 0xF;

								if (step < duty)
									sum += NST_MIN(dword(-timer),frequency);

								timer += idword(frequency);
							}
							while (timer < 0);

							return (sum * volume + (rate/2)) / rate;
						}
					}

					return 0;
				}

				NST_SINGLE_CALL dword Vrc6::Sound::Saw::GetSample(const Cycle rate)
				{
					NST_VERIFY( bool(active) == CanOutput() && timer >= 0 );

					if (active)
					{
						dword sum = timer;
						timer -= idword(rate);

						if (timer >= 0)
						{
							return (amp >> 3) * VOLUME;
						}
						else
						{
							sum *= amp;

							do
							{
								if (++step >= 0x7)
								{
									step = 0;
									amp = 0;
								}

								amp = (amp + phase) & 0xFF;
								sum += NST_MIN(dword(-timer),frequency) * amp;

								timer += idword(frequency);
							}
							while (timer < 0);

							return ((sum >> 3) * VOLUME + (rate / 2)) / rate;
						}
					}

					return 0;
				}

				Vrc6::Sound::Sample Vrc6::Sound::GetSample()
				{
					if (output)
					{
						dword sample = 0;

						for (uint i=0; i < 2; ++i)
							sample += square[i].GetSample( rate );

						sample += saw.GetSample( rate );

						return dcBlocker.Apply( sample * output / DEFAULT_VOLUME );
					}
					else
					{
						return 0;
					}
				}

				NES_POKE_D(Vrc6,B003)
				{
					SetMirroringVH01( data >> 2 );
				}

				NES_POKE_D(Vrc6,F000)
				{
					irq.Update();
					irq.unit.latch = data;
				}

				NES_POKE_D(Vrc6,F001)
				{
					irq.Toggle( data );
				}

				NES_POKE(Vrc6,F002)
				{
					irq.Toggle();
				}

				void Vrc6::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
