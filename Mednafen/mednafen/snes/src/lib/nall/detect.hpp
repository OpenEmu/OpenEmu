#ifndef NALL_DETECT_HPP
#define NALL_DETECT_HPP

/* Compiler detection */

#if defined(__GNUC__)
  #define COMPILER_GCC
#elif defined(_MSC_VER)
  #define COMPILER_VISUALC
#endif

/* Platform detection */

#if defined(_WIN32)
  #define PLATFORM_WIN
#elif defined(__APPLE__)
  #define PLATFORM_OSX
#elif defined(linux) || defined(__sun__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #define PLATFORM_X
#endif

/* Endian detection */

#if defined(__i386__) || defined(__amd64__) || defined(_M_IX86) || defined(_M_AMD64)
  #define ARCH_LSB
#elif defined(__powerpc__) || defined(_M_PPC) || defined(__BIG_ENDIAN__)
  #define ARCH_MSB
#endif

#endif
