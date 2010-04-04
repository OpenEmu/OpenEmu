#ifndef NALL_PROPERTY_HPP
#define NALL_PROPERTY_HPP

//nall::property implements a variable container that disallows write access
//to non-derived objects. This requires use of property::set(), as C++ lacks
//the ability to make this implementation completely transparent.

namespace nall {
  class property {
  public:
    template<typename T> class property_t;

  protected:
    template<typename T> T& get(property_t<T>&);
    template<typename T> property_t<T>& set(property_t<T>&, const T);

  public:
    template<typename T>
    class property_t {
    public:
      const T& operator()() const { return value; }
      property_t() : value() {}
      property_t(const T value_) : value(value_) {}

    protected:
      T value;
      operator T&() { return value; }
      property_t& operator=(const T newValue) { value = newValue; return *this; }
      friend T& property::get<T>(property_t<T>&);
      friend property_t<T>& property::set<T>(property_t<T>&, const T);
    };
  };

  template<typename T>
  T& property::get(property::property_t<T> &p) {
    return p.operator T&();
  }

  template<typename T>
  property::property_t<T>& property::set(property::property_t<T> &p, const T value) {
    return p.operator=(value);
  }
}

#endif
