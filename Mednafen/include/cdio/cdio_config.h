/** \file cdio_config.h
 *  \brief  configuration-time settings useful in compilation; a run-time
    version of config.h
*/
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* compiler does least-significant bit first in struct bitfields */
#define BITFIELD_LSBF 

/* Define 1 if you are compiling using cygwin */
/* #undef CYGWIN */

/* what to put between the brackets for empty arrays */
#define EMPTY_ARRAY_SIZE 

/* Define 1 if you have BSDI-type CD-ROM support */
/* #undef HAVE_BSDI_CDROM */

/* Define to 1 if you have the `bzero' function. */
#define HAVE_BZERO 1

/* Define this if you have libcddb installed */
/* #undef HAVE_CDDB */

/* Define to 1 if you have the <CoreFoundation/CFBase.h> header file. */
#define HAVE_COREFOUNDATION_CFBASE_H 1

/* Define to 1 if you have the <curses.h> header file. */
/* #undef HAVE_CURSES_H */

/* Define 1 if you have Darwin OS X-type CD-ROM support */
#define HAVE_DARWIN_CDROM 1

/* Define if time.h defines extern long timezone and int daylight vars. */
#define HAVE_DAYLIGHT 1

/* Define to 1 if you have the Apple DiskArbitration framework */
#define HAVE_DISKARBITRATION 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `drand48' function. */
#define HAVE_DRAND48 1

/* Define to 1 if you have the <dvd.h> header file. */
/* #undef HAVE_DVD_H */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define 1 if you have FreeBSD CD-ROM support */
/* #undef HAVE_FREEBSD_CDROM */

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
/* #undef HAVE_FSEEKO */

/* Define to 1 if you have the `ftruncate' function. */
#define HAVE_FTRUNCATE 1

/* Define to 1 if you have the `geteuid' function. */
#define HAVE_GETEUID 1

/* Define to 1 if you have the `getgid' function. */
#define HAVE_GETGID 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the `getpwuid' function. */
#define HAVE_GETPWUID 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `getuid' function. */
#define HAVE_GETUID 1

/* Define to 1 if you have the <glob.h> header file. */
#define HAVE_GLOB_H 1

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <IOKit/IOKitLib.h> header file. */
#define HAVE_IOKIT_IOKITLIB_H 1

/* Supports ISO _Pragma() macro */
#define HAVE_ISOC99_PRAGMA 

/* Define 1 if you want ISO-9660 Joliet extension support. You must have also
   libiconv installed to get Joliet extension support. */
#define HAVE_JOLIET 1

/* Define this if your libcurses has keypad */
#define HAVE_KEYPAD 

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
#define HAVE_LANGINFO_CODESET 1

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define 1 if you have Linux-type CD-ROM support */
/* #undef HAVE_LINUX_CDROM */

/* Define to 1 if you have the <linux/cdrom.h> header file. */
/* #undef HAVE_LINUX_CDROM_H */

/* Define 1 if timeout is in cdrom_generic_command struct */
/* #undef HAVE_LINUX_CDROM_TIMEOUT */

/* Define to 1 if you have the <linux/major.h> header file. */
/* #undef HAVE_LINUX_MAJOR_H */

/* Define to 1 if you have the <linux/version.h> header file. */
/* #undef HAVE_LINUX_VERSION_H */

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the `lstat' function. */
#define HAVE_LSTAT 1

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <ncurses.h> header file. */
#define HAVE_NCURSES_H 1

/* Define 1 if you have NetBSD CD-ROM support */
/* #undef HAVE_NETBSD_CDROM */

/* Define to 1 if you have the <pwd.h> header file. */
#define HAVE_PWD_H 1

/* Define to 1 if you have the `rand' function. */
#define HAVE_RAND 1

/* Define to 1 if you have the `readlink' function. */
#define HAVE_READLINK 1

/* Define 1 if you want ISO-9660 Rock-Ridge extension support. */
#define HAVE_ROCK 1

/* Define to 1 if you have the `setegid' function. */
#define HAVE_SETEGID 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1

/* Define to 1 if you have the `sleep' function. */
#define HAVE_SLEEP 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define 1 if you have Solaris CD-ROM support */
/* #undef HAVE_SOLARIS_CDROM */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdbool.h> header file. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define this if you have struct timespec */
#define HAVE_STRUCT_TIMESPEC 

/* Define to 1 if you have the <sys/cdio.h> header file. */
/* #undef HAVE_SYS_CDIO_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/timeb.h> header file. */
#define HAVE_SYS_TIMEB_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define this <sys/stat.h> defines S_ISLNK() */
#define HAVE_S_ISLNK 

/* Define this <sys/stat.h> defines S_ISSOCK() */
#define HAVE_S_ISSOCK 

/* Define to 1 if timegm is available */
#define HAVE_TIMEGM 1

/* Define if you have an extern long timenzone variable. */
