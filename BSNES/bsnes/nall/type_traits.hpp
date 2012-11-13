#ifndef NALL_STATIC_HPP
#define NALL_STATIC_HPP

#include <type_traits>

namespace nall {

template<typename T> class has_default_constructor {
  template<signed> class receive_size{};
  template<typename U> static signed sfinae(receive_size<sizeof U()>*);
  template<typename U> static char sfinae(...);

public:
  enum : bool { value = sizeof(sfinae<T>(0)) == sizeof(signed) };
};

template<bool C, typename T = bool> struct enable_if { typedef T type; };
template<typename T> struct enable_if<false, T> {};

template<bool C, typename T, typename F> struct type_if { typedef T type; };
template<typename T, typename F> struct type_if<false, T, F> { typedef F type; };

template<bool A, bool B> struct static_and { enum { value = false }; };
template<> struct static_and<true, true> { enum { value = true }; };

template<bool A, bool B> struct static_or { enum { value = false }; };
template<> struct static_or<false, true> { enum { value = true }; };
template<> struct static_or<true, false> { enum { value = true }; };
template<> struct static_or<true, true> { enum { value = true }; };

}

#endif
