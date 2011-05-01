#ifndef NALL_ARRAY_HPP
#define NALL_ARRAY_HPP

#include <stdlib.h>
#include <nall/algorithm.hpp>
#include <nall/bit.hpp>

namespace nall {
  //dynamic vector array
  //neither constructor nor destructor is ever invoked;
  //thus, this should only be used for POD objects.
  template<typename T> class array {
  protected:
    T *pool;
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

      pool = (T*)realloc(pool, newsize * sizeof(T));
      poolsize = newsize;
      buffersize = min(buffersize, newsize);
    }

    void resize(unsigned newsize) {
      if(newsize > poolsize) reserve(bit::round(newsize));  //round reserve size up to power of 2
      buffersize = newsize;
    }

    T* get(unsigned minsize = 0) {
      if(minsize > buffersize) resize(minsize);
      if(minsize > buffersize) throw "array[] out of bounds";
      return pool;
    }

    void add(const T data) {
      operator[](buffersize) = data;
    }

    signed find(const T data) {
      for(unsigned i = 0; i < size(); i++) if(pool[i] == data) return i;
      return -1;  //not found
    }

    void clear() {
      memset(pool, 0, buffersize * sizeof(T));
    }

    array() {
      pool = 0;
      poolsize = 0;
      buffersize = 0;
    }

    ~array() { reset(); }

    array(const array &source) : pool(0) {
      operator=(source);
    }

    array& operator=(const array &source) {
      if(pool) free(pool);
      buffersize = source.buffersize;
      poolsize = source.poolsize;
      pool = (T*)malloc(sizeof(T) * poolsize);            //allocate entire pool size,
      memcpy(pool, source.pool, sizeof(T) * buffersize);  //... but only copy used pool objects
      return *this;
    }

    inline T& operator[](unsigned index) {
      if(index >= buffersize) resize(index + 1);
      if(index >= buffersize) throw "array[] out of bounds";
      return pool[index];
    }

    inline const T& operator[](unsigned index) const {
      if(index >= buffersize) throw "array[] out of bounds";
      return pool[index];
    }
  };
}

#endif
