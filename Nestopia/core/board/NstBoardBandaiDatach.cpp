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
#include "../NstClock.hpp"
#include "NstBoard.hpp"
#include "NstBoardBandai.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Datach::Datach(const Context& c)
				:
				Lz93d50Ex     (c),
				barcodeReader (*c.cpu)
				{
				}

				Datach::Reader::Reader(Cpu& c)
				: cpu(c)
				{
					Reset( false );
				}

				Datach::Device Datach::QueryDevice(DeviceType devType)
				{
					if (devType == DEVICE_BARCODE_READER)
						return static_cast<BarcodeReader*>(&barcodeReader);
					else
						return Lz93d50Ex::QueryDevice( devType );
				}

				void Datach::SubReset(const bool hard)
				{
					Lz93d50Ex::SubReset( hard );

					barcodeReader.Reset();
					p6000 = cpu.Map( 0x6000 );

					for (uint i=0x6000; i < 0x8000; i += 0x100)
						Map( i, &Datach::Peek_6000 );
				}

				void Datach::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','D','A'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'B','R','C'>::V)
								barcodeReader.LoadState( state );

							state.End();
						}
					}
					else
					{
						Lz93d50Ex::SubLoad( state, baseChunk );
					}
				}

				void Datach::SubSave(State::Saver& state) const
				{
					Lz93d50Ex::SubSave( state );

					state.Begin( AsciiId<'B','D','A'>::V );
					barcodeReader.SaveState( state, AsciiId<'B','R','C'>::V );
					state.End();
				}

				void Datach::Reader::Reset(bool initHook)
				{
					cycles = Cpu::CYCLE_MAX;
					output = 0x00;
					stream = data;
					std::memset( data, END, MAX_DATA_LENGTH );

					if (initHook)
						cpu.AddHook( Hook(this,&Reader::Hook_Fetcher) );
				}

				bool Datach::Reader::IsTransferring() const
				{
					return *stream != END;
				}

				bool Datach::Reader::IsDigitsSupported(uint count) const
				{
					return count == MIN_DIGITS || count == MAX_DIGITS;
				}

				void Datach::Reader::LoadState(State::Loader& state)
				{
					Reset( false );

					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'P','T','R'>::V:

								stream = data + (state.Read8() & (MAX_DATA_LENGTH-1));
								break;

							case AsciiId<'D','A','T'>::V:

								state.Uncompress( data );
								data[MAX_DATA_LENGTH-1] = END;
								break;

							case AsciiId<'C','Y','C'>::V:

								cycles = state.Read16();
								break;
						}

						state.End();
					}

					if (Reader::IsTransferring())
					{
						output = (stream != data) ? stream[-1] : 0x00;

						if (cycles > CC_INTERVAL)
							cycles = CC_INTERVAL;

						cycles = cpu.GetCycles() + cpu.GetClock() * cycles;
					}
					else
					{
						cycles = Cpu::CYCLE_MAX;
						output = 0x00;
					}
				}

				void Datach::Reader::SaveState(State::Saver& state,const dword baseChunk) const
				{
					if (Reader::IsTransferring())
					{
						NST_ASSERT( cycles != Cpu::CYCLE_MAX );

						state.Begin( baseChunk );

						state.Begin( AsciiId<'P','T','R'>::V ).Write8( stream - data ).End();
						state.Begin( AsciiId<'D','A','T'>::V ).Compress( data ).End();

						uint next;

						if (cycles > cpu.GetCycles())
							next = (cycles - cpu.GetCycles()) / cpu.GetClock();
						else
							next = 0;

						state.Begin( AsciiId<'C','Y','C'>::V ).Write16( next ).End();

						state.End();
					}
				}

				bool Datach::Reader::Transfer(cstring const string,const uint length)
				{
					Reset( false );

					if (!string || (length != MAX_DIGITS && length != MIN_DIGITS))
						return false;

					static const byte prefixParityType[10][6] =
					{
						{8,8,8,8,8,8}, {8,8,0,8,0,0},
						{8,8,0,0,8,0}, {8,8,0,0,0,8},
						{8,0,8,8,0,0}, {8,0,0,8,8,0},
						{8,0,0,0,8,8}, {8,0,8,0,8,0},
						{8,0,8,0,0,8}, {8,0,0,8,0,8}
					};

					static const byte dataLeftOdd[10][7] =
					{
						{8,8,8,0,0,8,0}, {8,8,0,0,8,8,0},
						{8,8,0,8,8,0,0}, {8,0,0,0,0,8,0},
						{8,0,8,8,8,0,0}, {8,0,0,8,8,8,0},
						{8,0,8,0,0,0,0}, {8,0,0,0,8,0,0},
						{8,0,0,8,0,0,0}, {8,8,8,0,8,0,0}
					};

					static const byte dataLeftEven[10][7] =
					{
						{8,0,8,8,0,0,0}, {8,0,0,8,8,0,0},
						{8,8,0,0,8,0,0}, {8,0,8,8,8,8,0},
						{8,8,0,0,0,8,0}, {8,0,0,0,8,8,0},
						{8,8,8,8,0,8,0}, {8,8,0,8,8,8,0},
						{8,8,8,0,8,8,0}, {8,8,0,8,0,0,0}
					};

					static const byte dataRight[10][7] =
					{
						{0,0,0,8,8,0,8}, {0,0,8,8,0,0,8},
						{0,0,8,0,0,8,8}, {0,8,8,8,8,0,8},
						{0,8,0,0,0,8,8}, {0,8,8,0,0,0,8},
						{0,8,0,8,8,8,8}, {0,8,8,8,0,8,8},
						{0,8,8,0,8,8,8}, {0,0,0,8,0,8,8}
					};

					byte code[16];

					for (uint i=0; i < length; ++i)
					{
						if (string[i] >= '0' && string[i] <= '9')
							code[i] = string[i] - '0';
						else
							return false;
					}

					byte* NST_RESTRICT output = data;

					for (uint i=0; i < 1+32; ++i)
						*output++ = 8;

					*output++ = 0;
					*output++ = 8;
					*output++ = 0;

					uint sum = 0;

					if (length == MAX_DIGITS)
					{
						for (uint i=0; i < 6; ++i)
						{
							if (prefixParityType[code[0]][i])
							{
								for (uint j=0; j < 7; ++j)
									*output++ = dataLeftOdd[code[i+1]][j];
							}
							else
							{
								for (uint j=0; j < 7; ++j)
									*output++ = dataLeftEven[code[i+1]][j];
							}
						}

						*output++ = 8;
						*output++ = 0;
						*output++ = 8;
						*output++ = 0;
						*output++ = 8;

						for (uint i=7; i < 12; ++i)
						{
							for (uint j=0; j < 7; ++j)
								*output++ = dataRight[code[i]][j];
						}

						for (uint i=0; i < 12; ++i)
							sum += (i & 1) ? (code[i] * 3) : (code[i] * 1);
					}
					else
					{
						for (uint i=0; i < 4; ++i)
						{
							for (uint j=0; j < 7; ++j)
								*output++ = dataLeftOdd[code[i]][j];
						}

						*output++ = 8;
						*output++ = 0;
						*output++ = 8;
						*output++ = 0;
						*output++ = 8;

						for (uint i=4; i < 7; ++i)
						{
							for (uint j=0; j < 7; ++j)
								*output++ = dataRight[code[i]][j];
						}

						for (uint i=0; i < 7; ++i)
							sum += (i & 1) ? (code[i] * 1) : (code[i] * 3);
					}

					sum = (10 - (sum % 10)) % 10;

					for (uint i=0; i < 7; ++i)
						*output++ = dataRight[sum][i];

					*output++ = 0;
					*output++ = 8;
					*output++ = 0;

					for (uint i=0; i < 32; ++i)
						*output++ = 8;

					cycles = cpu.GetCycles() + cpu.GetClock() * CC_INTERVAL;

					return true;
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_HOOK(Datach::Reader,Fetcher)
				{
					while (cycles <= cpu.GetCycles())
					{
						output = *stream;
						stream += (output != END);

						if (output != END)
						{
							cycles += cpu.GetClock() * CC_INTERVAL;
						}
						else
						{
							output = 0x00;
							cycles = Cpu::CYCLE_MAX;
							break;
						}
					}
				}

				inline uint Datach::Reader::GetOutput() const
				{
					return output;
				}

				NES_PEEK_A(Datach,6000)
				{
					return barcodeReader.GetOutput() | p6000.Peek( address );
				}

				inline void Datach::Reader::Sync()
				{
					if (cycles != Cpu::CYCLE_MAX)
					{
						if (cycles >= cpu.GetFrameCycles())
							cycles -= cpu.GetFrameCycles();
						else
							cycles = 0;
					}
				}

				void Datach::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						barcodeReader.Sync();

					Lz93d50Ex::Sync( event, controllers );
				}
			}
		}
	}
}
