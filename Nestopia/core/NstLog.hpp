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

#ifndef NST_LOG_H
#define NST_LOG_H

#ifndef NST_CORE_H
#include "NstCore.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#ifndef NST_LINEBREAK
#ifdef NST_WIN32
#define NST_LINEBREAK "\r\n"
#else
#define NST_LINEBREAK "\n"
#endif
#endif

namespace Nes
{
	namespace Core
	{
		class Log
		{
		public:

			Log();
			~Log();

			struct Hex
			{
				const dword value;
				cstring const format;

				Hex(uint n,dword v)
				:
				value  (v),
				format (n == 8 ? "%02X" : n == 16 ? "%04X" : "%08X")
				{}
			};

			Log& operator << (char);
			Log& operator << (cstring);
			Log& operator << (const Hex&);
			Log& operator << (long);
			Log& operator << (ulong);

			static void Flush(cstring,dword);
			static bool Available();

		private:

			void Append(cstring,ulong);

			struct Object;
			Object* const object;

			static bool enabled;

		public:

			Log& operator << (schar  i) { return operator << ( long  (i) ); }
			Log& operator << (uchar  i) { return operator << ( ulong (i) ); }
			Log& operator << (short  i) { return operator << ( long  (i) ); }
			Log& operator << (ushort i) { return operator << ( ulong (i) ); }
			Log& operator << (int    i) { return operator << ( long  (i) ); }
			Log& operator << (uint   i) { return operator << ( ulong (i) ); }

			class Suppressor
			{
				const bool state;

			public:

				Suppressor()
				: state(enabled)
				{
					enabled = false;
				}

				~Suppressor()
				{
					enabled = state;
				}
			};

			template<dword N>
			static void Flush(const char (&c)[N])
			{
				NST_COMPILE_ASSERT( N > 0 );
				Flush( c, N-1 );
			}
		};
	}
}

#endif
