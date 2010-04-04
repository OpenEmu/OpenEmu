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

#ifndef NST_CORE_H
#define NST_CORE_H

#include <cstddef>

#ifndef NST_BASE_H
#include "NstBase.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#define NST_WIN32
#endif

#if NST_MSVC

 #pragma warning( disable : 4018 4100 4127 4244 4245 4308 4309 4310 4512 4800 4996 )

 #if NST_MSVC >= 800

  #ifdef NST_WIN32
  #define NST_FASTCALL __fastcall
  #define NST_REGCALL NST_FASTCALL
  #endif

  #if NST_MSVC >= 1200

   #ifndef NST_FASTDELEGATE
   #define NST_FASTDELEGATE
   #endif

   #ifndef _DEBUG
   #define NST_MSVC_OPTIMIZE
   #define NST_FORCE_INLINE __forceinline
   #pragma inline_depth( 255 )
   #pragma inline_recursion( on )
   #endif

   #define NST_SINGLE_CALL __forceinline

   #if NST_MSVC >= 1300

	#ifndef NST_NO_INLINE
	#define NST_NO_INLINE __declspec(noinline)
	#endif

	#ifndef NST_ASSUME
	#define NST_ASSUME(x_) __assume(x_)
	#endif

	#ifndef NST_DEBUG
	#define NST_UNREACHABLE() __assume(0)
	#endif

	#if !defined(NST_MM_INTRINSICS) && defined(NST_WIN32) && defined(_M_IX86)
	#define NST_MM_INTRINSICS
	#endif

	#define NST_NO_VTABLE __declspec(novtable)

	#if NST_MSVC >= 1400

     #ifndef NST_RESTRICT
     #define NST_RESTRICT __restrict
     #endif

     #pragma warning( default : 4191 4263 4287 4289 4296 4350 4545 4546 4547 4549 4555 4557 4686 4836 4905 4906 4928 4946 )

     #if 0
     #pragma warning( default : 4820 ) // byte padding on structs
     #pragma warning( default : 4710 ) // function not inlined
     #pragma warning( default : 4711 ) // function auto inlined
     #pragma warning( default : 4100 ) // unreferenced parameter
     #endif

	#endif

   #endif

  #endif

 #endif

#elif NST_GCC

 #if NST_GCC >= 291

  #ifndef NST_RESTRICT
  #define NST_RESTRICT __restrict__
  #endif

  #if NST_GCC >= 301

   #ifndef NST_NO_INLINE
   #define NST_NO_INLINE __attribute__((noinline))
   #endif

   #define NST_SINGLE_CALL __attribute__((always_inline))

   #if (NST_GCC >= 304) && defined(__i386__)
   #define NST_REGCALL __attribute__((regparm(2)))
   #endif

  #endif

 #endif

#endif

#if NST_ICC

 #pragma warning( disable : 11 69 304 373 383 444 810 869 981 1418 1572 1599 1786 )

 #if !defined(NST_RESTRICT) && NST_ICC >= 810
 #define NST_RESTRICT restrict
 #endif

#endif

#define NST_NOP() ((void)0)

#ifndef NST_FORCE_INLINE
#define NST_FORCE_INLINE inline
#endif

#ifndef NST_SINGLE_CALL
#define NST_SINGLE_CALL NST_FORCE_INLINE
#endif

#ifndef NST_NO_INLINE
#define NST_NO_INLINE
#endif

#ifndef NST_ASSUME
#define NST_ASSUME(x_) NST_NOP()
#endif

#ifndef NST_NO_VTABLE
#define NST_NO_VTABLE
#endif

#ifndef NST_RESTRICT
#define NST_RESTRICT
#endif

#ifndef NST_UNREACHABLE
#define NST_UNREACHABLE() NST_ASSERT(0)
#endif

#ifndef NST_FASTCALL
#define NST_FASTCALL
#endif

#ifndef NST_REGCALL
#define NST_REGCALL
#endif

#define NST_MIN(x_,y_) ((x_) < (y_) ? (x_) : (y_))
#define NST_MAX(x_,y_) ((x_) < (y_) ? (y_) : (x_))

