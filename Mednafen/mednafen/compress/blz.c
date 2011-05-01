/* Configuration macros. If undefined, it is automatically configured, otherwise its
value (0 or 1) determines the setting. */

/* Enable/disable use of negative/positive sign for faster high bit testing */
/*#define BLZ_USE_SIGN 1 */

/* Enable/disable fast version that makes unaligned memory accesses. Fast version
uses all the following flags. Portable version only uses BLZ_USE_SIGN flag. */
/*#define BLZ_FAST 1 */

/* Enable/disable use count leading zeros operation */
/*#define BLZ_FAST_CLZ 1 */

/* Big/little endian override. Only needs to be set if assertion in blz_pack() fails.
Can happen on processors which support both byte orders and are using the less-common
one, like little-endian PowerPC or big-endian ARM. */
/*#define BLZ_BIG_ENDIAN 1 */

/* Restricted pointer keyword. Auto configured for GCC and Microsoft, others might
use plain restrict. */
/*#define BLZ_RESTRICT restrict */

/* blz 0.9.1. http://www.slack.net/~ant/ */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include "blz.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>

/* Copyright (C) 2007 Shay Green. This module is free software; you can
redistribute it and/or modify it under the terms of the GNU General Public
License, version 2 or later, as published by the Free Software Foundation.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
details. You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

/* Comments with percentages are how often the condition is true */

/* Data format: [32-bit length] [block 1] [block 2] ... [32-bit data 0] [16-bit data 0]

block: [32 bits] [symbol 0] [symbol 1] ... [symbol 30]

Bits tell whether a symbol is a literal byte (1) or match (0), starting at bit 0.
Highest bit set is an end marker, so a block can have at most 31 symbols.
Matches are encoded in three ways.

Short match [3-bit length | 13-bit offset]
Mid match   [5-bit length] [16-bit offset]
Long match  [8-bit data 0] [32-bit length] [32-bit offset] */

typedef unsigned char byte;

#if INT_MAX < 0x7FFFFFFF || UINT_MAX < 0xFFFFFFFF
	#error "int must be at least 32 bits"
#endif

/* Automatically optimize for known processors. Thanks http://predef.sourceforge.net/ */

#define BLZ_PPC (defined (__POWERPC__) || defined (__ppc__) || defined (_M_PPC) || defined (_POWER))

#if BLZ_PPC || defined (__m68k__)
	#ifndef BLZ_BIG_ENDIAN
		#define BLZ_BIG_ENDIAN 1
	#endif
#endif

#if BLZ_PPC || defined (__m68k__) || defined (__i386__) || defined (__x86_64__) || \
		defined (_M_IX86) || defined (_M_X64)
	#ifndef BLZ_FAST
		#define BLZ_FAST 1
	#endif
	#ifndef BLZ_FAST_CLZ
		#define BLZ_FAST_CLZ 1
	#endif
#endif

#ifndef BLZ_RESTRICT
	#if __GNUC__ >= 3 || _MSC_VER
		#define BLZ_RESTRICT __restrict
	#else
		#define BLZ_RESTRICT
	#endif
#endif

#if INT_MAX != 0x7FFFFFFF || !BLZ_USE_SIGN
	typedef unsigned bits_t;
	#define TOP_BIT( n )    ((n) & 0x80000000u)
	#define ALL_BITS( n )   ((n) & 0xFFFFFFFFu)
#else
	typedef int bits_t;
	#define TOP_BIT( n )    ((n) < 0)
	#define ALL_BITS( n )   (n)
#endif



/**** Fast version ****/

#if BLZ_FAST

char const blz_version [] = "fast";

#if defined (HAVE_STDINT_H)
	#include <stdint.h>
#elif defined (HAVE_INTTYPES_H)
	#include <inttypes.h>
#else
	#if USHRT_MAX != 0xFFFF || UINT_MAX != 0xFFFFFFFF
		#error "Short must be 16 bits and int must be 32 bits"
	#endif
	/* macros instead of typedefs in case std headers have these typedefs */
	#define uint16_t unsigned short
	#define uint32_t unsigned int
	#define  int32_t signed   int
#endif

