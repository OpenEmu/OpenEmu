#ifndef NALL_BIT_HPP
#define NALL_BIT_HPP

#include <nall/stdint.hpp>

namespace nall {
  template<unsigned bits>
  inline uintmax_t uclamp(const uintmax_t x) {
    enum : uintmax_t { b = 1ull << (bits - 1), y = b * 2 - 1 };
    return y + ((x - y) & -(x < y));  //min(x, y);
  }

  template<unsigned bits>
  inline uintmax_t uclip(const uintmax_t x) {
    enum : uintmax_t { b = 1ull << (bits - 1), m = b * 2 - 1 };
    return (x & m);
  }

  template<unsigned bits>
  inline intmax_t sclamp(const intmax_t x) {
    enum : intmax_t { b = 1ull << (bits - 1), m = b - 1 };
    return (x > m) ? m : (x < -b) ? -b : x;
  }

  template<unsigned bits>
  inline intmax_t sclip(const intmax_t x) {
    enum : uintmax_t { b = 1ull << (bits - 1), m = b * 2 - 1 };
    return ((x & m) ^ b) - b;
  }

  namespace bit {
    constexpr inline uintmax_t mask(const char *s, uintmax_t sum = 0) {
      return (
        *s == '0' || *s == '1' ? mask(s + 1, (sum << 1) | 1) :
        *s == ' ' || *s == '_' ? mask(s + 1, sum) :
        *s ? mask(s + 1, sum << 1) :
        sum
      );
    }

    constexpr inline uintmax_t test(const char *s, uintmax_t sum = 0) {
      return (
        *s == '0' || *s == '1' ? test(s + 1, (sum << 1) | (*s - '0')) :
        *s == ' ' || *s == '_' ? test(s + 1, sum) :
        *s ? test(s + 1, sum << 1) :
        sum
      );
    }

    //lowest(0b1110) == 0b0010
    constexpr inline uintmax_t lowest(const uintmax_t x) {
      return x & -x;
    }

    //clear_lowest(0b1110) == 0b1100
    constexpr inline uintmax_t clear_lowest(const uintmax_t x) {
      return x & (x - 1);
    }

    //set_lowest(0b0101) == 0b0111
    constexpr inline uintmax_t set_lowest(const uintmax_t x) {
      return x | (x + 1);
    }

    //count number of bits set in a byte
    inline unsigned count(uintmax_t x) {
      unsigned count = 0;
      do count += x & 1; while(x >>= 1);
      return count;
    }

    //round up to next highest single bit:
    //round(15) == 16, round(16) == 16, round(17) == 32
    inline uintmax_t round(uintmax_t x) {
      if((x & (x - 1)) == 0) return x;
      while(x & (x - 1)) x &= x - 1;
      return x << 1;
    }
  }
}

#endif
