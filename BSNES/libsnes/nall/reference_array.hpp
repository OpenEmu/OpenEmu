#ifndef NALL_REFERENCE_ARRAY_HPP
#define NALL_REFERENCE_ARRAY_HPP

// Themaister: This is C++0x, but doesn't seem to be used anyways.

#include <type_traits>
#include <nall/bit.hpp>
#include <nall/concept.hpp>

namespace nall {
  template<typename T> struct reference_array {
  protected:
    typedef typename std::remove_reference<T>::type *Tptr;
    Tptr *pool;
    unsigned poolsize, buffersize;

  public:
    unsigned size() const { return buffersize; }
    unsigned capacity() const { return poolsize; }

    void reset() {
      if(pool) free(pool);
      pool = 0;
      poolsize = 0;
      buffersize = 0;
    }

    void reserve(unsigned newsize) {
      if(newsize == poolsize) return;

      pool = (Tptr*)realloc(pool, newsize * sizeof(T));
      poolsize = newsize;
      buffersize = min(buffersize, newsize);
    }

    void resize(unsigned newsize) {
      if(newsize > poolsize) reserve(bit::round(newsize));
      buffersize = newsize;
    }

    void append(const T data) {
      unsigned index = buffersize++;
      if(index >= poolsize) resize(index + 1);
      pool[index] = &data;
    }

    template<typename... Args> reference_array(Args&... args) : pool(0), poolsize(0), buffersize(0) {
      construct(args...);
    }

    ~reference_array() {
      reset();
    }

    reference_array& operator=(const reference_array &source) {
      if(pool) free(pool);
      buffersize = source.buffersize;
      poolsize = source.poolsize;
      pool = (Tptr*)malloc(sizeof(T) * poolsize);
      memcpy(pool, source.pool, sizeof(T) * buffersize);
      return *this;
    }

    reference_array& operator=(const reference_array &&source) {
      if(pool) free(pool);
      pool = source.pool;
      poolsize = source.poolsize;
      buffersize = source.buffersize;
      source.pool = 0;
      source.reset();
      return *this;
    }

    inline T operator[](unsigned index) {
      if(index >= buffersize) throw "reference_array[] out of bounds";
      return *pool[index];
    }

    inline const T operator[](unsigned index) const {
      if(index >= buffersize) throw "reference_array[] out of bounds";
      return *pool[index];
    }

  private:
    void construct() {
    }

    void construct(const reference_array &source) {
      operator=(source);
    }

    void construct(const reference_array &&source) {
      operator=(std::move(source));
    }

    template<typename... Args> void construct(T data, Args&... args) {
      append(data);
      construct(args...);
    }
  };

  template<typename T> struct has_size<reference_array<T>> { enum { value = true }; };
}

#endif