#ifndef GET16
	#define GET16( addr )       (*(uint16_t const*) (addr))
	#define GET32( addr )       (*(uint32_t const*) (addr))
	#define GET32S( addr )      (*( int32_t const*) (addr))
	#define SET16( addr, in )   (void) (*(uint16_t*) (addr) = (uint16_t) (in))
	#define SET32( addr, in )   (void) (*(uint32_t*) (addr) = (uint32_t) (in))
#endif

#if BLZ_BIG_ENDIAN
	#define BIG_LITTLE( big, little ) (big)
#else
	#define BIG_LITTLE( big, little ) (little)
#endif

#if BLZ_FAST_CLZ
	/* CLZ: count leading zeros, CTZ: count trailing zeros */
	#if __GNUC__ >= 4
		#define CLZ __builtin_clz
		#define CTZ __builtin_ctz
	#elif BLZ_PPC && (__GNUC__ >= 3 || __MWERKS__)
		#if __GNUC__
			#include <ppc_intrinsics.h>
		#endif
		#define CLZ __cntlzw
		#define CTZ( n ) (32 - __cntlzw( ((n) - 1) & ~(n) ))
	#endif
	
	#ifdef CLZ
		#define COUNT_MATCH( n ) (BIG_LITTLE( CLZ( n ), CTZ( n ) ) >> 3)
	#endif
#endif

#define CALC_HASH( n ) BIG_LITTLE( (n >> 20) ^ (n >> 8 & 0xFFF), ((n >> 12) ^ n) & 0xFFF )

#if BLZ_BIG_ENDIAN && BLZ_PPC
	/* optimizes out two instructions (unless compiler is really smart) */
	typedef int dict_entry_t;
	#define DICT_SHIFT( x, y ) \
		(((x) / ((1u << (y)) / sizeof (byte*))) & (0xFFF * sizeof (byte*)))
	#define DICT_ENTRY( d, n )  (DICT_SHIFT( n, 20 ) ^ DICT_SHIFT( n, 8 ))
	#define USE_ENTRY( d, o )   *(byte const**) ((byte*) d + o)
#else
	typedef byte const** dict_entry_t;
	#define DICT_ENTRY( d, n )  &d [CALC_HASH( n )]
	#define USE_ENTRY( d, o )   *o
#endif

/* Pack */

#define PRE_GOTO() \
	data = GET32( in )

#define GOTO_NEXT \
	if ( !TOP_BIT( bits ) ) /* 97% */ \
		goto match;\
	goto flush;

enum { in_max_offset = 11 }; /* 8+ byte match just before end will read this far ahead */

