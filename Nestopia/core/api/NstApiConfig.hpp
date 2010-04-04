/*
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
*/

#ifndef NST_BASE_H
#error Do not include NstApConfig.hpp directly!
#endif

/*
////////////////////////////////////////////////////////////////////////////////////////
//
// Compiler Configuration:
// -----------------------
//
// NST_DEBUG                 - Debug mode compilation.
//
// NST_PRAGMA_ONCE           - Define if #pragma once is supported. Auto-defined if
//                             compiler is MCW or MSVC.
//
// NST_U64 <type>            - For native 64bit (or greater) integer support. May not
//                             be needed if compiler is BC, GCC, ICC, MSVC or supports
//                             the C99 long long integer type.
//
//                             Example: #define NST_U64 unsigned long long
//
//                             If no suitable type is available, a default class based
//                             implementation will be used which may generate slower code.
//
// NST_MM_INTRINSICS         - For MMX/SSE compiler intrinsics support through
//                             xmmintrin.h/emmintrin.h/mmintrin.h. Auto-defined if
//                             compiler is Win32 MSVC and _M_IX86 is defined.
//
// NST_CALL <attribute>      - Compiler/platform specific calling convention for non-member
//                             functions. Placed between return type and function name, e.g
//                             void NST_CALL DoSomething().
//
//                             Example usage on a Win32 platform: #define NST_CALL __fastcall
//
// NST_CALLBACK <attribute>  - Same as above but for user-callback functions only. If
//                             defined, you're responsible for decorating your own
//                             callback functions with it.
//
// NST_LINEBREAK <string>    - Character sequence for line breaks. Default is "\n", or "\r\n"
//                             if target platform is Win32.
//
//                             Example usage in Nestopia:
//
//                              Print( "Hey!" NST_LINEBREAK "get off my lawn!" NST_LINEBREAK );
//
// NST_NO_INLINE <attribute> - To prevent automatic inline expansion of certain functions
//                             that won't benefit from it. Auto-defined if compiler is
//                             GCC, ICC or MSVC.
//
// NST_RESTRICT <attribute>  - C99 restrict qualifier support. Used on pointer types to
//                             inform the compiler that they're are not aliased which may
//                             enable it to generate faster code. Auto-defined if compiler
//                             is GCC, ICC or MSVC.
//
// NST_ASSUME(x) y(x)        - Optimization hint for the compiler. Informs that the condition
//                             will always evaluate to true. Should not be used if the condition
//                             checking imposes a performance penalty as it's not intended for
//                             debugging use. Auto-defined if compiler is MSVC.
//
//                             Example usage in Nestopia:
//
//                             #ifdef NST_DEBUG
//                              assert( miss_july == hot );
//                             #else
//                              NST_ASSUME( miss_july == hot );
//                             #endif
//
// NST_FASTDELEGATE          - Define this if your compiler can handle casts between member
//                             function pointers of different types and calls through them.
//
//                             If the size required to store a non-virtual member function
//                             pointer is deemed too large, as in
//
//                             sizeof( void (Class::*)() ) > sizeof( void (*)() )
//
//                             this option is not worth using and Nestopia will force a
//                             compile time error. Auto-defined if compiler is MSVC.
//
// Abbrevations:
//
// BC - Borland C++
// GCC - GNU Compiler Collection
// ICC - Intel C/C++ Compiler
// MCW - Metrowerks CodeWarrior
// MSVC - Microsoft Visual C++
//
////////////////////////////////////////////////////////////////////////////////////////
*/

/*
////////////////////////////////////////////////////////////////////////////////////////
//
// Define to disable a particular feature.
//
// NST_NO_ZLIB    - ZLib compression library. Not recommended to disable
//                  unless it's unavailable on the target platform. The core
//                  uses it internally for save states and the rewinder
//                  feature. Although they'll still work without them, memory
//                  footprint will increase drastically.
//
// NST_NO_HQ2X    - hq2x and hq3x video filters
//
// NST_NO_SCALEX  - Scale2x and Scale3x video filters
//
// NST_NO_2XSAI   - 2xSaI video filter
//
////////////////////////////////////////////////////////////////////////////////////////
*/
