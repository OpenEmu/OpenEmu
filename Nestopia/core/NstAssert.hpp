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

#ifndef NST_ASSERT_H
#define NST_ASSERT_H

#ifndef NST_CORE_H
#include "NstCore.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#ifndef NST_DEBUG

  #define NST_DEBUG_MSG(msg_) NST_NOP()
  #define NST_ASSERT_MSG(expr_,msg_) NST_ASSUME(expr_)
  #define NST_VERIFY_MSG(expr_,msg_) NST_NOP()

#elif defined(NST_WIN32)

  #if NST_GCC >= 200 || NST_MWERKS >= 0x3000
   #define NST_FUNC_NAME __PRETTY_FUNCTION__
  #elif NST_MSVC >= 1300
   #define NST_FUNC_NAME __FUNCTION__
  #elif NST_BCC >= 0x550
   #define NST_FUNC_NAME __FUNC__
  #else
   #define NST_FUNC_NAME 0
  #endif

  #if NST_MSVC >= 1300
   #define NST_HALT() __debugbreak()
  #elif NST_MSVC >= 1200 && defined(_M_IX86)
   #define NST_HALT() __asm {int 3} NST_NOP()
  #else
   #include <cstdlib>
   #define NST_HALT() std::abort()
  #endif

  namespace Nes
  {
      namespace Assertion
      {
          NST_NO_INLINE uint NST_CALL Issue
          (
              const char*,
              const char*,
              const char*,
              const char*,
              int
          );
      }
  }

  #define NST_DEBUG_MSG(msg_)                                                         \
  {                                                                                   \
      static bool ignore_ = false;                                                    \
                                                                                      \
      if (!ignore_)                                                                   \
      {                                                                               \
          switch (Nes::Assertion::Issue(0,msg_,__FILE__,NST_FUNC_NAME,__LINE__))      \
          {                                                                           \
              case 0: NST_HALT(); break;                                              \
              case 1: ignore_ = true; break;                                          \
          }                                                                           \
      }                                                                               \
  }                                                                                   \
  NST_NOP()

  #define NST_ASSERT_MSG(expr_,msg_)                                                  \
  {                                                                                   \
      static bool ignore_ = false;                                                    \
                                                                                      \
      if (!ignore_ && !(expr_))                                                       \
      {                                                                               \
          switch (Nes::Assertion::Issue(#expr_,msg_,__FILE__,NST_FUNC_NAME,__LINE__)) \
          {                                                                           \
              case 0: NST_HALT(); break;                                              \
              case 1: ignore_ = true; break;                                          \
          }                                                                           \
      }                                                                               \
  }                                                                                   \
  NST_NOP()

  #define NST_VERIFY_MSG(expr_,msg_) NST_ASSERT_MSG(expr_,msg_)

#else

  #include <cassert>

  #define NST_DEBUG_MSG(msg_) NST_NOP()
  #define NST_ASSERT_MSG(expr_,msg_) assert( !!(expr_) )
  #define NST_VERIFY_MSG(expr_,msg_) NST_NOP()

#endif

#define NST_ASSERT(expr_) NST_ASSERT_MSG(expr_,0)
#define NST_VERIFY(expr_) NST_VERIFY_MSG(expr_,0)

#endif