static int blz_pack_( byte const* const in_min, int const size,
		void* const out_min, blz_pack_t* const dict )
{
	/* On 32-bit x86, only dict, in_max, out_max, and bits_out don't fit in registers */
	byte const* BLZ_RESTRICT in     = in_min;
	byte const* const        in_max = in + size - in_max_offset;
	
	byte*       out      = (byte*) out_min;
	byte* const out_max  = out + size;
	byte*       out_bits;
	
	bits_t bits = 2;
	unsigned data;
	
	in += 4;
	SET32( out, size );
	out_bits = (out += 8);
	
literal_sub_4: /* Literal byte */
	{
		int const t = in [-4];
		in -= 3;
		PRE_GOTO();
		*out++ = t;
		if ( TOP_BIT( ++bits ) ) /* 3% */
			goto flush;
	}
	
match: /* Check for match and update dict */
	{
		/* calc dict entry and check for end of input */
		dict_entry_t const dict_offset = DICT_ENTRY( dict->dict, data );
		byte const* const BLZ_RESTRICT match = USE_ENTRY( dict->dict, dict_offset );
		bits <<= 1;
		if ( in >= in_max ) /* 1% */
			goto final_bytes;
		
		/* read first 4 bytes of match and check match distance */
		{
			unsigned const match_first = GET32( match - 4 );
			if ( match > in ) /* 1% */
				goto literal;
			
			/* compare first four bytes and update dict */
			USE_ENTRY( dict->dict, dict_offset ) = (in += 4);
			if ( (data ^= match_first) != 0 ) /* 74% */
			{
				int off = in - match;
				if ( BIG_LITTLE( data >> 8, data & 0x00FFFFFF ) ) /* 57% */
					goto literal_sub_4;
				if ( off >= (1 << 13) ) /* 19% */
					goto literal_sub_4;
				
				/* 3 byte match */
				--in;
				PRE_GOTO();
				SET16( out, BIG_LITTLE( (1 << 13) | off, (off << 3) + 1 ) );
				out += 2;
				GOTO_NEXT
			}
		}
		
		if ( (data = GET32( in ) ^ GET32( match )) != 0 ) /* 78% */
		{
			/* 4-7 byte match */
			int off = in - match;
			int len;
		#ifdef COUNT_MATCH
			len  = COUNT_MATCH( data );
			in  += len;
			len += 2;
		#else
			len = 2;
			if ( !(data & BIG_LITTLE( 0xFF000000, 0x000000FF )) ) /* 48% 5-7 byte match */
			{
				++in;
				len = 3;
				if ( !(data & BIG_LITTLE( 0x00FF0000, 0x0000FFFF )) ) /* 47% 6-7 byte match */
				{
					++in;
					len = 4;
					if ( !BIG_LITTLE( data >> 8, data & 0x00FF0000 ) ) /* 37% 7 byte match */
					{
						++in;
						len = 5;
					}
				}
			}
		#endif
			PRE_GOTO(); /* TODO: could avoid this in 4-byte case since it's already read */
			
			/* Encode offset and length */
			if ( off < (1 << 13) ) /* 82% */
			{
				/* LLLOOOOO OOOOOOOO */
				SET16( out, BIG_LITTLE( (len << 13) | off, (off << 3) + len ) );
				out += 2;
				GOTO_NEXT
			}
			
			/* unsigned cast and <= helps PowerPC */
			if ( (unsigned) off <= (1 << 16) - 1 ) /* 91% */
			{
				/* 000LLLLL OOOOOOOO OOOOOOOO */
				*out = BIG_LITTLE( len, len << 3 );
				SET16( out + 1, off );
				out += 3;
				GOTO_NEXT
			}
			
			in -= 2;
			in -= len;
			goto literal;
		}
		{
			/* 8+ byte match */
			byte const* const start = in + 2;
			int off = match - in;
			in += 4;
			{
				/* compare 4 bytes at a time */
				int temp;
				do
				{
					data = GET32( in );
					temp = GET32( in + off );
					if ( in >= in_max ) /* 1% */
						goto hit_end;
					in += 4;
				}
				while ( !(data ^= temp) ); /* 73% */
				
				/* determine which of the 4 bytes matched */
			#ifdef COUNT_MATCH
				in += COUNT_MATCH( data );
			#else
				if ( !(data & BIG_LITTLE( 0xFF000000, 0x000000FF )) ) /* 64% */
				{
					++in;
					if ( !(data & BIG_LITTLE( 0x00FF0000, 0x0000FFFF )) ) /* 58% */
					{
						++in;
						if ( !BIG_LITTLE( data >> 8, data & 0x00FF0000 ) ) /* 45% */
							++in;
					}
				}
			#endif
			}
		hit_end:
		
			/* Encode offset and length */
			{
				int len = in - start;
				in -= 4;
				off = -off;
				PRE_GOTO();
				
				if ( len < (1 << 5) ) /* 90% */
				{
					/* unsigned cast and <= helps PowerPC */
					if ( (unsigned) off <= (1 << 16) - 1 ) /* 98% */
					{
						/* 000LLLLL OOOOOOOO OOOOOOOO */
						*out = BIG_LITTLE( len, len << 3 );
						SET16( out + 1, off );
						out += 3;
						GOTO_NEXT
					}
					
					if ( len <= 9 - 2 ) /* 52% */
					{
						/* TODO: lame */
						in -= 2;
						in -= len;
						goto literal;
					}
				}
				
				/* 00000000 OOOOOOOO OOOOOOOO OOOOOOOO OOOOOOOO LLLLLLLL LLLLLLLL LLLLLLLL LLLLLLLL */
				*out = 0;
				SET32( out + 1, off );
				SET32( out + 5, len );
				out += 9;
				GOTO_NEXT
			}
		}
	}

	/* These two are at bottom of loop to make branches here positive and branches back
	negative, which helps CPUs that lack dynamic branch prediction */
	
literal: /* Literal byte */
	{
		int const t = *in++;
		PRE_GOTO();
		*out++ = t;
		if ( !TOP_BIT( ++bits ) ) /* 97% */
			goto match;
	}
	
flush: /* Flush bits */
	out += 4;
	SET32( out_bits - 4, (bits << 1) + 1 );
	out_bits = out;
	bits = 1;
	if ( out < out_max ) /* 99% */
		goto match;
	return -1; /* no compression */

final_bytes:
	/* Right-justify and flush final bits */
#ifdef CLZ
	SET32( out_bits - 4, (bits + 1) << (CLZ( bits ) + 1) );
#else
	++bits;
	if ( !(bits & 0xFFFF0000u) )
		bits <<= 16;
	
	if ( !TOP_BIT( bits ) )
		while ( !TOP_BIT( bits <<= 1 ) ) { }
	bits <<= 1;
	
	SET32( out_bits - 4, bits );
#endif
	
	/* Remaining bytes should be in narrow range */
	assert( in_max + in_max_offset - in >= in_max_offset - 6 );
	assert( in_max + in_max_offset - in <= in_max_offset + 1 );
	
	{
		/* Copy final bytes */
		unsigned t0 = GET32( in_max + (in_max_offset - 12) );
		unsigned t1 = GET32( in_max + (in_max_offset -  8) );
		unsigned t2 = GET32( in_max + (in_max_offset -  4) );
		SET32( out, 0 ); /* End marker */
		SET32( out + 4, t0 );
		SET32( out + 8, t1 );
		SET32( out + 12, t2 );
		out += 16;
	}
	
	return out - (byte*) out_min;
}

