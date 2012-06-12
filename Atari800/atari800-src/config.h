/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* The Austin Franklin 80 column card. */
/* #undef AF80 */

/* Target: Android */
/* #undef ANDROID */
//#define DREAMCAST

/* Target: standard I/O. */
//#define BASIC 1

/* Define to use buffered debug output. */
/* #undef BUFFERED_LOG */

/* Define to allow sound clipping. */
/* #undef CLIP_SOUND */

/* Define to 1 if the `closedir' function returns void instead of `int'. */
/* #undef CLOSEDIR_VOID */

/* Define to allow console sound (keyboard clicks). */
/* #undef CONSOLE_SOUND */

/* Define to activate crash menu after CIM instruction. */
/* #undef CRASH_MENU */

/* Define to disable bitmap graphics emulation in CURSES target. */
/* #undef CURSES_BASIC */

/* Alternate config filename due to 8+3 fs limit. */
/* #undef DEFAULT_CFG_NAME */

/* Target: Windows with DirectX. */
/* #undef DIRECTX */

/* Define to use dirty screen partial repaints. */
/* #undef DIRTYRECT */

/* Define to use back slash as directory separator. */
/* #undef DIR_SEP_BACKSLASH */

/* Target: DOS VGA. */
/* #undef DOSVGA */

/* Define to enable DOS style drives support. */
/* #undef DOS_DRIVES */

/* Define to enable event recording. */
/* #undef EVENT_RECORDING */

/* Target: Atari Falcon system. */
/* #undef FALCON */

/* Define to use m68k assembler CPU core for Falcon target. */
/* #undef FALCON_CPUASM */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the `atexit' function. */
#define HAVE_ATEXIT 1

/* Define to 1 if you have the `chmod' function. */
#define HAVE_CHMOD 1

/* Define to 1 if you have the `clock' function. */
#define HAVE_CLOCK 1

/* Define to 1 if you have the <direct.h> header file. */
/* #undef HAVE_DIRECT_H */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fdopen' function. */
#define HAVE_FDOPEN 1

/* Define to 1 if you have the `fflush' function. */
#define HAVE_FFLUSH 1

/* Define to 1 if you have the <file.h> header file. */
/* #undef HAVE_FILE_H */

/* Define to 1 if you have the `floor' function. */
#define HAVE_FLOOR 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `fstat' function. */
#define HAVE_FSTAT 1

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define to 1 if you have the `gethostbyaddr' function. */
#define HAVE_GETHOSTBYADDR 1

/* Define to 1 if you have the `gethostbyname' function. */
#define HAVE_GETHOSTBYNAME 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `inet_ntoa' function. */
#define HAVE_INET_NTOA 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `gem' library (-lgem). */
/* #undef HAVE_LIBGEM */

/* Define to 1 if you have the `png' library (-lpng). */
/* #undef HAVE_LIBPNG */

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the `localtime' function. */
#define HAVE_LOCALTIME 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Define to 1 if you have the `mkstemp' function. */
#define HAVE_MKSTEMP 1

/* Define to 1 if you have the `mktemp' function. */
#define HAVE_MKTEMP 1

/* Define to 1 if you have the `modf' function. */
#define HAVE_MODF 1

/* Define to 1 if you have the `nanosleep' function. */
#define HAVE_NANOSLEEP 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the `opendir' function. */
#define HAVE_OPENDIR 1

/* Support for OpenGL graphics acceleration. */
/* #undef HAVE_OPENGL */

/* Define to 1 if you have the <readline/readline.h> header file. */
#define HAVE_READLINE_READLINE_H 1

/* Define to 1 if you have the `rename' function. */
#define HAVE_RENAME 1

/* Define to 1 if you have the `rewind' function. */
#define HAVE_REWIND 1

/* Define to 1 if you have the `rmdir' function. */
#define HAVE_RMDIR 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Define to 1 if you have the `stat' function. */
#define HAVE_STAT 1

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncpy' function. */
#define HAVE_STRNCPY 1

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if you have the `system' function. */
#define HAVE_SYSTEM 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/soundcard.h> header file. */
/* #undef HAVE_SYS_SOUNDCARD_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the `time' function. */
#define HAVE_TIME 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the `tmpfile' function. */
#define HAVE_TMPFILE 1

/* Define to 1 if you have the `tmpnam' function. */
#define HAVE_TMPNAM 1

/* Define to 1 if you have the `uclock' function. */
/* #undef HAVE_UCLOCK */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <unixio.h> header file. */
/* #undef HAVE_UNIXIO_H */

/* Define to 1 if you have the `unlink' function. */
#define HAVE_UNLINK 1

/* Define to 1 if you have the `usleep' function. */
#define HAVE_USLEEP 1

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the <windows.h> header file. */
/* #undef HAVE_WINDOWS_H */

/* Define to 1 if you have the <winsock2.h> header file. */
/* #undef HAVE_WINSOCK2_H */

