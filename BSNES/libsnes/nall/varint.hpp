#ifndef NALL_VARINT_HPP
#define NALL_VARINT_HPP

#include <nall/bit.hpp>
#include <nall/static.hpp>

namespace nall {
  template<unsigned bits> class uint_t {
  private:
    unsigned data;

  public:
    inline operator unsigned() const { return data; }
    inline unsigned operator ++(int) { unsigned r = data; data = uclip<bits>(data + 1); return r; }
    inline unsigned operator --(int) { unsigned r = data; data = uclip<bits>(data - 1); return r; }
    inline unsigned operator ++() { return data = uclip<bits>(data + 1); }
    inline unsigned operator --() { return data = uclip<bits>(data - 1); }
    inline unsigned operator  =(const unsigned i) { return data = uclip<bits>(i); }
    inline unsigned operator |=(const unsigned i) { return data = uclip<bits>(data  | i); }
    inline unsigned operator ^=(const unsigned i) { return data = uclip<bits>(data  ^ i); }
    inline unsigned operator &=(const unsigned i) { return data = uclip<bits>(data  & i); }
    inline unsigned operator<<=(const unsigned i) { return data = uclip<bits>(data << i); }
    inline unsigned operator>>=(const unsigned i) { return data = uclip<bits>(data >> i); }
    inline unsigned operator +=(const unsigned i) { return data = uclip<bits>(data  + i); }
    inline unsigned operator -=(const unsigned i) { return data = uclip<bits>(data  - i); }
    inline unsigned operator *=(const unsigned i) { return data = uclip<bits>(data  * i); }
    inline unsigned operator /=(const unsigned i) { return data = uclip<bits>(data  / i); }
    inline unsigned operator %=(const unsigned i) { return data = uclip<bits>(data  % i); }

    inline uint_t() : data(0) {}
    inline uint_t(const unsigned i) : data(uclip<bits>(i)) {}
  };

  template<unsigned bits> class int_t {
    private:
      signed data;

    public:
      inline operator signed() const { return data; }
      inline signed operator ++(int) { signed r = data; data = sclip<bits>(data + 1); return r; }
      inline signed operator --(int) { signed r = data; data = sclip<bits>(data - 1); return r; }
      inline signed operator ++() { return data = sclip<bits>(data + 1); }
      inline signed operator --() { return data = sclip<bits>(data - 1); }
      inline signed operator  =(const signed i) { return data = sclip<bits>(i); }
      inline signed operator |=(const signed i) { return data = sclip<bits>(data  | i); }
      inline signed operator ^=(const signed i) { return data = sclip<bits>(data  ^ i); }
      inline signed operator &=(const signed i) { return data = sclip<bits>(data  & i); }
      inline signed operator<<=(const signed i) { return data = sclip<bits>(data << i); }
      inline signed operator>>=(const signed i) { return data = sclip<bits>(data >> i); }
      inline signed operator +=(const signed i) { return data = sclip<bits>(data  + i); }
      inline signed operator -=(const signed i) { return data = sclip<bits>(data  - i); }
      inline signed operator *=(const signed i) { return data = sclip<bits>(data  * i); }
      inline signed operator /=(const signed i) { return data = sclip<bits>(data  / i); }
      inline signed operator %=(const signed i) { return data = sclip<bits>(data  % i); }

      inline int_t() : data(0) {}
      inline int_t(const signed i) : data(sclip<bits>(i)) {}
  };

  class varuint_t {
    private:
      unsigned data;
      unsigned mask;

    public:
      inline operator unsigned() const { return data; }
      inline unsigned operator ++(int) { unsigned r = data; data = (data + 1) & mask; return r; }
      inline unsigned operator --(int) { unsigned r = data; data = (data - 1) & mask; return r; }
      inline unsigned operator ++() { return data = (data + 1) & mask; }
      inline unsigned operator --() { return data = (data - 1) & mask; }
      inline unsigned operator  =(const unsigned i) { return data = (i) & mask; }
      inline unsigned operator |=(const unsigned i) { return data = (data  | i) & mask; }
      inline unsigned operator ^=(const unsigned i) { return data = (data  ^ i) & mask; }
      inline unsigned operator &=(const unsigned i) { return data = (data  & i) & mask; }
      inline unsigned operator<<=(const unsigned i) { return data = (data << i) & mask; }
      inline unsigned operator>>=(const unsigned i) { return data = (data >> i) & mask; }
      inline unsigned operator +=(const unsigned i) { return data = (data  + i) & mask; }
      inline unsigned operator -=(const unsigned i) { return data = (data  - i) & mask; }
      inline unsigned operator *=(const unsigned i) { return data = (data  * i) & mask; }
      inline unsigned operator /=(const unsigned i) { return data = (data  / i) & mask; }
      inline unsigned operator %=(const unsigned i) { return data = (data  % i) & mask; }

      inline void bits(unsigned bits) { mask = (1U << (bits - 1)) + ((1U << (bits - 1)) - 1); data &= mask; }
      inline varuint_t() : data(0), mask(~0U) {}
      inline varuint_t(const unsigned i) : data(i), mask(~0U) {}
  };

  class varuintmax_t {
    private:
      uintmax_t data;
      uintmax_t mask;

    public:
      inline operator uintmax_t() const { return data; }
      inline uintmax_t operator ++(int) { uintmax_t r = data; data = (data + 1) & mask; return r; }
      inline uintmax_t operator --(int) { uintmax_t r = data; data = (data - 1) & mask; return r; }
      inline uintmax_t operator ++() { return data = (data + 1) & mask; }
      inline uintmax_t operator --() { return data = (data - 1) & mask; }
      inline uintmax_t operator  =(const uintmax_t i) { return data = (i) & mask; }
      inline uintmax_t operator |=(const uintmax_t i) { return data = (data  | i) & mask; }
      inline uintmax_t operator ^=(const uintmax_t i) { return data = (data  ^ i) & mask; }
      inline uintmax_t operator &=(const uintmax_t i) { return data = (data  & i) & mask; }
      inline uintmax_t operator<<=(const uintmax_t i) { return data = (data << i) & mask; }
      inline uintmax_t operator>>=(const uintmax_t i) { return data = (data >> i) & mask; }
      inline uintmax_t operator +=(const uintmax_t i) { return data = (data  + i) & mask; }
      inline uintmax_t operator -=(const uintmax_t i) { return data = (data  - i) & mask; }
      inline uintmax_t operator *=(const uintmax_t i) { return data = (data  * i) & mask; }
      inline uintmax_t operator /=(const uintmax_t i) { return data = (data  / i) & mask; }
      inline uintmax_t operator %=(const uintmax_t i) { return data = (data  % i) & mask; }

      inline void bits(unsigned bits) { mask = (1ULL << (bits - 1)) + ((1ULL << (bits - 1)) - 1); data &= mask; }
      inline varuintmax_t() : data(0), mask(~0ULL) {}
      inline varuintmax_t(const uintmax_t i) : data(i), mask(~0ULL) {}
  };
}

#endif
