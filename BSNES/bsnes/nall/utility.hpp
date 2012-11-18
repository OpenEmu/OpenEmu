#ifndef NALL_UTILITY_HPP
#define NALL_UTILITY_HPP

#include <type_traits>
#include <utility>

namespace nall {
  template<typename T> struct base_from_member {
    T value;
    base_from_member(T value_) : value(value_) {}
  };

  template<typename T> class optional {
  public:
    bool valid;
    T value;
  public:
    inline operator bool() const { return valid; }
    inline const T& operator()() const { if(!valid) throw; return value; }
    inline optional<T>& operator=(const optional<T> &source) { valid = source.valid; value = source.value; return *this; }
    inline optional() : valid(false) {}
    inline optional(bool valid, const T &value) : valid(valid), value(value) {}
  };

  template<typename T> inline T* allocate(unsigned size, const T &value) {
    T *array = new T[size];
    for(unsigned i = 0; i < size; i++) array[i] = value;
    return array;
  }
}

#endif