/* Define to 1 if you have the `_mkdir' function. */
/* #undef HAVE__MKDIR */

/* Define to add IDE harddisk emulation. */
#define IDE 1

/* Define to allow sound interpolation. */
/* #undef INTERPOLATE_SOUND */

/* Target: Java NestedVM. */
/* #undef JAVANVM */

/* Define to use LINUX joystick. */
/* #undef LINUX_JOYSTICK */

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
/* #undef LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Define if mkdir takes only one argument. */
/* #undef MKDIR_TAKES_ONE_ARG */

/* Define to activate assembler in monitor. */
#define MONITOR_ASSEMBLER 1

/* Define to activate code breakpoints and execution history. */
#define MONITOR_BREAK 1

/* Define to activate user-defined breakpoints. */
/* #undef MONITOR_BREAKPOINTS */

/* Define to activate hints in disassembler. */
#define MONITOR_HINTS 1

/* Define to activate 6502 opcode profiling. */
/* #undef MONITOR_PROFILE */

/* Define to activate readline support in monitor. */
#define MONITOR_READLINE 1

/* Define to activate TRACE command in monitor. */
/* #undef MONITOR_TRACE */

/* Target: X11 with Motif. */
/* #undef MOTIF */

/* Define to allow color changes inside a scanline. */
/* #undef NEW_CYCLE_EXACT */

/* Define to use nonlinear POKEY mixing. */
/* #undef NONLINEAR_MIXING */

/* Use NTSC video filter. */
/* #undef NTSC_FILTER */

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "pstehlik@sophics.cz"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Atari800"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Atari800 2.2.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "atari800"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.2.1"

/* Define to use page-based attribute array. */
/* #undef PAGED_ATTRIB */

/* Define to emulate the Black Box. */
#define PBI_BB 1

/* Define to emulate the MIO board. */
#define PBI_MIO 1

/* A prototype 80 column card for the 1090 expansion box. */
/* #undef PBI_PROTO80 */

/* Define to emulate the 1400XL/1450XLD. */
/* #undef PBI_XLD */

/* Target: Sony PlayStation 2. */
/* #undef PS2 */

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to use R: device. */
/* #undef R_IO_DEVICE */

/* Define to use IP network connection with the R: device. */
/* #undef R_NETWORK */

/* Define to use the host serial port with the R: device. */
/* #undef R_SERIAL */

/* Target: SDL library. */
/* #undef SDL */

/* Define to the type of arg 1 for `select'. */
#define SELECT_TYPE_ARG1 int

/* Define to the type of args 2, 3 and 4 for `select'. */
#define SELECT_TYPE_ARG234 (fd_set *)

/* Define to the type of arg 5 for `select'. */
#define SELECT_TYPE_ARG5 (struct timeval *)

/* Define to allow serial in/out sound. */
/* #undef SERIO_SOUND */

/* Target: X11 with shared memory extensions. */
/* #undef SHM */

/* Define to activate sound support. */
/* #undef SOUND */
#define SIGNED_SAMPLES 1
#define POKEYSND_SIGNED_SAMPLES 1
#define SOUND 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to allow stereo sound. */
/* #undef STEREO_SOUND */
#define STEREO_SOUND 1

/* Can change video modes on the fly. */
/* #undef SUPPORTS_CHANGE_VIDEOMODE */

/* Save additional config file options. */
/* #undef SUPPORTS_PLATFORM_CONFIGSAVE */

/* Additional config file options. */
/* #undef SUPPORTS_PLATFORM_CONFIGURE */

/* Update the Palette if it changed. */
/* #undef SUPPORTS_PLATFORM_PALETTEUPDATE */

/* Platform-specific sleep function. */
/* #undef SUPPORTS_PLATFORM_SLEEP */

/* Can display the screen rotated sideways. */
/* #undef SUPPORTS_ROTATE_VIDEOMODE */

/* Reinitialise the sound system. */
/* #undef SUPPORTS_SOUND_REINIT */

/* Define to use synchronized sound. */
/* #undef SYNCHRONIZED_SOUND */

/* Alternate system-wide config file for non-Unix OS. */
/* #undef SYSTEM_WIDE_CFG_FILE */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Target: Curses-compatible library. */
/* #undef USE_CURSES */

/* Define for using cursor/ctrl keys for keyboard joystick. */
/* #undef USE_CURSORBLOCK */

/* Target: Ncurses library. */
/* #undef USE_NCURSES */

/* Define to use very slow computer support (faster -refresh). */
/* #undef VERY_SLOW */

/* Define to emulate the Alien Group Voice Box. */
/* #undef VOICEBOX */

/* Define to allow volume only sound. */
/* #undef VOL_ONLY_SOUND */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define if unaligned word access is ok. */
#define WORDS_UNALIGNED_OK 1

/* Target: Standard X11. */
/* #undef X11 */

/* Emulate the XEP80. */
/* #undef XEP80_EMULATION */

/* Target: X11 with XView. */
/* #undef XVIEW */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */
