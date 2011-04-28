#ifndef NALL_VECTOR_HPP
#define NALL_VECTOR_HPP

#include <new>
#include <nall/algorithm.hpp>
#include <nall/bit.hpp>
#include <nall/utility.hpp>

namespace nall {
  //linear_vector
  //memory: O(capacity * 2)
  //
  //linear_vector uses placement new + manual destructor calls to create a
  //contiguous block of memory for all objects. accessing individual elements
  //is fast, though resizing the array incurs significant overhead.
  //reserve() overhead is reduced from quadratic time to amortized constant time
  //by resizing twice as much as requested.
  //
  //if objects hold memory address references to themselves (introspection), a
  //valid copy constructor will be needed to keep pointers valid.

  template<typename T> class linear_vector : noncopyable {
  protected:
    T *pool;
    unsigned poolsize, objectsize;

  public:
    unsigned size() const { return objectsize; }
    unsigned capacity() const { return poolsize; }

    void reset() {
      if(pool) {
        for(unsigned i = 0; i < objectsize; i++) pool[i].~T();
        free(pool);
      }
      pool = 0;
      poolsize = 0;
      objectsize = 0;
    }

    void reserve(unsigned newsize) {
      newsize = bit::round(newsize);  //round to nearest power of two (for amortized growth)

      T *poolcopy = (T*)malloc(newsize * sizeof(T));
      for(unsigned i = 0; i < min(objectsize, newsize); i++) new(poolcopy + i) T(pool[i]);
      for(unsigned i = 0; i < objectsize; i++) pool[i].~T();
      free(pool);
      pool = poolcopy;
      poolsize = newsize;
      objectsize = min(objectsize, newsize);
    }

    void resize(unsigned newsize) {
      if(newsize > poolsize) reserve(newsize);

      if(newsize < objectsize) {
        //vector is shrinking; destroy excess objects
        for(unsigned i = newsize; i < objectsize; i++) pool[i].~T();
      } else if(newsize > objectsize) {
        //vector is expanding; allocate new objects
        for(unsigned i = objectsize; i < newsize; i++) new(pool + i) T;
      }

      objectsize = newsize;
    }

    void add(const T data) {
      if(objectsize + 1 > poolsize) reserve(objectsize + 1);
      new(pool + objectsize++) T(data);
    }

    inline T& operator[](unsigned index) {
      if(index >= objectsize) resize(index + 1);
      return pool[index];
    }

    inline const T& operator[](unsigned index) const {
      if(index >= objectsize) throw "vector[] out of bounds";
      return pool[index];
    }

    linear_vector() : pool(0), poolsize(0), objectsize(0) {}
    ~linear_vector() { reset(); }
  };

  //pointer_vector
  //memory: O(1)
  //
  //pointer_vector keeps an array of pointers to each vector object. this adds
  //significant overhead to individual accesses, but allows for optimal memory
  //utilization.
  //
  //by guaranteeing that the base memory address of each objects never changes,
  //this avoids the need for an object to have a valid copy constructor.

  template<typename T> class pointer_vector : noncopyable {
  protected:
    T **pool;
    unsigned poolsize, objectsize;

  public:
    unsigned size() const { return objectsize; }
    unsigned capacity() const { return poolsize; }

    void reset() {
      if(pool) {
        for(unsigned i = 0; i < objectsize; i++) { if(pool[i]) delete pool[i]; }
        free(pool);
      }
      pool = 0;
      poolsize = 0;
      objectsize = 0;
    }

    void reserve(unsigned newsize) {
      newsize = bit::round(newsize);  //round to nearest power of two (for amortized growth)

      for(unsigned i = newsize; i < objectsize; i++) {
        if(pool[i]) { delete pool[i]; pool[i] = 0; }
      }

      pool = (T**)realloc(pool, newsize * sizeof(T*));
      for(unsigned i = poolsize; i < newsize; i++) pool[i] = 0;
      poolsize = newsize;
      objectsize = min(objectsize, newsize);
    }

    void resize(unsigned newsize) {
      if(newsize > poolsize) reserve(newsize);

      for(unsigned i = newsize; i < objectsize; i++) {
        if(pool[i]) { delete pool[i]; pool[i] = 0; }
      }

      objectsize = newsize;
    }

    void add(const T data) {
      if(objectsize + 1 > poolsize) reserve(objectsize + 1);
      pool[objectsize++] = new T(data);
    }

    inline T& operator[](unsigned index) {
      if(index >= objectsize) resize(index + 1);
      if(!pool[index]) pool[index] = new T;
      return *pool[index];
    }

    inline const T& operator[](unsigned index) const {
      if(index >= objectsize || !pool[index]) throw "vector[] out of bounds";
      return *pool[index];
    }

    pointer_vector() : pool(0), poolsize(0), objectsize(0) {}
    ~pointer_vector() { reset(); }
  };

  //default vector type
  template<typename T> class vector : public linear_vector<T> {};
}

#endif
