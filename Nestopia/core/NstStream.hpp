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

#ifndef NST_STREAM_H
#define NST_STREAM_H

#ifndef NST_ASSERT_H
#include "NstAssert.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		template<typename T>
		class Vector;

		typedef void* StdStream;

		namespace Stream
		{
			class In
			{
				StdStream const stream;

				void SafeRead(byte*,dword);
				void Clear();

			public:

				explicit In(StdStream s)
				: stream(s)
				{
					NST_ASSERT( stream );
				}

				static dword AsciiToC(char* NST_RESTRICT,const byte* NST_RESTRICT,dword);

				void  Read(byte*,dword);
				void  Read(char*,dword);
				dword Read(Vector<char>&);
				uint  Read8();
				uint  Read16();
				dword Read32();
				qword Read64();
				uint  SafeRead8();
				void  Peek(byte*,dword);
				uint  Peek8();
				uint  Peek16();
				dword Peek32();
				void  Seek(idword);
				ulong Length();
				bool  Eof();

				template<dword N>
				void Read(byte (&data)[N])
				{
					NST_COMPILE_ASSERT( N > 0 );
					Read( data, N );
				}

				bool operator == (StdStream s) const
				{
					return stream == s;
				}

				bool operator != (StdStream s) const
				{
					return stream != s;
				}
			};

			class Out
			{
				StdStream const stream;

				void Clear();

			public:

				explicit Out(StdStream s)
				: stream(s)
				{
					NST_ASSERT( stream );
				}

				void Write(const byte*,dword);
				void Write8(uint);
				void Write16(uint);
				void Write32(dword);
				void Write64(qword);
				void Seek(idword);
				bool SeekEnd();

				template<dword N>
				void Write(const byte (&data)[N])
				{
					NST_COMPILE_ASSERT( N > 0 );
					Write( data, N );
				}

				bool operator == (StdStream s) const
				{
					return stream == s;
				}

				bool operator != (StdStream s) const
				{
					return stream != s;
				}
			};
		}
	}
}

#endif
