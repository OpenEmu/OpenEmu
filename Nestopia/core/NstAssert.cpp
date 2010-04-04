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

#include "NstCore.hpp"

#if defined(NST_DEBUG) && defined(NST_WIN32)

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef _UNICODE
 #define NST_MESSAGEBOX MessageBoxA
#else
 #define NST_MESSAGEBOX MessageBox
#endif

#if NST_MSVC >= 1200
 #ifdef _UNICODE
  #define NST_DEBUGSTRING(s_) OutputDebugStringA( s_ );
 #else
  #define NST_DEBUGSTRING(s_) OutputDebugString( s_ );
 #endif
#else
 #define NST_DEBUGSTRING(s_)
#endif

namespace Nes
{
	namespace Assertion
	{
		NST_NO_INLINE uint NST_CALL Issue
		(
			const char* expression,
			const char* msg,
			const char* file,
			const char* function,
			int line
		)
		{
			const std::size_t length =
			(
				(msg ?        std::strlen(msg)        :  0) +
				(expression ? std::strlen(expression) : 16) +
				(file ?       std::strlen(file)       : 16) +
				(function ?   std::strlen(function)   : 16) +
				64 + 1
			);

			if (char* const buffer = new (std::nothrow) char [length])
			{
				std::sprintf
				(
					buffer,
					msg ? "%s, Expression: %s\n\n File: %s\n Function: %s\n Line: %i\n\n" :
                          "%sExpression: %s\n\n File: %s\n Function: %s\n Line: %i\n\n",
					msg ? msg : "",
					expression ? expression : "break point",
					file,
					function ? function : "unknown",
					line
				);

				NST_DEBUGSTRING( buffer );

				int result = NST_MESSAGEBOX
				(
					::GetActiveWindow(),
					buffer,
					"Nestopia Debug Assertion!",
					MB_ABORTRETRYIGNORE|MB_SETFOREGROUND|MB_TOPMOST
				);

				delete [] buffer;

				if (result != IDABORT)
					return result == IDIGNORE ? 1 : 2;

				result = NST_MESSAGEBOX
				(
					::GetActiveWindow(),
					"break into the debugger?",
					"Nestopia Debug Assertion!",
					MB_YESNO|MB_SETFOREGROUND|MB_TOPMOST
				);

				if (result == IDNO)
					::FatalExit( EXIT_FAILURE );
			}
			else
			{
				NST_MESSAGEBOX
				(
					::GetActiveWindow(),
					"Out of memory!",
					"Nestopia Debug Assertion!",
					MB_OK|MB_ICONERROR|MB_SETFOREGROUND|MB_TOPMOST
				);

				::FatalExit( EXIT_FAILURE );
			}

			return 0;
		}
	}
}

#endif