#define NST_COMMA ,

#define NST_CAT_NEXT(x_,y_) x_##y_
#define NST_CAT(x_,y_) NST_CAT_NEXT(x_,y_)

#define NST_COMPILE_ASSERT(expr_) typedef char NST_CAT(Nestopia_assertion_at_line_,__LINE__)[(expr_) ? 1 : -1]

namespace Nes
{
	namespace Core
	{
		typedef const char* cstring;
		typedef const wchar_t* wcstring;
		typedef uint ibool;
		typedef uint Data;
		typedef uint Address;
		typedef dword Cycle;

		template<typename T,dword N>
		char(& array(T(&)[N]))[N];

		namespace Helper
		{
			template<ulong W> struct CountBits
			{
				enum { VALUE = 1 + CountBits<W / 2>::VALUE };
			};

			template<> struct CountBits<1UL>
			{
				enum { VALUE = 1 };
			};

			template<> struct CountBits<0UL>
			{
				enum { VALUE = 0 };
			};

			template<typename T,bool>
			struct ShiftSigned
			{
				static long Left(T v,uint c)
				{
					return v << c;
				}

				static long Right(T v,uint c)
				{
					return v >> c;
				}
			};

			template<typename T>
			struct ShiftSigned<T,false>
			{
				static long Left(T v,uint c)
				{
					return (v >= 0) ? +long(ulong(v) << c) : -long(ulong(-v) << c);
				}

				static long Right(T v,uint c)
				{
					return (v >= 0) ? +long(ulong(v) >> c) : -long(ulong(-v) >> c);
				}
			};

			template<typename T,bool>
			struct SignExtend8
			{
				static T Convert(T v)
				{
					return schar(v);
				}
			};

			template<typename T>
			struct SignExtend8<T,false>
			{
				static T Convert(T v)
				{
					return (v & 1U << 7) ? (v | ~0UL << 7) : v;
				}
			};
		}

		template<ulong V> struct ValueBits
		{
			enum { VALUE = Helper::CountBits<V>::VALUE };
		};

		template<typename T>
		inline long signed_shl(T v,uint c)
		{
			enum {NATIVE = T(-7) << 1 == -14};
			return Helper::ShiftSigned<T,NATIVE>::Left( v, c );
		}

		template<typename T>
		inline long signed_shr(T v,uint c)
		{
			enum {NATIVE = T(-7) >> 1 == -4 || T(-7) >> 1 == -3};
			return Helper::ShiftSigned<T,NATIVE>::Right( v, c );
		}

		template<typename T>
		inline T sign_extend_8(T v)
		{
			enum {NATIVE = CHAR_BIT == 8 && UCHAR_MAX == 0xFF && SCHAR_MIN == -128 && SCHAR_MAX == 127 && T(-2) == T(~1UL)};
			return Helper::SignExtend8<T,NATIVE>::Convert( v );
		}

		template<idword Min,idword Max>
		inline idword Clamp(idword value)
		{
			return (value <= Max) ? (value >= Min) ? value : Min : Max;
		}

		enum
		{
			SIZE_1K     = 0x400,
			SIZE_2K     = 0x800,
			SIZE_4K     = 0x1000,
			SIZE_5K     = 0x1400,
			SIZE_6K     = 0x1800,
			SIZE_8K     = 0x2000,
			SIZE_16K    = 0x4000,
			SIZE_32K    = 0x8000,
			SIZE_40K    = 0xA000,
			SIZE_64K    = 0x10000,
			SIZE_128K   = 0x20000,
			SIZE_256K   = 0x40000,
			SIZE_512K   = 0x80000,
			SIZE_1024K  = 0x100000,
			SIZE_2048K  = 0x200000,
			SIZE_3072K  = 0x300000,
			SIZE_4096K  = 0x400000,
			SIZE_8192K  = 0x800000,
			SIZE_16384K = 0x1000000
		};

