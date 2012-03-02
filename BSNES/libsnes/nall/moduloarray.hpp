#ifndef NALL_MODULO_HPP
#define NALL_MODULO_HPP

#include <nall/serializer.hpp>

namespace nall {
  template<typename T, int size> class modulo_array {
  public:
    inline T operator[](int index) const {
      return buffer[size + index];
    }

    inline T read(int index) const {
      return buffer[size + index];
    }

    inline void write(unsigned index, const T value) {
      buffer[index] =
      buffer[index + size] =
      buffer[index + size + size] = value;
    }

    void serialize(serializer &s) {
      s.array(buffer, size * 3);
    }

    modulo_array() {
      buffer = new T[size * 3]();
    }

    ~modulo_array() {
      delete[] buffer;
    }

  private:
    T *buffer;
  };
}

#endif
