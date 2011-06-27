#ifndef NALL_ANY_HPP
#define NALL_ANY_HPP

#include <typeinfo>
#include <nall/static.hpp>
#include <nall/traits.hpp>

namespace nall {
  class any {
  public:
    bool empty() const { return container; }
    const std::type_info& type() const { return container ? container->type() : typeid(void); }

    template<typename T> any& operator=(const T& value_) {
      typedef typename static_if<
        is_array<T>::value,
        typename remove_extent<typename add_const<T>::type>::type*,
        T
      >::type auto_t;

      if(type() == typeid(auto_t)) {
        static_cast<holder<auto_t>*>(container)->value = (auto_t)value_;
      } else {
        if(container) delete container;
        container = new holder<auto_t>((auto_t)value_);
      }

      return *this;
    }

    any() : container(0) {}
    template<typename T> any(const T& value_) : container(0) { operator=(value_); }

  private:
    struct placeholder {
      virtual const std::type_info& type() const = 0;
    } *container;

    template<typename T> struct holder : placeholder {
      T value;
      const std::type_info& type() const { return typeid(T); }
      holder(const T& value_) : value(value_) {}
    };

    template<typename T> friend T any_cast(any&);
    template<typename T> friend T any_cast(const any&);
    template<typename T> friend T* any_cast(any*);
    template<typename T> friend const T* any_cast(const any*);
  };

  template<typename T> T any_cast(any &value) {
    typedef typename remove_reference<T>::type nonref;
    if(value.type() != typeid(nonref)) throw;
    return static_cast<any::holder<nonref>*>(value.container)->value;
  }

  template<typename T> T any_cast(const any &value) {
    typedef const typename remove_reference<T>::type nonref;
    if(value.type() != typeid(nonref)) throw;
    return static_cast<any::holder<nonref>*>(value.container)->value;
  }

  template<typename T> T* any_cast(any *value) {
    if(!value || value->type() != typeid(T)) return 0;
    return &static_cast<any::holder<T>*>(value->container)->value;
  }

  template<typename T> const T* any_cast(const any *value) {
    if(!value || value->type() != typeid(T)) return 0;
    return &static_cast<any::holder<T>*>(value->container)->value;
  }
}

#endif
