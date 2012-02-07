// Sets up common environment for Shay Green's libraries.

// Gb_Snd_Emu 0.2.0
#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#include <stdlib.h>
#include <limits.h>

/* Uncomment to have Gb_Apu run at 4x normal clock rate (16777216 Hz), useful in
a Game Boy Advance emulator. */
#define GB_APU_OVERCLOCK 4

#define GB_APU_CUSTOM_STATE 1

/* Uncomment to enable platform-specific (and possibly non-portable) optimizations. */
#define BLARGG_NONPORTABLE 1

/* Uncomment if automatic byte-order determination doesn't work */
//#define BLARGG_BIG_ENDIAN 1

#ifndef STATIC_CAST
	#if __GNUC__ >= 4
		#define STATIC_CAST(T,expr) static_cast<T> (expr)
		#define CONST_CAST( T,expr) const_cast<T> (expr)
	#else
		#define STATIC_CAST(T,expr) ((T) (expr))
		#define CONST_CAST( T,expr) ((T) (expr))
	#endif
#endif

/* blargg_vector - very lightweight vector of POD types (no constructor/destructor) */
template<class T>
class blargg_vector {
	T* begin_;
	size_t size_;
public:
	blargg_vector() : begin_( 0 ), size_( 0 ) { }
	~blargg_vector() { free( begin_ ); }
	size_t size() const { return size_; }
	T* begin() const { return begin_; }
	T* end() const { return begin_ + size_; }
	const char * resize( size_t n )
	{
		// TODO: blargg_common.cpp to hold this as an outline function, ugh
		void* p = realloc( begin_, n * sizeof (T) );
		if ( p )
			begin_ = (T*) p;
		else if ( n > size_ ) // realloc failure only a problem if expanding
			return "Out of memory";
		size_ = n;
		return 0;
	}
	T& operator [] ( size_t n ) const
	{
		return begin_ [n];
	}
};

// BLARGG_COMPILER_HAS_BOOL: If 0, provides bool support for old compiler. If 1,
// compiler is assumed to support bool. If undefined, availability is determined.
#ifndef BLARGG_COMPILER_HAS_BOOL
	#if defined (__MWERKS__)
		#if !__option(bool)
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	#elif defined (_MSC_VER)
		#if _MSC_VER < 1100
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	#elif defined (__GNUC__)
		// supports bool
	#elif __cplusplus < 199711
		#define BLARGG_COMPILER_HAS_BOOL 0
	#endif
#endif
#if defined (BLARGG_COMPILER_HAS_BOOL) && !BLARGG_COMPILER_HAS_BOOL
	typedef int bool;
	const bool true  = 1;
	const bool false = 0;
#endif

/* HAVE_STDINT_H: If defined, use <stdint.h> for int8_t etc.*/
#if defined (HAVE_STDINT_H)
	#include <stdint.h>

/* HAVE_INTTYPES_H: If defined, use <stdint.h> for int8_t etc.*/
#elif defined (HAVE_INTTYPES_H)
	#include <inttypes.h>
#endif

#endif
