#ifndef NALL_INTRINSICS_HPP
#define NALL_INTRINSICS_HPP

struct Intrinsics {
  enum class Compiler : unsigned { GCC, VisualC, Unknown };
  enum class Platform : unsigned { X, OSX, Windows, Unknown };
  enum class Endian : unsigned { LSB, MSB, Unknown };

  static inline Compiler compiler();
  static inline Platform platform();
  static inline Endian endian();
};

/* Compiler detection */

#if defined(__GNUC__)
  #define COMPILER_GCC
  Intrinsics::Compiler Intrinsics::compiler() { return Intrinsics::Compiler::GCC; }
#elif defined(_MSC_VER)
  #define COMPILER_VISUALC
  Intrinsics::Compiler Intrinsics::compiler() { return Intrinsics::Compiler::VisualC; }
#else
  #warning "unable to detect compiler"
  #define COMPILER_UNKNOWN
  Intrinsics::Compiler Intrinsics::compiler() { return Intrinsics::Compiler::Unknown; }
#endif

/* Platform detection */

#if defined(linux) || defined(__sun__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #define PLATFORM_X
  Intrinsics::Platform Intrinsics::platform() { return Intrinsics::Platform::X; }
#elif defined(__APPLE__)
  #define PLATFORM_OSX
  Intrinsics::Platform Intrinsics::platform() { return Intrinsics::Platform::OSX; }
#elif defined(_WIN32)
  #define PLATFORM_WINDOWS
  #define PLATFORM_WIN
  Intrinsics::Platform Intrinsics::platform() { return Intrinsics::Platform::Windows; }
#else
  #warning "unable to detect platform"
  #define PLATFORM_UNKNOWN
  Intrinsics::Platform Intrinsics::platform() { return Intrinsics::Platform::Unknown; }
#endif

/* Endian detection */

#if defined(__i386__) || defined(__amd64__) || defined(_M_IX86) || defined(_M_AMD64)
  #define ENDIAN_LSB
  #define ARCH_LSB
  Intrinsics::Endian Intrinsics::endian() { return Intrinsics::Endian::LSB; }
#elif defined(__powerpc__) || defined(_M_PPC) || defined(__BIG_ENDIAN__)
  #define ENDIAN_MSB
  #define ARCH_MSB
  Intrinsics::Endian Intrinsics::endian() { return Intrinsics::Endian::MSB; }
#else
  #warning "unable to detect endian"
  #define ENDIAN_UNKNOWN
  #define ARCH_UNKNOWN
  Intrinsics::Endian Intrinsics::endian() { return Intrinsics::Endian::Unknown; }
#endif

#endif