int blz_pack( void const* in_min, int size, void* out, blz_pack_t* dict )
{
	byte const* in = (byte const*) in_min;
	assert( (unsigned) size <= 0x7FFFFFFF );
	if ( size >= 31 && dict ) /* 31 repeated bytes compress to 34 bytes */
	{
		/* clear dict */
		byte const** dout = dict->dict;
		int count;
		in += 4;
		for ( count = 0x1000; count--; )
			*dout++ = in;
		in -= 4;
		
		{
			int out_size = blz_pack_( in, size, out, dict );
			if ( (unsigned) out_size < (unsigned) size + 4 )
				return out_size;
		}
	}
	SET32( out, -size );
	memcpy( (byte*) out + 4, in, size );
	
	#ifndef NDEBUG
	{
		/* If these assertions fail, enable the appropriate byte order macro at the
		top of this file */
		static byte const data [5] = { 0x12, 0x34, 0x56, 0x78, 0x9A };
		assert( GET32( data     ) == BIG_LITTLE( 0x12345678, 0x78563412 ) );
		assert( GET32( data + 1 ) == BIG_LITTLE( 0x3456789A, 0x9A785634 ) );
		
		{
			volatile bits_t i = 0x40000000;
			assert( !TOP_BIT( i ) && TOP_BIT( i << 1 ) && !TOP_BIT( i << 2 ) && !ALL_BITS( i << 2 ) );
		}
	}
	#endif
	
	return size + 4;
}

/* Unpack */

int blz_size( void const* in )
{
	int n = GET32S( in );
	if ( n < 0 )
		n = -n;
	return n;
}

