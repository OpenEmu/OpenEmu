#ifndef NALL_VECTOR_HPP
#define NALL_VECTOR_HPP

#include <algorithm>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>
#include <nall/algorithm.hpp>
#include <nall/bit.hpp>
#include <nall/sort.hpp>
#include <nall/utility.hpp>

namespace nall {
  template<typename T> struct vector {
    struct exception_out_of_bounds{};

  protected:
    T *pool;
    unsigned poolsize;
    unsigned objectsize;

  public:
    operator bool() const { return pool; }
    T* data() { return pool; }

    bool empty() const { return pool == nullptr; }
    unsigned size() const { return objectsize; }
    unsigned capacity() const { return poolsize; }

    void reset() {
      if(pool) {
        for(unsigned n = 0; n < objectsize; n++) pool[n].~T();
        free(pool);
      }
      pool = nullptr;
      poolsize = 0;
      objectsize = 0;
    }

    void reserve(unsigned size) {
      size = bit::round(size);  //amortize growth
      T *copy = (T*)calloc(size, sizeof(T));
      for(unsigned n = 0; n < min(size, objectsize); n++) new(copy + n) T(pool[n]);
      for(unsigned n = 0; n < objectsize; n++) pool[n].~T();
      free(pool);
      pool = copy;
      poolsize = size;
      objectsize = min(size, objectsize);
    }

    //requires trivial constructor
    void resize(unsigned size) {
      if(size == objectsize) return;
      if(size < objectsize) return reserve(size);
      while(size > objectsize) append(T());
    }

    template<typename... Args>
    void append(const T& data, Args&&... args) {
      append(data);
      append(std::forward<Args>(args)...);
    }

    void append(const T& data) {
      if(objectsize + 1 > poolsize) reserve(objectsize + 1);
      new(pool + objectsize++) T(data);
    }

    bool appendonce(const T& data) {
      if(find(data) == true) return false;
      append(data);
      return true;
    }

    void insert(unsigned position, const T& data) {
      append(data);
      for(signed n = size() - 1; n > position; n--) pool[n] = pool[n - 1];
      pool[position] = data;
    }

    void prepend(const T& data) {
      insert(0, data);
    }

    void remove(unsigned index = ~0u, unsigned count = 1) {
      if(index == ~0) index = objectsize ? objectsize - 1 : 0;
      for(unsigned n = index; count + n < objectsize; n++) pool[n] = pool[count + n];
      objectsize = (count + index >= objectsize) ? index : objectsize - count;
    }

    T take(unsigned index = ~0u) {
      if(index == ~0) index = objectsize ? objectsize - 1 : 0;
      if(index >= objectsize) throw exception_out_of_bounds();
      T item = pool[index];
      remove(index);
      return item;
    }

    void sort() {
      nall::sort(pool, objectsize);
    }

    template<typename Comparator> void sort(const Comparator &lessthan) {
      nall::sort(pool, objectsize, lessthan);
    }

    optional<unsigned> find(const T& data) {
      for(unsigned n = 0; n < size(); n++) if(pool[n] == data) return { true, n };
      return { false, 0u };
    }

    T& first() {
      if(objectsize == 0) throw exception_out_of_bounds();
      return pool[0];
    }

    T& last() {
      if(objectsize == 0) throw exception_out_of_bounds();
      return pool[objectsize - 1];
    }

    //access
    inline T& operator[](unsigned position) {
      if(position >= objectsize) throw exception_out_of_bounds();
      return pool[position];
    }

    inline const T& operator[](unsigned position) const {
      if(position >= objectsize) throw exception_out_of_bounds();
      return pool[position];
    }

    inline T& operator()(unsigned position) {
      if(position >= poolsize) reserve(position + 1);
      while(position >= objectsize) append(T());
      return pool[position];
    }

    inline const T& operator()(unsigned position, const T& data) const {
      if(position >= objectsize) return data;
      return pool[position];
    }

    //iteration
    T* begin() { return &pool[0]; }
    T* end() { return &pool[objectsize]; }
    const T* begin() const { return &pool[0]; }
    const T* end() const { return &pool[objectsize]; }

    //copy
    inline vector& operator=(const vector &source) {
      reset();
      reserve(source.capacity());
      for(auto &data : source) append(data);
      return *this;
    }

    vector(const vector &source) : pool(nullptr), poolsize(0), objectsize(0) {
      operator=(source);
    }

    //move
    inline vector& operator=(vector &&source) {
      reset();
      pool = source.pool, poolsize = source.poolsize, objectsize = source.objectsize;
      source.pool = nullptr, source.poolsize = 0, source.objectsize = 0;
      return *this;
    }

    vector(vector &&source) : pool(nullptr), poolsize(0), objectsize(0) {
      operator=(std::move(source));
    }

    //construction
    vector() : pool(nullptr), poolsize(0), objectsize(0) {
    }

    vector(std::initializer_list<T> list) : pool(nullptr), poolsize(0), objectsize(0) {
      for(auto &data : list) append(data);
    }

    ~vector() {
      reset();
    }
  };

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

  #define NALL_DEPRECATED
  #if defined(NALL_DEPRECATED)
  template<typename T> struct linear_vector {
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
      pool = nullptr;
      poolsize = 0;
      objectsize = 0;
    }

    void reserve(unsigned newsize) {
      newsize = bit::round(newsize);  //round to nearest power of two (for amortized growth)

      T *poolcopy = (T*)calloc(newsize, sizeof(T));
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

    void append(const T data) {
      if(objectsize + 1 > poolsize) reserve(objectsize + 1);
      new(pool + objectsize++) T(data);
    }

    template<typename U> void insert(unsigned index, const U list) {
      linear_vector<T> merged;
      for(unsigned i = 0; i < index; i++) merged.append(pool[i]);
      for(auto &item : list) merged.append(item);
      for(unsigned i = index; i < objectsize; i++) merged.append(pool[i]);
      operator=(merged);
    }

    void insert(unsigned index, const T item) {
      insert(index, linear_vector<T>{ item });
    }

    void remove(unsigned index, unsigned count = 1) {
      for(unsigned i = index; count + i < objectsize; i++) {
        pool[i] = pool[count + i];
      }
      if(count + index >= objectsize) resize(index);  //every element >= index was removed
      else resize(objectsize - count);
    }

    linear_vector() : pool(nullptr), poolsize(0), objectsize(0) {
    }

    linear_vector(std::initializer_list<T> list) : pool(nullptr), poolsize(0), objectsize(0) {
      for(const T *p = list.begin(); p != list.end(); ++p) append(*p);
    }

    ~linear_vector() {
      reset();
    }

    //copy
    inline linear_vector<T>& operator=(const linear_vector<T> &source) {
      reset();
      reserve(source.capacity());
      resize(source.size());
      for(unsigned i = 0; i < source.size(); i++) operator[](i) = source.operator[](i);
      return *this;
    }

    linear_vector(const linear_vector<T> &source) : pool(nullptr), poolsize(0), objectsize(0) {
      operator=(source);
    }

    //move
    inline linear_vector<T>& operator=(linear_vector<T> &&source) {
      reset();
      pool = source.pool;
      poolsize = source.poolsize;
      objectsize = source.objectsize;
      source.pool = nullptr;
      source.reset();
      return *this;
    }

    linear_vector(linear_vector<T> &&source) : pool(nullptr), poolsize(0), objectsize(0) {
      operator=(std::move(source));
    }

    //index
    inline T& operator[](unsigned index) {
      if(index >= objectsize) resize(index + 1);
      return pool[index];
    }

    inline const T& operator[](unsigned index) const {
      if(index >= objectsize) throw "vector[] out of bounds";
      return pool[index];
    }

    //iteration
    T* begin() { return &pool[0]; }
    T* end() { return &pool[objectsize]; }
    const T* begin() const { return &pool[0]; }
    const T* end() const { return &pool[objectsize]; }
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

  template<typename T> struct pointer_vector {
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
      pool = nullptr;
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

    void append(const T data) {
      if(objectsize + 1 > poolsize) reserve(objectsize + 1);
      pool[objectsize++] = new T(data);
    }

    template<typename U> void insert(unsigned index, const U list) {
      pointer_vector<T> merged;
      for(unsigned i = 0; i < index; i++) merged.append(*pool[i]);
      for(auto &item : list) merged.append(item);
      for(unsigned i = index; i < objectsize; i++) merged.append(*pool[i]);
      operator=(merged);
    }

    void insert(unsigned index, const T item) {
      insert(index, pointer_vector<T>{ item });
    }

    void remove(unsigned index, unsigned count = 1) {
      for(unsigned i = index; count + i < objectsize; i++) {
        *pool[i] = *pool[count + i];
      }
      if(count + index >= objectsize) resize(index);  //every element >= index was removed
      else resize(objectsize - count);
    }

    pointer_vector() : pool(nullptr), poolsize(0), objectsize(0) {
    }

    pointer_vector(std::initializer_list<T> list) : pool(nullptr), poolsize(0), objectsize(0) {
      for(const T *p = list.begin(); p != list.end(); ++p) append(*p);
    }

    ~pointer_vector() {
      reset();
    }

    //copy
    inline pointer_vector<T>& operator=(const pointer_vector<T> &source) {
      reset();
      reserve(source.capacity());
      resize(source.size());
      for(unsigned i = 0; i < source.size(); i++) operator[](i) = source.operator[](i);
      return *this;
    }

    pointer_vector(const pointer_vector<T> &source) : pool(nullptr), poolsize(0), objectsize(0) {
      operator=(source);
    }

    //move
    inline pointer_vector<T>& operator=(pointer_vector<T> &&source) {
      reset();
      pool = source.pool;
      poolsize = source.poolsize;
      objectsize = source.objectsize;
      source.pool = nullptr;
      source.reset();
      return *this;
    }

    pointer_vector(pointer_vector<T> &&source) : pool(nullptr), poolsize(0), objectsize(0) {
      operator=(std::move(source));
    }

    //index
    inline T& operator[](unsigned index) {
      if(index >= objectsize) resize(index + 1);
      if(!pool[index]) pool[index] = new T;
      return *pool[index];
    }

    inline const T& operator[](unsigned index) const {
      if(index >= objectsize || !pool[index]) throw "vector[] out of bounds";
      return *pool[index];
    }

    //iteration
    struct iterator {
      bool operator!=(const iterator &source) const { return index != source.index; }
      T& operator*() { return vector.operator[](index); }
      iterator& operator++() { index++; return *this; }
      iterator(const pointer_vector &vector, unsigned index) : vector(vector), index(index) {}
    private:
      const pointer_vector &vector;
      unsigned index;
    };

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, objectsize); }
    const iterator begin() const { return iterator(*this, 0); }
    const iterator end() const { return iterator(*this, objectsize); }
  };
  #endif
}

#endif