		template<char T>
		struct Ascii
		{
			enum
			{
				V = ( T >= 'a' && T <= 'z') ? T - 'a' + 0x61 :
					( T >= 'A' && T <= 'Z') ? T - 'A' + 0x41 :
					( T >= '0' && T <= '9') ? T - '0' + 0x30 :
					( T == '\0' ) ? 0x00 :
					( T == ' '  ) ? 0x20 :
					( T == '!'  ) ? 0x21 :
					( T == '#'  ) ? 0x23 :
					( T == '%'  ) ? 0x25 :
					( T == '^'  ) ? 0x5E :
					( T == '&'  ) ? 0x26 :
					( T == '*'  ) ? 0x2A :
					( T == '('  ) ? 0x28 :
					( T == ')'  ) ? 0x29 :
					( T == '-'  ) ? 0x2D :
					( T == '_'  ) ? 0x5F :
					( T == '+'  ) ? 0x2B :
					( T == '='  ) ? 0x3D :
					( T == '~'  ) ? 0x7E :
					( T == '['  ) ? 0x5B :
					( T == ']'  ) ? 0x5D :
					( T == '\\' ) ? 0x5C :
					( T == '|'  ) ? 0x7C :
					( T == ';'  ) ? 0x3B :
					( T == ':'  ) ? 0x3A :
					( T == '\'' ) ? 0x27 :
					( T == '\"' ) ? 0x22 :
					( T == '{'  ) ? 0x7B :
					( T == '}'  ) ? 0x7D :
					( T == ','  ) ? 0x2C :
					( T == '.'  ) ? 0x2E :
					( T == '<'  ) ? 0x3C :
					( T == '>'  ) ? 0x3E :
					( T == '/'  ) ? 0x2F :
					( T == '?'  ) ? 0x3F :
					( T == '\a' ) ? 0x07 :
					( T == '\b' ) ? 0x08 :
					( T == '\t' ) ? 0x09 :
					( T == '\v' ) ? 0x0B :
					( T == '\n' ) ? 0x0A :
					( T == '\r' ) ? 0x0D :
					( T == '\f' ) ? 0x0C : 0xFF
			};

			NST_COMPILE_ASSERT( V != 0xFF );
		};

		template<char A,char B,char C=0,char D=0>
		struct AsciiId
		{
			enum
			{
				V =
				(
					dword( Ascii<A>::V ) <<  0 |
					dword( Ascii<B>::V ) <<  8 |
					dword( Ascii<C>::V ) << 16 |
					dword( Ascii<D>::V ) << 24
				)
			};

			static dword R(byte a,byte b=0,byte c=0,byte d=0)
			{
				return
				(
					dword( Ascii<A>::V + a ) <<  0 |
					dword( Ascii<B>::V + b ) <<  8 |
					dword( Ascii<C>::V + c ) << 16 |
					dword( Ascii<D>::V + d ) << 24
				);
			}
		};

		template<typename T,typename U>
		int StringCompare(const T* a,const U* b)
		{
			do
			{
				const wchar_t v[] =
				{
					(*a < L'a' || *a > L'z') ? *a : (L'A' + (*a - L'a')),
					(*b < L'a' || *b > L'z') ? *b : (L'A' + (*b - L'a'))
				};

				if (v[0] < v[1])
					return -1;

				if (v[0] > v[1])
					return +1;
			}
			while (++b, *a++);

			return 0;
		}

		template<typename T,typename U>
		int StringCompare(const T* a,const U* b,uint l)
		{
			for (; l--; ++a, ++b)
			{
				const wchar_t v[] =
				{
					(*a < L'a' || *a > L'z') ? *a : (L'A' + (*a - L'a')),
					(*b < L'a' || *b > L'z') ? *b : (L'A' + (*b - L'a'))
				};

				if (v[0] < v[1])
					return -1;

				if (v[0] > v[1])
					return +1;

				if (!v[0])
					return 0;
			}

			return 0;
		}
	}

#ifdef NST_U64

	typedef NST_U64 qword;
	#define NST_NATIVE_QWORD

#elif (ULONG_MAX > 0xFFFFFFFF) && (ULONG_MAX / 0xFFFFFFFF - 1 > 0xFFFFFFFF)

