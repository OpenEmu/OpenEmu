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
#include "NstBoardSunsoftFme7.hpp"
#include "NstBoardSunsoft5b.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sunsoft
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				const word S5b::Sound::levels[32] =
				{
					// 32 levels, 1.5dB per step
					0,89,106,127,152,181,216,257,306,364,433,515,613,729,867,1031,1226,1458,
					1733,2060,2449,2911,3460,4113,4889,5811,6907,8209,9757,11597,13784,16383
				};

				S5b::Sound::Sound(Apu& a,bool connect)
				: Channel(a), fixed(1)
				{
					Reset();
					bool audible = UpdateSettings();

					if (connect)
						Connect( audible );
				}

				S5b::S5b(const Context& c)
				:
				Fme7  (c),
				sound (*c.apu)
				{}

				void S5b::Sound::Envelope::Reset(const uint fixed)
				{
					holding = false;
					hold = 0;
					alternate = 0;
					attack = 0;
					timer = 0;
					frequency = 1 * 8 * fixed;
					length = 0;
					count = 0;
					volume = 0;
				}

				void S5b::Sound::Noise::Reset(const uint fixed)
				{
					timer = 0;
					frequency = 1 * 16 * fixed;
					length = 0;
					rng = 1;
					dc = 0;
				}

				void S5b::Sound::Square::Reset(const uint fixed)
				{
					timer = 0;
					frequency = 1 * 16 * fixed;
					status = 0;
					ctrl = 0;
					volume = 0;
					dc = 0;
					length = 0;
				}

				void S5b::Sound::Reset()
				{
					active = false;

					regSelect = 0x0;
					envelope.Reset( fixed );

					for (uint i=0; i < NUM_SQUARES; ++i)
						squares[i].Reset( fixed );

					noise.Reset( fixed );
					dcBlocker.Reset();
				}

				void S5b::SubReset(const bool hard)
				{
					Fme7::SubReset( hard );

					Map( 0xC000U, 0xDFFFU, &S5b::Poke_C000 );
					Map( 0xE000U, 0xFFFFU, &S5b::Poke_E000 );
				}

				void S5b::Sound::Envelope::UpdateSettings(const uint fixed)
				{
					timer = 0;
					UpdateFrequency( fixed );
				}

				void S5b::Sound::Noise::UpdateSettings(const uint fixed)
				{
					timer = 0;
					UpdateFrequency( fixed );
				}

				void S5b::Sound::Square::UpdateSettings(const uint fixed)
				{
					timer = 0;
					UpdateFrequency( fixed );
				}

				bool S5b::Sound::UpdateSettings()
				{
					output = GetVolume(EXT_S5B) * 94U / DEFAULT_VOLUME;

					GetOscillatorClock( rate, fixed );

					envelope.UpdateSettings( fixed );

					for (uint i=0; i < NUM_SQUARES; ++i)
						squares[i].UpdateSettings( fixed );

					noise.UpdateSettings( fixed );

					dcBlocker.Reset();

					return output;
				}

				void S5b::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'S','5','B'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'S','N','D'>::V)
								sound.LoadState( state );

							state.End();
						}
					}
					else
					{
						Fme7::SubLoad( state, baseChunk );
					}
				}

				void S5b::SubSave(State::Saver& state) const
				{
					Fme7::SubSave( state );

					state.Begin( AsciiId<'S','5','B'>::V );
					sound.SaveState( state, AsciiId<'S','N','D'>::V );
					state.End();
				}

				void S5b::Sound::SaveState(State::Saver& state,const dword baseChunk) const
				{
					state.Begin( baseChunk );

					state.Begin( AsciiId<'R','E','G'>::V ).Write8( regSelect ).End();

					envelope.SaveState   ( state, AsciiId<'E','N','V'>::V );
					noise.SaveState      ( state, AsciiId<'N','O','I'>::V );
					squares[0].SaveState ( state, AsciiId<'S','Q','0'>::V );
					squares[1].SaveState ( state, AsciiId<'S','Q','1'>::V );
					squares[2].SaveState ( state, AsciiId<'S','Q','2'>::V );

					state.End();
				}

				void S5b::Sound::LoadState(State::Loader& state)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:

								regSelect = state.Read8();
								break;

							case AsciiId<'E','N','V'>::V:

								envelope.LoadState( state, fixed );
								break;

							case AsciiId<'N','O','I'>::V:

								noise.LoadState( state, fixed );
								break;

							case AsciiId<'S','Q','0'>::V:

								squares[0].LoadState( state, fixed );
								break;

							case AsciiId<'S','Q','1'>::V:

								squares[1].LoadState( state, fixed );
								break;

							case AsciiId<'S','Q','2'>::V:

								squares[2].LoadState( state, fixed );
								break;
						}

						state.End();
					}
				}

				void S5b::Sound::Envelope::SaveState(State::Saver& state,const dword chunk) const
				{
					const byte data[4] =
					{
						(holding   ? 0x1U : 0x0U) |
						(hold      ? 0x2U : 0x1U) |
						(alternate ? 0x4U : 0x0U) |
						(attack    ? 0x8U : 0x0U),
						count,
						length & 0xFF,
						length >> 8
					};

					state.Begin( chunk ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void S5b::Sound::Noise::SaveState(State::Saver& state,const dword chunk) const
				{
					state.Begin( chunk ).Begin( AsciiId<'R','E','G'>::V ).Write8( length ).End().End();
				}

				void S5b::Sound::Square::SaveState(State::Saver& state,const dword chunk) const
				{
					const byte data[3] =
					{
						(~status & 0x1) | (ctrl << 1),
						length & 0xFF,
						(length >> 8) | ((status & 0x8) << 1),
					};

					state.Begin( chunk ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void S5b::Sound::Envelope::LoadState(State::Loader& state,const uint fixed)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<4> data( state );

							holding = data[0] & 0x1;
							hold = data[0] & 0x2;
							alternate = data[0] & 0x4;
							attack = (data[0] & 0x8) ? 0x1F : 0x00;
							count = data[1] & 0x1F;
							length = data[2] | (data[3] << 8 & 0xF00);
							volume = levels[count ^ attack];

							UpdateSettings( fixed );
						}

						state.End();
					}
				}

				void S5b::Sound::Noise::LoadState(State::Loader& state,const uint fixed)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							length = state.Read8() & 0x1F;
							dc = 0;
							rng = 1;

							UpdateSettings( fixed );
						}

						state.End();
					}
				}

				void S5b::Sound::Square::LoadState(State::Loader& state,const uint fixed)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<3> data( state );

							status = (~data[0] & 0x1) | (data[2] >> 1 & 0x8);
							ctrl = data[0] >> 1 & 0x1F;
							length = data[1] | (data[2] << 8 & 0xF00);
							volume = levels[(ctrl & 0xF) ? (ctrl & 0xF) * 2 + 1 : 0];
							dc = (status & 0x1) ? ~0UL : 0UL;

							UpdateSettings( fixed );
						}

						state.End();
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(S5b,C000)
				{
					sound.SelectReg( data );
				}

				void S5b::Sound::Square::UpdateFrequency(const uint fixed)
				{
					const idword prev = frequency;
					frequency = NST_MAX(length,1) * 16 * fixed;
					timer = NST_MAX(timer + idword(frequency) - prev,0);
				}

				void S5b::Sound::Square::WriteReg0(const uint data,const uint fixed)
				{
					length = (length & 0x0F00) | data;
					UpdateFrequency( fixed );
				}

				void S5b::Sound::Square::WriteReg1(const uint data,const uint fixed)
				{
					length = (length & 0x00FF) | (data & 0xF) << 8;
					UpdateFrequency( fixed );
				}

				void S5b::Sound::Square::WriteReg2(const uint data)
				{
					status = data & (0x1|0x8);

					if (status & 0x1)
						dc = (dword)~0UL;
				}

				void S5b::Sound::Square::WriteReg3(const uint data)
				{
					ctrl = data & 0x1F;
					volume = levels[(ctrl & 0xF) ? (ctrl & 0xF) * 2 + 1 : 0];
				}

				void S5b::Sound::Envelope::UpdateFrequency(const uint fixed)
				{
					const idword prev = frequency;
					frequency = NST_MAX(length*16,1*8) * fixed;
					timer = NST_MAX(timer + idword(frequency) - prev,0);
				}

				void S5b::Sound::Envelope::WriteReg0(const uint data,const uint fixed)
				{
					length = (length & 0xFF00) | data << 0;
					UpdateFrequency( fixed );
				}

				void S5b::Sound::Envelope::WriteReg1(const uint data,const uint fixed)
				{
					length = (length & 0x00FF) | data << 8;
					UpdateFrequency( fixed );
				}

				void S5b::Sound::Envelope::WriteReg2(const uint data)
				{
					holding = false;
					attack = (data & 0x04) ? 0x1F : 0x00;

					if (data & 0x8)
					{
						hold = data & 0x1;
						alternate = data & 0x2;
					}
					else
					{
						hold = 1;
						alternate = attack;
					}

					timer = frequency;
					count = 0x1F;
					volume = levels[count ^ attack];
				}

				void S5b::Sound::Noise::UpdateFrequency(const uint fixed)
				{
					const idword prev = frequency;
					frequency = NST_MAX(length,1) * 16 * fixed;
					timer = NST_MAX(timer + idword(frequency) - prev,0);
				}

				void S5b::Sound::Noise::WriteReg(const uint data,const uint fixed)
				{
					length = data & 0x1F;
					UpdateFrequency( fixed );
				}

				void S5b::Sound::WriteReg(const uint data)
				{
					Update();
					active = true;

					switch (regSelect & 0xF)
					{
						case 0x0:
						case 0x2:
						case 0x4:

							squares[regSelect >> 1].WriteReg0( data, fixed );
							break;

						case 0x1:
						case 0x3:
						case 0x5:

							squares[regSelect >> 1].WriteReg1( data, fixed );
							break;

						case 0x6:

							noise.WriteReg( data, fixed );
							break;

						case 0x7:

							for (uint i=0; i < NUM_SQUARES; ++i)
								squares[i].WriteReg2( data >> i );

							break;

						case 0x8:
						case 0x9:
						case 0xA:

							squares[regSelect - 0x8].WriteReg3( data );
							break;

						case 0xB:

							envelope.WriteReg0( data, fixed );
							break;

						case 0xC:

							envelope.WriteReg1( data, fixed );
							break;

						case 0xD:

							envelope.WriteReg2( data );
							break;
					}
				}

				NES_POKE_D(S5b,E000)
				{
					sound.WriteReg( data );
				}

				NST_SINGLE_CALL dword S5b::Sound::Envelope::Clock(const Cycle rate)
				{
					if (!holding)
					{
						timer -= idword(rate);

						if (timer < 0)
						{
							do
							{
								--count;
								timer += idword(frequency);
							}
							while (timer < 0);

							if (count > 0x1F)
							{
								if (hold)
								{
									if (alternate)
										attack ^= 0x1FU;

									holding = true;
									count = 0x00;
								}
								else
								{
									if (alternate && count & 0x20)
										attack ^= 0x1FU;

									count = 0x1F;
								}
							}

							volume = levels[count ^ attack];
						}
					}

					return volume;
				}

				NST_SINGLE_CALL dword S5b::Sound::Noise::Clock(const Cycle rate)
				{
					for (timer -= idword(rate); timer < 0; timer += idword(frequency))
					{
						if ((rng + 1) & 0x2) dc = ~dc;
						if ((rng + 0) & 0x1) rng ^= 0x24000;

						rng >>= 1;
					}

					return dc;
				}

				NST_SINGLE_CALL dword S5b::Sound::Square::GetSample(const Cycle rate,const uint envelope,const uint noise)
				{
					dword sum = timer;
					timer -= idword(rate);

					const uint out = (ctrl & 0x10) ? envelope : volume;

					if ((noise|status) & 0x8 && out)
					{
						if (timer >= 0)
						{
							return out & dc;
						}
						else
						{
							sum &= dc;

							do
							{
								dc ^= (status & 0x1) - 1UL;
								sum += NST_MIN(dword(-timer),frequency) & dc;
								timer += idword(frequency);
							}
							while (timer < 0);

							NST_VERIFY( sum <= 0xFFFFFFFF / out + rate/2 );
							return (sum * out + rate/2) / rate;
						}
					}
					else
					{
						while (timer < 0)
						{
							dc ^= (status & 0x1) - 1UL;
							timer += idword(frequency);
						}

						return 0;
					}
				}

				S5b::Sound::Sample S5b::Sound::GetSample()
				{
					if (active && output)
					{
						dword sample = 0;

						for (dword i=0, e=envelope.Clock( rate ), n=noise.Clock( rate ); i < NUM_SQUARES; ++i)
							sample += squares[i].GetSample( rate, e, n );

						return dcBlocker.Apply( sample * output / DEFAULT_VOLUME );
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