int blz_unpack( void const* in_min, void* out_min )
{
	int out_size = GET32S( in_min );
	byte const* BLZ_RESTRICT in = (byte const*) in_min + 4;
	byte* out = (byte*) out_min;
	
	if ( out_size > 0 )
	{
		byte* const out_end = out + out_size;
		
		/* Everything fits in registers, even on 32-bit x86 */
		unsigned data;
		bits_t bits;
		
refill: /* Refill bits */
		bits = GET32( in );
		in += 4;
		data = GET16( in );
		if ( TOP_BIT( bits ) ) /* 48% */
			goto literal;
		if ( !bits ) /* 1% */
			goto final_bytes;

match:  { /* Matching symbol */
			int off;
			bits <<= 1;
			off = BIG_LITTLE( data & 0x1FFF, data >> 3 );
			if ( !BIG_LITTLE( data >>= 13, data &= 0x07 ) ) /* 14% */
				goto long_match;
copy_bytes:
			/*assert( data > 0 && (size_t) off <= (size_t) (out - (byte*) out_min) );*/
			
			/* Copy bytes */
			off = -off;
			{
				byte* const end = out + 2 + data;
				SET32( out, GET32( out + off ) );
				if ( data > 2 ) /* 30% */
				{
					out += 4;
					do
					{
						SET32( out, GET32( out + off ) );
						out += 4;
					}
					while ( out < end ); /* 55% */
				}
				out = end;
			}
			in += 2;
			
			data = GET16( in );
			if ( !TOP_BIT( bits ) ) /* 60% */
				goto match;
literal:
			if ( ALL_BITS( bits <<= 1 ) == 0 ) /* 7% */
				goto refill;
			
			/* Literal byte */
			*out++ = BIG_LITTLE( data >> 8, data );
			++in;

			data = GET16( in );
			if ( TOP_BIT( bits ) ) /* 60% */
				goto literal;
			
			/* Start decoding match */
			bits <<= 1;
			off = BIG_LITTLE( data & 0x1FFF, data >> 3 );
			if ( BIG_LITTLE( data >>= 13, data &= 0x07 ) != 0 ) /* 86% */
				goto copy_bytes;
			
			/* Longer match encodings */
long_match:
			data = BIG_LITTLE( off >> 8, off & 0x1F );
			++in;
			off = GET16( in );
			if ( data ) /* 92% */
				goto copy_bytes;
			
			off  = GET32( in );
			data = GET32( in + 4 );
			in += 6;
			goto copy_bytes;
		}
	
final_bytes: /* Copy final bytes */
		{
			unsigned t0 = GET32( in     );
			unsigned t1 = GET32( in + 4 );
			unsigned t2 = GET32( in + 8 );
			SET32( out_end - 12, t0 );
			SET32( out_end -  8, t1 );
			SET32( out_end -  4, t2 );
		}
	}
	else
	{
		memcpy( out, in, (out_size = -out_size) );
	}
	return out_size;
}

#else

/**** Slow version ****/

char const blz_version [] = "slow";

/* Makes heavy use of pre-increment since that is more efficient on popular RISC processors */

/* Pack */

#define PRE_GOTO( off ) \
{\
	data = (in [off + 2] << 16) | (in [off + 1] << 8);\
	data |= *(in += off);\
}
	
#define GOTO_NEXT \
	if ( !TOP_BIT( bits ) ) /* 97% */ \
		goto match;\
	goto flush;

enum { in_max_offset = 9 }; /* 8+ byte match just before end will read this far ahead */

