/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SYMBIAN_PORTDEFS_H
#define SYMBIAN_PORTDEFS_H

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <e32def.h>

#include <e32std.h>
#include <libc\math.h>

/* define pi */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  /*  M_PI  */


// Enable Symbians own datatypes
// This is done for two reasons
// a) uint is already defined by Symbians libc component
// b) Symbian is using its "own" datatyping, and the Scummvm port
//    should follow this to ensure the best compability possible.
typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short int uint16;
typedef signed short int int16;
typedef unsigned long int uint32;
typedef signed long int int32;

// Define SCUMMVM_DONT_DEFINE_TYPES to prevent scummsys.h from trying to
// re-define those data types.
#define SCUMMVM_DONT_DEFINE_TYPES

#define SMALL_SCREEN_DEVICE

#define DISABLE_COMMAND_LINE

#if defined(USE_TREMOR) && !defined(USE_VORBIS)
#define USE_VORBIS // make sure this one is defined together with USE_TREMOR!
#endif

// hack in some tricks to work around not having these fcns for Symbian
// and we _really_ don't wanna link with any other windows LIBC library!
#if defined(__GCC32__)
	// taken from public domain http://www.opensource.apple.com/darwinsource/WWDC2004/gcc_legacy-939/gcc/floatlib.c
	#define SIGNBIT		0x80000000
	#define HIDDEN		(1 << 23)
	#define EXCESSD		1022
	#define EXPD(fp)	(((fp.l.upper) >> 20) & 0x7FF)
	#define SIGND(fp)	((fp.l.upper) & SIGNBIT)
	#define HIDDEND_LL	((long long)1 << 52)
	#define MANTD_LL(fp)	((fp.ll & (HIDDEND_LL-1)) | HIDDEND_LL)

	union double_long {
	    double d;
	    struct {
	      long upper;
	      unsigned long lower;
	    } l;
	    long long ll;
	};

	/* convert double float to double int (dfdi) */
	long long inline
	scumm_fixdfdi (double a1) { // __fixdfdi (double a1)
	    register union double_long dl1;
	    register int exp;
	    register long long l;

	    dl1.d = a1;

	    if (!dl1.l.upper && !dl1.l.lower)
			return (0);

	    exp = EXPD (dl1) - EXCESSD - 64;
	    l = MANTD_LL(dl1);

	    if (exp > 0) {
		l = (long long)1<<63;
		if (!SIGND(dl1))
		    l--;
		return l;
	    }

	    /* shift down until exp = 0 or l = 0 */
	    if (exp < 0 && exp > -64 && l)
			l >>= -exp;
	    else
			return (0);

	    return (SIGND (dl1) ? -l : l);
	}

	/*	okay, okay: I admit it: I absolutely have _NO_ idea why __fixdfdi does not get linked in by gcc from libgcc.a
		because I know it's in there: I checked with `ar x _fixdfdi.o libgcc.a` and the symbol is in there, so I'm lost
		and had to fix it this way. I tried all gcc and ld options I could find: no hope :( If someone can enlighten me:
		feel free to let me know at sumthinwicked@users.sf.net! Much obliged.
		PS1. I think for __fixunsdfdi they have made a circumvention by having to add STATICLIBRARY EGCC.LIB
		PS2. http://gcc.gnu.org/ml/gcc-bugs/2004-01/msg01596.html might have found out the same problem there
	*/

#elif defined(__WINS__) // WINS
	extern "C" int symbian_snprintf(char *text, size_t maxlen, const char *fmt, ...);
	extern "C" int symbian_vsnprintf(char *text, size_t maxlen, const char *fmt, va_list ap);
	#define snprintf(buf,len,args...) symbian_snprintf(buf,len,args)
	#define vsnprintf(buf,len,format,valist) symbian_vsnprintf(buf,len,format,valist)

	void*	symbian_malloc	(size_t _size);

	#define malloc symbian_malloc
#else // GCCE and the rest
	extern "C" int symbian_snprintf(char *text, size_t maxlen, const char *fmt, ...);
	extern "C" int symbian_vsnprintf(char *text, size_t maxlen, const char *fmt, va_list ap);
	#define snprintf(buf,len,args...) symbian_snprintf(buf,len,args)
	#define vsnprintf(buf,len,format,valist) symbian_vsnprintf(buf,len,format,valist)
#endif

#ifndef __WINS__
#define USE_ARM_GFX_ASM
#define USE_ARM_SMUSH_ASM
#define USE_ARM_COSTUME_ASM
#define USE_ARM_SOUND_ASM
#endif
// This is not really functioning yet.
// Default SDL keys should map to standard keys I think!
//#define ENABLE_KEYMAPPER

// Symbian bsearch implementation is flawed
void *scumm_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
#define bsearch	scumm_bsearch
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

// we cannot include SymbianOS.h everywhere, but this works too (functions code is in SymbianOS.cpp)
namespace Symbian {
extern char* GetExecutablePath();
}
#endif
