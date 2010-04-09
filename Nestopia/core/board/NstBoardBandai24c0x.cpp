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
#include "NstBoardBandai24c0x.hpp"

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

				void X24C0X<0>::Reset()
				{
					line.scl      = 0;
					line.sda      = 0;
					mode          = MODE_IDLE;
					next          = MODE_IDLE;
					latch.bit     = 0;
					latch.address = 0;
					latch.data    = 0;
					rw            = false;
					output        = 0x10;
				}

				void X24C0X<0>::SaveState
				(
					State::Saver& state,
					const dword baseChunk,
					const byte* const mem,
					const uint size
				)   const
				{
					state.Begin( baseChunk );

					const byte data[6] =
					{
						line.scl | line.sda,
						uint(mode << 0) | uint(next << 4),
						latch.address,
						latch.data,
						latch.bit,
						output | (rw ? 0x80 : 0x00)
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					state.Begin( AsciiId<'R','A','M'>::V ).Compress( mem, size ).End();

					state.End();
				}

				void X24C0X<0>::LoadState(State::Loader& state,byte* const mem,const uint size)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:
							{
								State::Loader::Data<6> data( state );

								line.scl = data[0] & 0x20;
								line.sda = data[0] & 0x40;

								if ((data[1] & 0xF) < MODE_MAX)
									mode = static_cast<Mode>(data[1] & 0xF);

								if ((data[1] >> 4) < MODE_MAX)
									next = static_cast<Mode>(data[1] >> 4);

								latch.address = data[2] & (size-1);
								latch.data = data[3];
								latch.bit = NST_MAX(data[4],8);

								rw = data[5] & 0x80;
								output = data[5] & 0x10;
								break;
							}

							case AsciiId<'R','A','M'>::V:

								state.Uncompress( mem, size );
								break;
						}

						state.End();
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void X24C0X<0>::Stop()
				{
					mode = MODE_IDLE;
					output = 0x10;
				}

				template<>
				void X24C0X<128>::Start()
				{
					mode = MODE_ADDRESS;
					latch.bit = 0;
					latch.address = 0;
					output = 0x10;
				}

				template<>
				void X24C0X<256>::Start()
				{
					mode = MODE_DATA;
					latch.bit = 0;
					output = 0x10;
				}

				template<>
				void X24C0X<128>::Rise(const uint bit)
				{
					NST_ASSERT( bit <= 1 );

					switch (mode)
					{
						case MODE_ADDRESS:

							if (latch.bit < 7)
							{
								latch.address &= ~(1U << latch.bit);
								latch.address |= bit << latch.bit++;
							}
							else if (latch.bit < 8)
							{
								latch.bit = 8;

								if (bit)
								{
									next = MODE_READ;
									latch.data = mem[latch.address];
								}
								else
								{
									next = MODE_WRITE;
								}
							}
							break;

						case MODE_ACK:

							output = 0x00;
							break;

						case MODE_READ:

							if (latch.bit < 8)
								output = (latch.data & 1U << latch.bit++) ? 0x10 : 0x00;

							break;

						case MODE_WRITE:

							if (latch.bit < 8)
							{
								latch.data &= ~(1U << latch.bit);
								latch.data |= bit << latch.bit++;
							}
							break;

						case MODE_ACK_WAIT:

							if (bit == 0)
								next = MODE_IDLE;

							break;
					}
				}

				template<>
				void X24C0X<128>::Fall()
				{
					switch (mode)
					{
						case MODE_ADDRESS:

							if (latch.bit == 8)
							{
								mode = MODE_ACK;
								output = 0x10;
							}
							break;

						case MODE_ACK:

							mode = next;
							latch.bit = 0;
							output = 0x10;
							break;

						case MODE_READ:

							if (latch.bit == 8)
							{
								mode = MODE_ACK_WAIT;
								latch.address = (latch.address+1) & 0x7F;
							}
							break;

						case MODE_WRITE:

							if (latch.bit == 8)
							{
								mode = MODE_ACK;
								next = MODE_IDLE;
								mem[latch.address] = latch.data;
								latch.address = (latch.address+1) & 0x7F;
							}
							break;
					}
				}

				template<>
				void X24C0X<256>::Rise(const uint bit)
				{
					NST_ASSERT( bit <= 1 );

					switch (mode)
					{
						case MODE_DATA:

							if (latch.bit < 8)
							{
								latch.data &= ~(1U << (7-latch.bit));
								latch.data |= bit << (7-latch.bit++);
							}
							break;

						case MODE_ADDRESS:

							if (latch.bit < 8)
							{
								latch.address &= ~(1U << (7-latch.bit));
								latch.address |= bit << (7-latch.bit++);
							}
							break;

						case MODE_READ:

							if (latch.bit < 8)
								output = (latch.data & 1U << (7-latch.bit++)) ? 0x10 : 0x00;

							break;

						case MODE_WRITE:

							if (latch.bit < 8)
							{
								latch.data &= ~(1U << (7-latch.bit));
								latch.data |= bit << (7-latch.bit++);
							}
							break;

						case MODE_NOT_ACK:

							output = 0x10;
							break;

						case MODE_ACK:

							output = 0x00;
							break;

						case MODE_ACK_WAIT:

							if (bit == 0)
							{
								next = MODE_READ;
								latch.data = mem[latch.address];
							}
							break;
					}
				}

				template<>
				void X24C0X<256>::Fall()
				{
					switch (mode)
					{
						case MODE_DATA:

							if (latch.bit == 8)
							{
								if ((latch.data & 0xA0) == 0xA0)
								{
									latch.bit = 0;
									mode = MODE_ACK;
									rw = latch.data & 0x01;
									output = 0x10;

									if (rw)
									{
										next = MODE_READ;
										latch.data = mem[latch.address];
									}
									else
									{
										next = MODE_ADDRESS;
									}
								}
								else
								{
									mode = MODE_NOT_ACK;
									next = MODE_IDLE;
									output = 0x10;
								}
							}
							break;

						case MODE_ADDRESS:

							if (latch.bit == 8)
							{
								latch.bit = 0;
								mode = MODE_ACK;
								next = (rw ? MODE_IDLE : MODE_WRITE);
								output = 0x10;
							}
							break;

						case MODE_READ:

							if (latch.bit == 8)
							{
								mode = MODE_ACK_WAIT;
								latch.address = (latch.address+1) & 0xFF;
							}
							break;

						case MODE_WRITE:

							if (latch.bit == 8)
							{
								latch.bit = 0;
								mode = MODE_ACK;
								next = MODE_WRITE;
								mem[latch.address] = latch.data;
								latch.address = (latch.address+1) & 0xFF;
							}
							break;

						case MODE_NOT_ACK:

							mode = MODE_IDLE;
							latch.bit = 0;
							output = 0x10;
							break;

						case MODE_ACK:
						case MODE_ACK_WAIT:

							mode = next;
							latch.bit = 0;
							output = 0x10;
							break;
					}
				}
			}
		}
	}
}
