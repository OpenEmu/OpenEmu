#ifndef NALL_Cpp98_HPP
#define NALL_Cpp98_HPP

#include <cstddef>

namespace std {
  template <typename T, T v> struct integral_constant
  { static const T value = v;
    typedef T value_type;
    typedef integral_constant<T, v> type;
  };
  typedef integral_constant <bool, true>  true_type;
  typedef integral_constant <bool, false> false_type;
  template <typename>             struct is_array        : public false_type { };
  template <typename T, size_t n> struct is_array <T[n]> : public true_type  { };
  template <typename T>           struct is_array <T[ ]> : public true_type  { };
  template <typename T>           struct remove_extent        { typedef T type; };
  template <typename T, size_t n> struct remove_extent <T[n]> { typedef T type; };
  template <typename T>           struct remove_extent <T[ ]> { typedef T type; };
  template <typename T> struct add_const { typedef T const type; };
  template <typename> struct remove_reference;
  template <typename, typename> struct is_same       : public false_type { };
  template <typename T>         struct is_same<T, T> : public true_type  { };
  template <typename T> class initializer_list{
  public:
    typedef T         value_type;
    typedef T const & reference;
    typedef T const & const_reference;
    typedef size_t    size_type;
    typedef T const * iterator;
    typedef T const * const_iterator;
  private:
    iterator  _M_array;
    size_type _M_len;
    initializer_list(const_iterator __a, size_type __l) : _M_array(__a), _M_len(__l) { }
  public:
    initializer_list() : _M_array(NULL), _M_len(0) { }
    size_type      size () const { return _M_len; }
    const_iterator begin() const { return _M_array; }
    const_iterator end  () const { return begin() + size(); }
  };
}

#endif

