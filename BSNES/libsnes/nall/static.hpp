#ifndef NALL_STATIC_HPP
#define NALL_STATIC_HPP

namespace nall {
  template<bool C, typename T, typename F> struct static_if { typedef T type; };
  template<typename T, typename F> struct static_if<false, T, F> { typedef F type; };
  template<typename C, typename T, typename F> struct mp_static_if { typedef typename static_if<C::type, T, F>::type type; };

  template<bool A, bool B> struct static_and { enum { value = false }; };
  template<> struct static_and<true, true> { enum { value = true }; };
  template<typename A, typename B> struct mp_static_and { enum { value = static_and<A::value, B::value>::value }; };

  template<bool A, bool B> struct static_or { enum { value = false }; };
  template<> struct static_or<false, true> { enum { value = true }; };
  template<> struct static_or<true, false> { enum { value = true }; };
  template<> struct static_or<true, true> { enum { value = true }; };
  template<typename A, typename B> struct mp_static_or { enum { value = static_or<A::value, B::value>::value }; };
}

#endif
