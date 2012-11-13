template<typename T, unsigned size> struct moduloarray {
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

  moduloarray() {
    buffer = new T[size * 3]();
  }

  ~moduloarray() {
    delete[] buffer;
  }

private:
  T *buffer;
};
