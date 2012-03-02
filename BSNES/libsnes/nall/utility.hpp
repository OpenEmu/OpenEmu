#ifndef NALL_UTILITY_HPP
#define NALL_UTILITY_HPP

#include <nall/C++98.hpp>

namespace nall {
  template<bool C, typename T = bool> struct enable_if { typedef T type; };
  template<typename T> struct enable_if<false, T> {};
  template<typename C, typename T = bool> struct mp_enable_if : enable_if<C::value, T> {};

  template<typename T> inline void swap(T & x, T & y) {
    T tmp = x;
    x     = y;
    y     = tmp;
  }

  template<typename T> struct base_from_member {
    T value;
    base_from_member(T value_) : value(value_) {}
  };

  template<typename T> class optional {
    bool valid;
    T value;
  public:
    inline operator bool() const { return valid; }
    inline const T& operator()() const { if(!valid) throw; return value; }
    inline optional(bool valid, const T &value) : valid(valid), value(value) {}
  };

  template<typename T> inline T* allocate(unsigned size, const T &value) {
    T *array = new T[size];
    for(unsigned i = 0; i < size; i++) array[i] = value;
    return array;
  }
}

#endif
