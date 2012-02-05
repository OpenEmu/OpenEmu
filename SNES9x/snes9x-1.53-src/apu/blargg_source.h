/* Included at the beginning of library source files, after all other #include lines.
Sets up helpful macros and services used in my source code. They don't need
module an annoying module prefix on their names since they are defined after
all other #include lines. */

// snes_spc 0.9.0
#ifndef BLARGG_SOURCE_H
#define BLARGG_SOURCE_H

// If debugging is enabled, abort program if expr is false. Meant for checking
// internal state and consistency. A failed assertion indicates a bug in the module.
// void assert( bool expr );
#include <assert.h>

// If debugging is enabled and expr is false, abort program. Meant for checking
// caller-supplied parameters and operations that are outside the control of the
// module. A failed requirement indicates a bug outside the module.
// void require( bool expr );
#undef require
#define require( expr ) assert( expr )

// Like printf() except output goes to debug log file. Might be defined to do
// nothing (not even evaluate its arguments).
// void dprintf( const char* format, ... );
static inline void blargg_dprintf_( const char*, ... ) { }
#undef dprintf
#define dprintf (1) ? (void) 0 : blargg_dprintf_

// If enabled, evaluate expr and if false, make debug log entry with source file
// and line. Meant for finding situations that should be examined further, but that
// don't indicate a problem. In all cases, execution continues normally.
#undef check
#define check( expr ) ((void) 0)

// If expr yields error string, return it from current function, otherwise continue.
#undef RETURN_ERR
#define RETURN_ERR( expr ) do {                         \
		blargg_err_t blargg_return_err_ = (expr);               \
		if ( blargg_return_err_ ) return blargg_return_err_;    \
	} while ( 0 )

// If ptr is 0, return out of memory error string.
#undef CHECK_ALLOC
#define CHECK_ALLOC( ptr ) do { if ( (ptr) == 0 ) return "Out of memory"; } while ( 0 )

// Avoid any macros which evaluate their arguments multiple times
#undef min
#undef max

#define DEF_MIN_MAX( type ) \
	static inline type min( type x, type y ) { if ( x < y ) return x; return y; }\
	static inline type max( type x, type y ) { if ( y < x ) return x; return y; }

DEF_MIN_MAX( int )
DEF_MIN_MAX( unsigned )
DEF_MIN_MAX( long )
DEF_MIN_MAX( unsigned long )
DEF_MIN_MAX( float )
DEF_MIN_MAX( double )

#undef DEF_MIN_MAX

/*
// using const references generates crappy code, and I am currenly only using these
// for built-in types, so they take arguments by value

// TODO: remove
inline int min( int x, int y ) 
template<class T>
inline T min( T x, T y )
{
	if ( x < y )
		return x;
	return y;
}

template<class T>
inline T max( T x, T y )
{
	if ( x < y )
		return y;
	return x;
}
*/

// TODO: good idea? bad idea?
#undef byte
#define byte byte_
typedef unsigned char byte;

// deprecated
#define BLARGG_CHECK_ALLOC CHECK_ALLOC
#define BLARGG_RETURN_ERR RETURN_ERR

// BLARGG_SOURCE_BEGIN: If defined, #included, allowing redefition of dprintf and check
#ifdef BLARGG_SOURCE_BEGIN
	#include BLARGG_SOURCE_BEGIN
#endif

#endif
