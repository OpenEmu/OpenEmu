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

#ifndef NST_BOARD_BANDAI_24C0X_H
#define NST_BOARD_BANDAI_24C0X_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include <cstring>

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				template<uint N>
				class X24C0X;

				template<>
				class X24C0X<0>
				{
				public:

					void Stop();
					void Reset();

				protected:

					void LoadState(State::Loader&,byte*,uint);
					void SaveState(State::Saver&,dword,const byte*,uint) const;

					enum Mode
					{
						MODE_IDLE,
						MODE_DATA,
						MODE_ADDRESS,
						MODE_READ,
						MODE_WRITE,
						MODE_ACK,
						MODE_NOT_ACK,
						MODE_ACK_WAIT,
						MODE_MAX
					};

					struct
					{
						uint scl;
						uint sda;
					}   line;

					Mode mode;
					Mode next;

					struct
					{
						uint bit;
						uint address;
						uint data;
					}   latch;

					ibool rw;
					uint output;

				public:

					uint Read() const
					{
						return output;
					}
				};

				template<uint N>
				class X24C0X : public X24C0X<0>
				{
				public:

					enum
					{
						SIZE = N
					};

					void Set(uint,uint);

				private:

					void Start();
					void Rise(uint);
					void Fall();

					byte mem[SIZE];

				public:

					X24C0X()
					{
						std::memset( mem, 0, SIZE );
					}

					byte* GetData()
					{
						return mem;
					}

					const byte* GetData() const
					{
						return mem;
					}

					void LoadState(State::Loader& loader)
					{
						X24C0X<0>::LoadState( loader, mem, SIZE );
					}

					void SaveState(State::Saver& saver,dword baseChunk) const
					{
						X24C0X<0>::SaveState( saver, baseChunk, mem, SIZE );
					}

					void SetScl(uint scl)
					{
						Set( scl, line.sda );
					}

					void SetSda(uint sda)
					{
						Set( line.scl, sda );
					}
				};

				template<uint N>
				void X24C0X<N>::Set(const uint scl,const uint sda)
				{
					if (line.scl && sda < line.sda)
					{
						Start();
					}
					else if (line.scl && sda > line.sda)
					{
						Stop();
					}
					else if (scl > line.scl)
					{
						Rise( sda >> 6 );
					}
					else if (scl < line.scl)
					{
						Fall();
					}

					line.scl = scl;
					line.sda = sda;
				}

				typedef X24C0X<128> X24C01;
				typedef X24C0X<256> X24C02;
			}
		}
	}
}

#endif