	typedef unsigned long qword;
	#define NST_NATIVE_QWORD

#elif (defined(ULLONG_MAX) && (ULLONG_MAX > 0xFFFFFFFF) && (ULLONG_MAX / 0xFFFFFFFF - 1 > 0xFFFFFFFF)) || (NST_GCC >= 300)

	#if NST_GCC
	__extension__ typedef unsigned long long qword;
	#else
	typedef unsigned long long qword;
	#endif
	#define NST_NATIVE_QWORD

#elif defined(_UI64_MAX) && (NST_MSVC >= 900 || NST_BCB >= 0x530)

	typedef unsigned __int64 qword;
	#define NST_NATIVE_QWORD

#else

	class qword
	{
		void Multiply(qword);
		static void Divide(qword&,const qword,bool);
		void Shl(uint);
		void Shr(uint);

		enum
		{
			LO_MASK = 0xFFFFFFFF,
			LO_MSB  = 0x80000000
		};

		dword lo;
		dword hi;

	public:

		qword() {}

		qword(dword v)
		: lo(v), hi(0) {}

		qword(dword msdw,dword lsdw)
		: lo(lsdw), hi(msdw) {}

		qword(const qword& v)
		: lo(v.lo), hi(v.hi) {}

		template<typename V>
		qword& operator = (const V& v)
		{
			lo = v;
			hi = 0;
			return *this;
		}

		qword& operator = (const qword& v)
		{
			lo = v.lo;
			hi = v.hi;
			return *this;
		}

		template<typename V>
		qword& operator += (const V& v)
		{
			dword t = lo;
			lo = (lo + v) & LO_MASK;
			hi = (hi + (t > lo)) & LO_MASK;
			return *this;
		}

		template<typename V>
		qword& operator -= (const V& v)
		{
			dword t = lo;
			lo = (lo - v) & LO_MASK;
			hi = (hi - (t < lo)) & LO_MASK;
			return *this;
		}

		qword operator ++ (int)
		{
			qword t;
			t.lo = lo;
			lo = (lo + 1) & LO_MASK;
			t.hi = hi;
			hi = (hi + (t.lo > lo)) & LO_MASK;
			return t;
		}

		qword& operator ++ ()
		{
			dword t = lo;
			lo = (lo + 1) & LO_MASK;
			hi = (hi + (t > lo)) & LO_MASK;
			return *this;
		}

		qword operator -- (int)
		{
			qword t;
			t.lo = lo;
			lo = (lo - 1) & LO_MASK;
			t.hi = hi;
			hi = (hi - (t.lo < lo)) & LO_MASK;
			return t;
		}

		qword& operator -- ()
		{
			dword t = lo;
			lo = (lo - 1) & LO_MASK;
			hi = (hi - (t < lo)) & LO_MASK;
			return *this;
		}

		template<typename V>
		qword& operator *= (const V& v)
		{
			if (!(((lo | v) & 0xFFFF0000) | hi))
				lo = (lo * v) & LO_MASK;
			else
				Multiply( qword(v) );

			return *this;
		}

		template<typename V>
		qword& operator /= (const V& v)
		{
			if (!hi)
				lo /= v;
			else
				Divide( *this, qword(v), false );

			return *this;
		}

		template<typename V>
		qword& operator %= (const V& v)
		{
			if (!hi)
				lo %= v;
			else
				Divide( *this, qword(v), true );

			return *this;
		}

		template<typename V> qword operator + (const V& v) const { return qword(*this) += v; }
		template<typename V> qword operator - (const V& v) const { return qword(*this) -= v; }
		template<typename V> qword operator * (const V& v) const { return qword(*this) *= v; }
		template<typename V> qword operator / (const V& v) const { return qword(*this) /= v; }
		template<typename V> qword operator % (const V& v) const { return qword(*this) %= v; }

		template<typename V> qword& operator |= (const V& v) { lo |= v;         return *this; }
		template<typename V> qword& operator &= (const V& v) { lo &= v; hi = 0; return *this; }
		template<typename V> qword& operator ^= (const V& v) { lo ^= v;         return *this; }