static int blz_pack_( byte const* const in_min, int size,
		void* const out_min, blz_pack_t* const dict )
{
	byte const* BLZ_RESTRICT in     = in_min;
	byte const* const        in_max = in + size - in_max_offset;
	
	byte*       out      = (byte*) out_min;
	byte* const out_max  = out + size;
	byte*       out_bits;
	
	unsigned data;
	bits_t bits = 2;
	
	out [0] = size;
	out [1] = size >> 8;
	out [2] = size >> 16;
	out [3] = size >> 24;
	out [4] = in [0];
	out [5] = in [1];
	in  += 2;
	out_bits = (out += 9);
	
literal: /* Literal byte */
	{
		int const t = *in;
		PRE_GOTO( 1 );
		*++out = t;
		if ( TOP_BIT( ++bits ) ) /* 3% */
			goto flush;
	}
	
match: /* Check for match and update dict */
	{
		/* get then update dict entry and handle end of input */
		int off = (data >> 12) ^ (data & 0xFFF);
		byte const* const BLZ_RESTRICT match = dict->dict [off];
		bits <<= 1;
		dict->dict [off] = in;
		if ( in >= in_max ) /* 1% */
			goto final_bytes;
		
		/* compare first 3 bytes and pre-read 4th bytes */
		data ^= match [0];
		data ^= match [1] << 8;
		{
			unsigned const in3    = in    [3];
			unsigned const match3 = match [3];
			if ( data != ((unsigned) match [2] << 16) ) /* 42% first 3 bytes don't match */
				goto literal;
			
			off = in - match;
			if ( in3 != match3 ) /* 53% 3 byte match */
			{
				if ( off >= (1 << 13) ) /* 19% too far */
					goto literal;
				
				/* LLLhhhhh llllllll */
				PRE_GOTO( 3 );
				*(out += 2) = off;
				out [-1] = (off >> 8) | (1 << 5);
				GOTO_NEXT
			}
		}
		
		{
			/* 4-7 byte match */
			int len = 2 << 5;
			if ( *(in += 4) == match [4] ) /* 59% */
			{
				len = 3 << 5;
				if ( *++in == match [5] ) /* 66% */
				{
					len = 4 << 5;
					if ( *++in == match [6] ) /* 71% */
					{
						len = 5 << 5;
						if ( *++in == match [7] ) /* 75% */
							goto long_match;
					}
				}
			}
			PRE_GOTO( 0 );
			
			/* Encode offset and length */
			if ( off < (1 << 13) ) /* 82% */
			{
				/* LLLhhhhh llllllll */
				*(out += 2) = off;
				out [-1] = (off >> 8) | len;
				GOTO_NEXT
			}
			
			len >>= 5;
			if ( off < (1 << 16) ) /* 91% */
			{
				/* 000LLLLL llllllll hhhhhhhh */
				out [1] = len;
				out [2] = off;
				*(out += 3) = off >> 8;
				GOTO_NEXT
			}
			in -= 2;
			in -= len;
			goto literal;
		}
		
	long_match:
		{
			/* 8+ byte match */
			byte const* const start = in - 4;
			off = -off;
			++in;
			{
				int x, y;
				do
				{
					x = *in;
					y = in [off];
					if ( in >= in_max )
						break;
					++in;
				}
				while ( x == y ); /* 92% */
			}
			
			/* Encode offset and length */
			{
				int len = in - start;
				PRE_GOTO( -1 );
				off = -off;
				
				if ( len < (1 << 5) ) /* 90% */
				{
					out [1] = len;
					if ( off < (1 << 16) ) /* 98% */
					{
						/* 000LLLLL llllllll hhhhhhhh */
						out [2] = off;
						*(out += 3) = off >> 8;
						GOTO_NEXT
					}
					
					if ( len <= 9 - 2 ) /* 52% */
					{
						/* TODO: lame */
						in -= 2;
						in -= len;
						goto literal;
					}
				}
				
				/* 8-bit 0, 32-bit offset, 32-bit length */
				out [1] = 0;
				out [2] = off;
				out [3] = off >> 8;
				out [4] = off >> 16;
				out [5] = off >> 24;
				out [6] = len;
				out [7] = len >> 8;
				out [8] = len >> 16;
				*(out += 9) = len >> 24;
				GOTO_NEXT
			}
		}
	}

	/* This is at bottom of loop to make branches here positive and branches back
	negative, which helps CPUs that lack dynamic branch prediction */
	
flush: /* Flush bits */
	out += 4;
	out_bits [-3] = (bits << 1) + 1;
	out_bits [-2] = bits >> 7;
	out_bits [-1] = bits >> 15;
	out_bits [ 0] = bits >> 23;
	out_bits = out;
	bits = 1;
	if ( out < out_max ) /* 99% */
		goto match;
	return -1; /* no compression */

final_bytes:
	/* Right-justify and flush final bits */
	{
		++bits;
		if ( (bits & 0xFFFF0000) == 0 )
			bits <<= 16;
		
		if ( !TOP_BIT( bits ) )
			while ( !TOP_BIT( bits <<= 1 ) ) { }
		bits <<= 1;
		
		out_bits [-3] = bits;
		out_bits [-2] = bits >> 8;
		out_bits [-1] = bits >> 16;
		out_bits [ 0] = bits >> 24;
	}
	
	{
		byte const* const end = in_max + in_max_offset;
		
		/* Remaining bytes should be in narrow range */
		assert( end - in >= in_max_offset - 6 );
		assert( end - in <= in_max_offset + 1 );
		
		/* End marker */
		out [1] = 0;
		out [2] = 0;
		out [3] = 0;
		*(out += 4) = 0;
		
		/* Copy final bytes */
		do
		{
			*++out = *in++;
		}
		while ( in < end );
	}
	
	return out + 1 - (byte*) out_min;
}

