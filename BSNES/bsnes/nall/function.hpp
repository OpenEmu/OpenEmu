#ifndef NALL_FUNCTION_HPP
#define NALL_FUNCTION_HPP

namespace nall {
  template<typename T> class function;

  template<typename R, typename... P> class function<R (P...)> {
    struct container {
      virtual R operator()(P... p) const = 0;
      virtual container* copy() const = 0;
      virtual ~container() {}
    } *callback;

    struct global : container {
      R (*function)(P...);
      R operator()(P... p) const { return function(std::forward<P>(p)...); }
      container* copy() const { return new global(function); }
      global(R (*function)(P...)) : function(function) {}
    };

    template<typename C> struct member : container {
      R (C::*function)(P...);
      C *object;
      R operator()(P... p) const { return (object->*function)(std::forward<P>(p)...); }
      container* copy() const { return new member(function, object); }
      member(R (C::*function)(P...), C *object) : function(function), object(object) {}
    };

    template<typename L> struct lambda : container {
      mutable L object;
      R operator()(P... p) const { return object(std::forward<P>(p)...); }
      container* copy() const { return new lambda(object); }
      lambda(const L& object) : object(object) {}
    };

  public:
    operator bool() const { return callback; }
    R operator()(P... p) const { return (*callback)(std::forward<P>(p)...); }
    void reset() { if(callback) { delete callback; callback = nullptr; } }

    function& operator=(const function &source) {
      if(this != &source) {
        if(callback) { delete callback; callback = nullptr; }
        if(source.callback) callback = source.callback->copy();
      }
      return *this;
    }

    function(const function &source) : callback(nullptr) { operator=(source); }
    function() : callback(nullptr) {}
    function(void *function) : callback(nullptr) { if(function) callback = new global((R (*)(P...))function); }
    function(R (*function)(P...)) { callback = new global(function); }
    template<typename C> function(R (C::*function)(P...), C *object) { callback = new member<C>(function, object); }
    template<typename C> function(R (C::*function)(P...) const, C *object) { callback = new member<C>((R (C::*)(P...))function, object); }
    template<typename L> function(const L& object) { callback = new lambda<L>(object); }
    ~function() { if(callback) delete callback; }
  };
}

#endif
