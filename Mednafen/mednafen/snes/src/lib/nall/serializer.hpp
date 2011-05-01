#ifndef NALL_SERIALIZER_HPP
#define NALL_SERIALIZER_HPP

#include <nall/stdint.hpp>
#include <nall/traits.hpp>
#include <nall/utility.hpp>

namespace nall {

  //serializer: a class designed to save and restore the state of classes.
  //
  //benefits:
  //- data() will be portable in size (it is not necessary to specify type sizes.)
  //- data() will be portable in endianness (always stored internally as little-endian.)
  //- one serialize function can both save and restore class states.
  //
  //caveats:
  //- only plain-old-data can be stored. complex classes must provide serialize(serializer&);
  //- floating-point usage is not portable across platforms

  class serializer {
  public:
    enum mode_t { Load, Save, Size };

    mode_t mode() const {
      return imode;
    }

    const uint8_t* data() const {
      return idata;
    }

    unsigned size() const {
      return isize;
    }

    unsigned capacity() const {
      return icapacity;
    }

    template<typename T> void floatingpoint(T &value) {
      enum { size = sizeof(T) };
      //this is rather dangerous, and not cross-platform safe;
      //but there is no standardized way to export FP-values
      uint8_t *p = (uint8_t*)&value;
      if(imode == Save) {
        for(unsigned n = 0; n < size; n++) idata[isize++] = p[n];
      } else if(imode == Load) {
        for(unsigned n = 0; n < size; n++) p[n] = idata[isize++];
      } else {
        isize += size;
      }
    }

    template<typename T> void integer(T &value) {
      enum { size = is_bool<T>::value ? 1 : sizeof(T) };
      if(imode == Save) {
        for(unsigned n = 0; n < size; n++) idata[isize++] = value >> (n << 3);
      } else if(imode == Load) {
        value = 0;
        for(unsigned n = 0; n < size; n++) value |= idata[isize++] << (n << 3);
      } else if(imode == Size) {
        isize += size;
      }
    }

    template<typename T> void array(T &array) {
      enum { size = sizeof(T) / sizeof(typename remove_extent<T>::type) };
      for(unsigned n = 0; n < size; n++) integer(array[n]);
    }

    template<typename T> void array(T array, unsigned size) {
      for(unsigned n = 0; n < size; n++) integer(array[n]);
    }

    serializer& operator=(const serializer &s) {
      if(idata) delete[] idata;

      imode = s.imode;
      idata = new uint8_t[s.icapacity];
      isize = s.isize;
      icapacity = s.icapacity;

      memcpy(idata, s.idata, s.icapacity);
      return *this;
    }

    serializer(const serializer &s) : idata(0) {
      operator=(s);
    }

    serializer() {
      imode = Size;
      idata = 0;
      isize = 0;
    }

    serializer(unsigned capacity) {
      imode = Save;
      idata = new uint8_t[capacity]();
      isize = 0;
      icapacity = capacity;
    }

    serializer(const uint8_t *data, unsigned capacity) {
      imode = Load;
      idata = new uint8_t[capacity];
      isize = 0;
      icapacity = capacity;
      memcpy(idata, data, capacity);
    }

    ~serializer() {
      if(idata) delete[] idata;
    }

  private:
    mode_t imode;
    uint8_t *idata;
    unsigned isize;
    unsigned icapacity;
  };

};

#endif