int blz_pack( void const* in_min, int size, void* out, blz_pack_t* dict )
{
	byte const* in = (byte const*) in_min;
	assert( (unsigned) size <= 0x7FFFFFFF );
	if ( size >= 21 && dict ) /* 21 repeated bytes compress to 24 bytes */
	{
		/* clear dict */
		byte const** dout = dict->dict;
		int count;
		in += 1; /* avoid match at beginning since out is one less in unpack */
		for ( count = 0x1000; count--; )
			*dout++ = in;
		in -= 1;
		
		{
			int out_size = blz_pack_( in, size, out, dict );
			if ( (unsigned) out_size < (unsigned) size + 4 )
				return out_size;
		}
	}
	((byte*) out) [0] = size;
	((byte*) out) [1] = size >> 8;
	((byte*) out) [2] = size >> 16;
	((byte*) out) [3] = 0x80 | (size >> 24);
	memcpy( (byte*) out + 4, in, size );
	
	#ifndef NDEBUG
	{
		volatile bits_t i = 0x40000000;
		assert( !TOP_BIT( i ) && TOP_BIT( i << 1 ) && !TOP_BIT( i << 2 ) && !ALL_BITS( i << 2 ) );
	}
	#endif
	
	return size + 4;
}

/* Unpack */

int blz_size( void const* in_ )
{
	byte const* in = (byte const*) in_;
	return in [0] | (in [1] << 8) | (in [2] << 16) | (in [3] << 24 & 0x7F);
}

int blz_unpack( void const* in_min, void* out_min )
{
	byte const* BLZ_RESTRICT in = (byte const*) in_min;
	byte* out = (byte*) out_min;
	bits_t out_size = in [0] | (in [1] << 8) | (in [2] << 16) | (in [3] << 24);
	in += 4;
	
	if ( !TOP_BIT( out_size ) )
	{
		byte* const out_max = out + out_size - 2;
		unsigned data; /* usually holds next input byte, read ahead of time */
		bits_t bits;
		
		*out   = in [0];
		*++out = in [1];
		in += 2;
		
refill: { /* Refill bits */
			bits_t first = (in [3] << 24);
			bits = in [0] | first | (in [1] << 8) | (in [2] << 16);
			data = *(in += 4);
			if ( TOP_BIT( first ) ) /* 48% */
				goto literal;
			if ( !bits ) /* 1% */
				goto final_bytes;
		}
		
match:  { /* Matching symbol */
			int len, off;
			bits <<= 1;
			off    = *++in;
			len    = data >> 5;
			data  &= 0x1F;
			if ( !len ) /* 17% */
				goto long_match;
			
copy_bytes: /* Copy len+2 bytes from out-off to out */
			off |= data << 8;
			{
				byte const* match = out - off;
				/*assert( len > 0 && (size_t) (match - (byte*) out_min) < (size_t) out_size );*/
				
				/* match may = out + 1, so can't read ahead at all */
				out [1]     = match [1];
				*(out += 2) = match [2];
				data        = *(match += 3);
				if ( --len ) /* 50% */
				{
					do
					{
						*++out = data;
						data = *++match;
					}
					while ( --len ); /* 80% */
				}
				*++out = data;
			}
			
			data = *++in;
			if ( !TOP_BIT( bits ) ) /* 58% */
				goto match;
literal:
			if ( ALL_BITS( bits <<= 1 ) == 0 ) /* 7% */
				goto refill;
			
			/* Literal byte */
			*++out = data;
			data   = *++in;
			if ( TOP_BIT( bits ) ) /* 60% */
				goto literal;
			
			/* Start decoding match */
			bits <<= 1;
			off    = *++in;
			len    = data >> 5;
			data  &= 0x1F;
			if ( len ) /* 82% */
				goto copy_bytes;
			
long_match: /* Longer match encodings */
			len  = data;
			data = *++in;
			if ( len ) /* 93% */
				goto copy_bytes;
			
			off |= in [1] << 16;
			off |= in [2] << 24;
			len  = in [3];
			len |= in [4] <<  8;
			len |= in [5] << 16;
			len |= *(in += 6) << 24;
			goto copy_bytes;
		}
		
final_bytes: /* Copy final bytes */
		do
		{
			*++out = data;
			data   = *++in;
		}
		while ( out < out_max );
		*++out = data;
	}
	else
	{
		memcpy( out, in, out_size ^= 0x80000000u );
	}
	return out_size;
}

#endif
