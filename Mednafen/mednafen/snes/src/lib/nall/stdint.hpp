#ifndef NALL_STDINT_HPP
#define NALL_STDINT_HPP

#include <nall/static.hpp>

#if defined(_MSC_VER)
  typedef signed char int8_t;
  typedef signed short int16_t;
  typedef signed int int32_t;
  typedef signed long long int64_t;
  typedef int64_t intmax_t;
  #if defined(_WIN64)
  typedef int64_t intptr_t;
  #else
  typedef int32_t intptr_t;
  #endif

  typedef unsigned char uint8_t;
  typedef unsigned short uint16_t;
  typedef unsigned int uint32_t;
  typedef unsigned long long uint64_t;
  typedef uint64_t uintmax_t;
  #if defined(_WIN64)
  typedef uint64_t uintptr_t;
  #else
  typedef uint32_t uintptr_t;
  #endif
#else
  #include <stdint.h>
#endif

namespace nall {
  static static_assert<sizeof(int8_t)   == 1> int8_t_assert;
  static static_assert<sizeof(int16_t)  == 2> int16_t_assert;
  static static_assert<sizeof(int32_t)  == 4> int32_t_assert;
  static static_assert<sizeof(int64_t)  == 8> int64_t_assert;

  static static_assert<sizeof(uint8_t)  == 1> uint8_t_assert;
  static static_assert<sizeof(uint16_t) == 2> uint16_t_assert;
  static static_assert<sizeof(uint32_t) == 4> uint32_t_assert;
  static static_assert<sizeof(uint64_t) == 8> uint64_t_assert;
}

#endif
