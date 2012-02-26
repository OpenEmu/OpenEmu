#ifndef NALL_CONCEPT_HPP
#define NALL_CONCEPT_HPP

#include <nall/static.hpp>
#include <nall/utility.hpp>

namespace nall {
  template<typename T> struct has_count  { enum { value = false }; };
  template<typename T> struct has_length { enum { value = false }; };
  template<typename T> struct has_size   { enum { value = false }; };
  template<typename T> unsigned container_size(const T& object, typename mp_enable_if< has_count <T> >::type = 0) { return object.count();  }
  template<typename T> unsigned container_size(const T& object, typename mp_enable_if< has_length<T> >::type = 0) { return object.length(); }
  template<typename T> unsigned container_size(const T& object, typename mp_enable_if< has_size  <T> >::type = 0) { return object.size();   }

  template<typename T> unsigned container_size(const T& object, typename mp_enable_if< std::is_array<T> >::type = 0) {
    return sizeof(T) / sizeof(typename std::remove_extent<T>::type);
  }
}

#endif