		template<typename V> qword operator | (const V& v) const { return qword( hi, lo | v ); }
		template<typename V> qword operator & (const V& v) const { return qword(     lo & v ); }
		template<typename V> qword operator ^ (const V& v) const { return qword( hi, lo ^ v ); }

		template<typename V> qword& operator >>= (const V& v) { Shr(v); return *this; }
		template<typename V> qword& operator <<= (const V& v) { Shl(v); return *this; }

		template<typename V> qword operator >> (const V& v) const { return qword(*this) >>= v; }
		template<typename V> qword operator << (const V& v) const { return qword(*this) <<= v; }

		qword operator ~() const
		{
			return qword( hi ^ LO_MASK, lo ^ LO_MASK );
		}

		template<typename V>
		bool operator == (const V& v) const
		{
			return !((lo - v) | hi);
		}

		template<typename V>
		bool operator < (const V& v) const
		{
			return (lo < v && !hi);
		}

		template<typename V>
		bool operator <= (const V& v) const
		{
			return (lo <= v && !hi);
		}

		template<typename V>
		bool operator != (const V& v) const
		{
			return !(*this == v);
		}

		template<typename V>
		bool operator > (const V& v) const
		{
			return !(*this <= v);
		}

		template<typename V>
		bool operator >= (const V& v) const
		{
			return !(*this < v);
		}

		bool operator !() const
		{
			return !(lo|hi);
		}

		operator bool() const
		{
			return (lo|hi);
		}

		operator int    () const { return lo; }
		operator uint   () const { return lo; }
		operator char   () const { return lo; }
		operator schar  () const { return lo; }
		operator uchar  () const { return lo; }
		operator short  () const { return lo; }
		operator ushort () const { return lo; }
		operator long   () const { return lo; }
		operator ulong  () const { return lo; }
	};

	template<>
	inline qword& qword::operator += (const qword& v)
	{
		dword t = lo;
		lo = (lo + v.lo) & LO_MASK;
		hi = (hi + (t > lo) + v.hi) & LO_MASK;
		return *this;
	}

	template<>
	inline qword& qword::operator -= (const qword& v)
	{
		dword t = lo;
		lo = (lo - v.lo) & LO_MASK;
		hi = (hi - ((t < lo) + v.hi)) & LO_MASK;
		return *this;
	}

	template<>
	inline qword& qword::operator *= (const qword& v)
	{
		Multiply( v );
		return *this;
	}

	template<>
	inline qword& qword::operator /= (const qword& v)
	{
		if (hi | v.hi)
			Divide( *this, v, false );
		else
			lo /= v.lo;

		return *this;
	}

	template<>
	inline qword& qword::operator %= (const qword& v)
	{
		Divide( *this, v, true );
		return *this;
	}

	template<> inline qword& qword::operator |= (const qword& v) { lo |= v.lo; hi |= v.hi; return *this; }
	template<> inline qword& qword::operator &= (const qword& v) { lo &= v.lo; hi &= v.hi; return *this; }
	template<> inline qword& qword::operator ^= (const qword& v) { lo ^= v.lo; hi ^= v.hi; return *this; }

	template<> inline qword qword::operator | (const qword& v) const { return qword( hi | v.hi, lo | v.lo ); }
	template<> inline qword qword::operator & (const qword& v) const { return qword( hi & v.hi, lo & v.lo ); }
	template<> inline qword qword::operator ^ (const qword& v) const { return qword( hi ^ v.hi, lo ^ v.lo ); }

	template<>
	inline bool qword::operator == (const qword& v) const
	{
		return !((lo - v.lo) | (hi - v.hi));
	}

	template<>
	inline bool qword::operator < (const qword& v) const
	{
		return (hi < v.hi) || (lo < v.lo && hi == v.hi);
	}

	template<>
	inline bool qword::operator <= (const qword& v) const
	{
		return (hi < v.hi) || (hi == v.hi ? (lo <= v.lo) : false);
	}

#endif
}

#endif
