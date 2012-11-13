#ifndef NALL_PUBLIC_CAST_HPP
#define NALL_PUBLIC_CAST_HPP

//this is a proof-of-concept-*only* C++ access-privilege elevation exploit.
//this code is 100% legal C++, per C++98 section 14.7.2 paragraph 8:
//"access checking rules do not apply to names in explicit instantiations."
//usage example:

//struct N { typedef void (Class::*)(); };
//template class public_cast<N, &Class::Reference>;
//(class.*public_cast<N>::value);

//Class::Reference may be public, protected or private
//Class::Reference may be a function, object or variable

namespace nall {
  template<typename T, typename T::type... P> struct public_cast;

  template<typename T> struct public_cast<T> {
    static typename T::type value;
  };

  template<typename T> typename T::type public_cast<T>::value;

  template<typename T, typename T::type P> struct public_cast<T, P> {
    static typename T::type value;
  };

  template<typename T, typename T::type P> typename T::type public_cast<T, P>::value = public_cast<T>::value = P;
}

#endif
