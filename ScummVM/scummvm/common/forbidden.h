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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_FORBIDDEN_H
#define COMMON_FORBIDDEN_H

/**
 * @file
 * This header file is meant to help ensure that engines and
 * infrastructure code do not make use of certain "forbidden" APIs, such
 * as fopen(), setjmp(), etc.
 * This is achieved by re-#defining various symbols to a "garbage"
 * string which then triggers a compiler error.
 *
 * Backend files may #define FORBIDDEN_SYMBOL_ALLOW_ALL if they
 * have to access functions like fopen, fread etc.
 * Regular code, esp. code in engines/, should never do that.
 * To ease transition, though, we allow re-enabling selected symbols
 * in frontend code. However, this should only be used as a temporary
 * measure. Especially new code should avoid this at all costs.
 */

#ifndef FORBIDDEN_SYMBOL_ALLOW_ALL

// Make sure scummsys.h is always included first
#include "common/scummsys.h"


/**
 * The garbage string to use as replacement for forbidden symbols.
 *
 * The reason for this particular string is the following:
 * By including a space and some non-alphanumeric symbols we trigger
 * a compiler error. By including the words "forbidden symbol" (which
 * the compiler will hopefully print along with its own error message),
 * we try to make clear what is causing the error.
 */
#define FORBIDDEN_SYMBOL_REPLACEMENT	FORBIDDEN_look_at_common_forbidden_h_for_more_info SYMBOL !%*


#ifndef FORBIDDEN_SYMBOL_EXCEPTION_printf
#undef printf
#define printf	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#undef fprintf
#define fprintf	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_vprintf
#undef vprintf
#define vprintf	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_vfprintf
#undef vfprintf
#define vfprintf	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_FILE
#undef FILE
#define FILE	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_stdin
#undef stdin
#define stdin	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_stdout
#undef stdout
#define stdout	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_stderr
#undef stderr
#define stderr	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fopen
#undef fopen
#define fopen(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fclose
#undef fclose
#define fclose(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fread
#undef fread
#define fread(a,b,c,d)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#undef fwrite
#define fwrite(a,b,c,d)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fseek
#undef fseek
#define fseek(a,b,c)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_ftell
#undef ftell
#define ftell(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_feof
#undef feof
#define feof(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fgetc
#undef fgetc
#define fgetc(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fputc
#undef fputc
#define fputc(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fgets
#undef fgets
#define fgets(a,b,c)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_fputs
#undef fputs
#define fputs(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_getc
#undef getc
#define getc(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_putc
#undef putc
#define putc(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_gets
#undef gets
#define gets(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_puts
#undef puts
#define puts(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_getchar
#undef getchar
#define getchar()	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_putchar
#undef putchar
#define putchar(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

// mingw-w64 uses [set|long]jmp in system headers
#ifndef __MINGW64__
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#undef setjmp
#define setjmp(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#undef longjmp
#define longjmp(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif
#endif // __MINGW64__

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_system
#undef system
#define system(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_exit
#undef exit
#define exit(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_abort
#undef abort
#define abort()	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_getenv
#undef getenv
#define getenv(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_putenv
#undef putenv
#define putenv(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setenv
#undef setenv
#define setenv(a,b,c)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_unsetenv
#undef unsetenv
#define unsetenv(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif


//
// Disable various symbols from time.h
//
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_time_h

	/*
	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_time_t
	#undef time_t
	#define time_t	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif
	*/

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_asctime
	#undef asctime
	#define asctime(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_clock
	#undef clock
	#define clock()	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_ctime
	#undef ctime
	#define ctime(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_difftime
	#undef difftime
	#define difftime(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_getdate
	#undef getdate
	#define getdate(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_gmtime
	#undef gmtime
	#define gmtime(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_localtime
	#undef localtime
	#define localtime(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_mktime
	#undef mktime
	#define mktime(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_time
	#undef time
	#define time(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

#endif // FORBIDDEN_SYMBOL_EXCEPTION_time_h

//
// Disable various symbols from unistd.h
//
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_chdir
	#undef chdir
	#define chdir(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_getcwd
	#undef getcwd
	#define getcwd(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_getwd
	#undef getwd
	#define getwd(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_unlink
	#undef unlink
	#define unlink(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

#endif // FORBIDDEN_SYMBOL_EXCEPTION_unistd_h


//
// Disable various symbols from ctype.h
//
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_ctype_h

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_isalnum
	#undef isalnum
	#define isalnum(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_isalpha
	#undef isalpha
	#define isalpha(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_isdigit
	#undef isdigit
	#define isdigit(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_isnumber
	#undef isnumber
	#define isnumber(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_islower
	#undef islower
	#define islower(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_isspace
	#undef isspace
	#define isspace(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#ifndef FORBIDDEN_SYMBOL_EXCEPTION_isupper
	#undef isupper
	#define isupper(a)	FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

#endif // FORBIDDEN_SYMBOL_EXCEPTION_ctype_h

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#undef mkdir
#define mkdir(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

/*
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setlocale
#undef setlocale
#define setlocale(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif
*/

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setvbuf
#undef setvbuf
#define setvbuf(a,b,c,d)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif


#ifndef FORBIDDEN_SYMBOL_EXCEPTION_tmpfile
#undef tmpfile
#define tmpfile()	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_tmpnam
#undef tmpnam
#define tmpnam(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_tempnam
#undef tempnam
#define tempnam(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_rand
#undef rand
#define rand()	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_srand
#undef srand
#define srand(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_random
#undef random
#define random()	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_srandom
#undef srandom
#define srandom(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif


#ifndef FORBIDDEN_SYMBOL_EXCEPTION_stricmp
#undef stricmp
#define stricmp(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_strnicmp
#undef strnicmp
#define strnicmp(a,b,c)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp
#undef strcasecmp
#define strcasecmp(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_strncasecmp
#undef strncasecmp
#define strncasecmp(a,b,c)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif


/*
 * We also would like to disable the following symbols;
 * however, these are also frequently used in regular code,
 * e.g. for method names, so we don't override them.
 * - read
 * - remove
 * - write
 * - ...
 */


#endif


#endif
