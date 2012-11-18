#ifndef NALL_UDL_HPP
#define NALL_UDL_HPP

//user-defined literals

#include <nall/atoi.hpp>
#include <type_traits>

namespace nall {
  constexpr inline uintmax_t operator"" _b(const char *n) { return binary(n); }

  //convert to bytes
  constexpr inline uintmax_t operator"" _kb(unsigned long long n) { return 1024 * n; }
  constexpr inline uintmax_t operator"" _mb(unsigned long long n) { return 1024 * 1024 * n; }
  constexpr inline uintmax_t operator"" _gb(unsigned long long n) { return 1024 * 1024 * 1024 * n; }

  //convert to bits
  constexpr inline uintmax_t operator"" _kbit(unsigned long long n) { return 1024 * n / 8; }
  constexpr inline uintmax_t operator"" _mbit(unsigned long long n) { return 1024 * 1024 * n / 8; }
  constexpr inline uintmax_t operator"" _gbit(unsigned long long n) { return 1024 * 1024 * 1024 * n / 8; }

  //convert to hz
  constexpr inline uintmax_t operator"" _khz(long double n) { return n * 1000; }
  constexpr inline uintmax_t operator"" _mhz(long double n) { return n * 1000000; }
  constexpr inline uintmax_t operator"" _ghz(long double n) { return n * 1000000000; }
}

#endif
