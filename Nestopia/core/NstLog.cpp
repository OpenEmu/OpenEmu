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

#include <cstdio>
#include <new>
#include <string>
#include "NstAssert.hpp"
#include "NstLog.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		struct Log::Object
		{
			std::string string;
		};

		bool Log::enabled = true;

		Log::Log()
		: object( !Api::User::logCallback ? NULL : new (std::nothrow) Object )
		{
		}

		Log::~Log()
		{
			if (object)
			{
				if (enabled)
					Api::User::logCallback( object->string.c_str(), object->string.size() );

				delete object;
			}
		}

		bool Log::Available()
		{
			return Api::User::logCallback;
		}

		void Log::Append(cstring c,ulong n)
		{
			object->string.append( c, n );
		}

		Log& Log::operator << (long value)
		{
			if (enabled && object)
			{
				char buffer[24];

				const int length = std::sprintf( buffer, "%li", value );
				NST_VERIFY( length > 0 );

				if (length > 0)
					Append( buffer, length );
			}

			return *this;
		}

		Log& Log::operator << (ulong value)
		{
			if (enabled && object)
			{
				char buffer[24];

				const int length = std::sprintf( buffer, "%lu", value );
				NST_VERIFY( length > 0 );

				if (length > 0)
					Append( buffer, length );
			}

			return *this;
		}

		Log& Log::operator << (cstring c)
		{
			if (enabled && object)
				object->string.append( c );

			return *this;
		}

		Log& Log::operator << (char c)
		{
			if (enabled && object)
				object->string.append( 1, c );

			return *this;
		}

		Log& Log::operator << (const Hex& hex)
		{
			if (enabled && object)
			{
				char buffer[16];

				buffer[0] = '0';
				buffer[1] = 'x';

				const int length = std::sprintf( buffer + 2, hex.format, hex.value );
				NST_VERIFY( length > 0 );

				if (length > 0)
					Append( buffer, 2 + length );
			}

			return *this;
		}

		void Log::Flush(cstring string,dword length)
		{
			if (enabled)
				Api::User::logCallback( string, length );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
