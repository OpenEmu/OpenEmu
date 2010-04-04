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

#ifndef NST_STATE_H
#define NST_STATE_H

#ifndef NST_VECTOR_H
#include "NstVector.hpp"
#endif

#include "NstStream.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace State
		{
			class Saver
			{
			public:

				Saver(StdStream,bool,bool,dword=0);
				~Saver();

				Saver& Begin(dword);
				Saver& Write8(uint);
				Saver& Write16(uint);
				Saver& Write32(dword);
				Saver& Write64(qword);
				Saver& Write(const byte*,dword);
				Saver& Compress(const byte*,dword);
				Saver& End();

			protected:

				Stream::Out stream;

			private:

				enum
				{
					CHUNK_RESERVE = 8
				};

				Vector<dword> chunks;
				const bool useCompression;
				const bool internal;

			public:

				template<dword N>
				Saver& Write(const byte (&data)[N])
				{
					NST_COMPILE_ASSERT( N > 0 );
					return Write( data, N );
				}

				template<dword N>
				Saver& Compress(const byte (&data)[N])
				{
					NST_COMPILE_ASSERT( N > 0 );
					return Compress( data, N );
				}

				bool Internal() const
				{
					return internal;
				}
			};

			class Loader
			{
			public:

				Loader(StdStream,bool);
				~Loader();

				dword Begin();
				dword Check();
				dword Length() const;
				uint  Read8();
				uint  Read16();
				dword Read32();
				qword Read64();
				void  Read(byte*,dword);
				void  Uncompress(byte*,dword);
				void  End();
				void  End(dword);

				template<uint N>
				class Data
				{
					struct Block
					{
						byte data[N];

						explicit Block(Loader& loader)
						{
							loader.Read( data, N );
						}
					};

					const Block block;

				public:

					explicit Data(Loader& loader)
					: block(loader) {}

					uint operator [] (uint i) const
					{
						NST_ASSERT( i < N );
						return block.data[i];
					}
				};

			protected:

				Stream::In stream;

			private:

				void CheckRead(dword);

				enum
				{
					CHUNK_RESERVE = 8
				};

				Vector<dword> chunks;
				const bool checkCrc;

			public:

				template<dword N>
				void Read(byte (&data)[N])
				{
					NST_COMPILE_ASSERT( N > 0 );
					Read( data, N );
				}

				template<dword N>
				void Uncompress(byte (&data)[N])
				{
					NST_COMPILE_ASSERT( N > 0 );
					Uncompress( data, N );
				}

				bool CheckCrc() const
				{
					return checkCrc;
				}
			};
		}
	}
}

#endif
