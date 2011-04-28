#ifndef NALL_TRAITS_HPP
#define NALL_TRAITS_HPP

namespace nall {
  //==
  //is
  //==

  template<typename T> struct is_integral { enum { value = false }; };
  template<> struct is_integral<bool> { enum { value = true }; };
  template<> struct is_integral<char> { enum { value = true }; };
  template<> struct is_integral<signed char> { enum { value = true }; };
  template<> struct is_integral<unsigned char> { enum { value = true }; };
  template<> struct is_integral<wchar_t> { enum { value = true }; };
  template<> struct is_integral<short> { enum { value = true }; };
  template<> struct is_integral<unsigned short> { enum { value = true }; };
  template<> struct is_integral<long> { enum { value = true }; };
  template<> struct is_integral<unsigned long> { enum { value = true }; };
  template<> struct is_integral<long long> { enum { value = true }; };
  template<> struct is_integral<unsigned long long> { enum { value = true }; };
  template<> struct is_integral<int> { enum { value = true }; };
  template<> struct is_integral<unsigned int> { enum { value = true }; };

  template<typename T> struct is_floating_point { enum { value = false }; };
  template<> struct is_floating_point<float> { enum { value = true }; };
  template<> struct is_floating_point<double> { enum { value = true }; };
  template<> struct is_floating_point<long double> { enum { value = true }; };

  template<typename T> struct is_bool { enum { value = false }; };
  template<> struct is_bool<bool> { enum { value = true }; };

  template<typename T> struct is_void { enum { value = false }; };
  template<> struct is_void<void> { enum { value = true }; };

  template<typename T> struct is_arithmetic {
    enum { value = is_integral<T>::value || is_floating_point<T>::value };
  };

  template<typename T> struct is_fundamental {
    enum { value = is_integral<T>::value || is_floating_point<T>::value || is_void<T>::value };
  };

  template<typename T> struct is_compound {
    enum { value = !is_fundamental<T>::value };
  };

  template<typename T> struct is_array { enum { value = false }; };
  template<typename T> struct is_array<T[]> { enum { value = true }; };
  template<typename T, int N> struct is_array<T[N]> { enum { value = true }; };

  template<typename T> struct is_const { enum { value = false }; };
  template<typename T> struct is_const<const T> { enum { value = true }; };
  template<typename T> struct is_const<const T&> { enum { value = true }; };

  template<typename T> struct is_pointer { enum { value = false }; };
  template<typename T> struct is_pointer<T*> { enum { value = true }; };

  template<typename T> struct is_reference { enum { value = false }; };
  template<typename T> struct is_reference<T&> { enum { value = true }; };

  template<typename T, typename U> struct is_same { enum { value = false }; };
  template<typename T> struct is_same<T, T> { enum { value = true }; };

  //===
  //add
  //===

  template<typename T> struct add_const { typedef const T type; };
  template<typename T> struct add_const<const T> { typedef const T type; };
  template<typename T> struct add_const<const T&> { typedef const T& type; };

  template<typename T> struct add_pointer { typedef T* type; };
  template<typename T> struct add_pointer<T*> { typedef T** type; };

  template<typename T> struct add_reference { typedef T& type; };
  template<typename T> struct add_reference<T&> { typedef T& type; };

  //======
  //remove
  //======

  template<typename T> struct remove_const { typedef T type; };
  template<typename T> struct remove_const<const T> { typedef T type; };
  template<typename T> struct remove_const<const T&> { typedef T type; };

  template<typename T> struct remove_extent { typedef T type; };
  template<typename T> struct remove_extent<T[]> { typedef T type; };
  template<typename T, int N> struct remove_extent<T[N]> { typedef T type; };

  template<typename T> struct remove_pointer { typedef T type; };
  template<typename T> struct remove_pointer<T*> { typedef T type; };

  template<typename T> struct remove_reference { typedef T type; };
  template<typename T> struct remove_reference<T&> { typedef T type; };
}

#endif
