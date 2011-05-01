#ifndef NALL_UPS_HPP
#define NALL_UPS_HPP

#include <stdio.h>

#include <nall/algorithm.hpp>
#include <nall/crc32.hpp>
#include <nall/file.hpp>
#include <nall/stdint.hpp>

namespace nall {
  class ups {
  public:
    enum result {
      ok,
      patch_unreadable,
      patch_unwritable,
      patch_invalid,
      input_invalid,
      output_invalid,
      patch_crc32_invalid,
      input_crc32_invalid,
      output_crc32_invalid,
    };

    ups::result create(const char *patch_fn, const uint8_t *x_data, unsigned x_size, const uint8_t *y_data, unsigned y_size) {
      if(!fp.open(patch_fn, file::mode_write)) return patch_unwritable;

      crc32 = ~0;
      uint32_t x_crc32 = crc32_calculate(x_data, x_size);
      uint32_t y_crc32 = crc32_calculate(y_data, y_size);

      //header
      write('U');
      write('P');
      write('S');
      write('1');
      encptr(x_size);
      encptr(y_size);

      //body
      unsigned max_size = max(x_size, y_size);
      unsigned relative = 0;
      for(unsigned i = 0; i < max_size;) {
        uint8_t x = i < x_size ? x_data[i] : 0x00;
        uint8_t y = i < y_size ? y_data[i] : 0x00;

        if(x == y) {
          i++;
          continue;
        }

        encptr(i++ - relative);
        write(x ^ y);

        while(true) {
          if(i >= max_size) {
            write(0x00);
            break;
          }

          x = i < x_size ? x_data[i] : 0x00;
          y = i < y_size ? y_data[i] : 0x00;
          i++;
          write(x ^ y);
          if(x == y) break;
        }

        relative = i;
      }

      //footer
      for(unsigned i = 0; i < 4; i++) write(x_crc32 >> (i << 3));
      for(unsigned i = 0; i < 4; i++) write(y_crc32 >> (i << 3));
      uint32_t p_crc32 = ~crc32;
      for(unsigned i = 0; i < 4; i++) write(p_crc32 >> (i << 3));

      fp.close();
      return ok;
    }

    ups::result apply(const uint8_t *p_data, unsigned p_size, const uint8_t *x_data, unsigned x_size, uint8_t *&y_data, unsigned &y_size) {
      if(p_size < 18) return patch_invalid;
      p_buffer = p_data;

      crc32 = ~0;

      //header
      if(read() != 'U') return patch_invalid;
      if(read() != 'P') return patch_invalid;
      if(read() != 'S') return patch_invalid;
      if(read() != '1') return patch_invalid;

      unsigned px_size = decptr();
      unsigned py_size = decptr();

      //mirror
      if(x_size != px_size && x_size != py_size) return input_invalid;
      y_size = (x_size == px_size) ? py_size : px_size;
      y_data = new uint8_t[y_size]();

      for(unsigned i = 0; i < x_size && i < y_size; i++) y_data[i] = x_data[i];
      for(unsigned i = x_size; i < y_size; i++) y_data[i] = 0x00;

      //body
      unsigned relative = 0;
      while(p_buffer < p_data + p_size - 12) {
        relative += decptr();

        while(true) {
          uint8_t x = read();
          if(x && relative < y_size) {
            uint8_t y = relative < x_size ? x_data[relative] : 0x00;
            y_data[relative] = x ^ y;
          }
          relative++;
          if(!x) break;
        }
      }

      //footer
      unsigned px_crc32 = 0, py_crc32 = 0, pp_crc32 = 0;
      for(unsigned i = 0; i < 4; i++) px_crc32 |= read() << (i << 3);
      for(unsigned i = 0; i < 4; i++) py_crc32 |= read() << (i << 3);
      uint32_t p_crc32 = ~crc32;
      for(unsigned i = 0; i < 4; i++) pp_crc32 |= read() << (i << 3);

      uint32_t x_crc32 = crc32_calculate(x_data, x_size);
      uint32_t y_crc32 = crc32_calculate(y_data, y_size);

      if(px_size != py_size) {
        if(x_size == px_size && x_crc32 != px_crc32) return input_crc32_invalid;
        if(x_size == py_size && x_crc32 != py_crc32) return input_crc32_invalid;
        if(y_size == px_size && y_crc32 != px_crc32) return output_crc32_invalid;
        if(y_size == py_size && y_crc32 != py_crc32) return output_crc32_invalid;
      } else {
        if(x_crc32 != px_crc32 && x_crc32 != py_crc32) return input_crc32_invalid;
        if(y_crc32 != px_crc32 && y_crc32 != py_crc32) return output_crc32_invalid;
        if(x_crc32 == y_crc32 && px_crc32 != py_crc32) return output_crc32_invalid;
        if(x_crc32 != y_crc32 && px_crc32 == py_crc32) return output_crc32_invalid;
      }

      if(p_crc32 != pp_crc32) return patch_crc32_invalid;
      return ok;
    }

  private:
    file fp;
    uint32_t crc32;
    const uint8_t *p_buffer;

    uint8_t read() {
      uint8_t n = *p_buffer++;
      crc32 = crc32_adjust(crc32, n);
      return n;
    }

    void write(uint8_t n) {
      fp.write(n);
      crc32 = crc32_adjust(crc32, n);
    }

    void encptr(uint64_t offset) {
      while(true) {
        uint64_t x = offset & 0x7f;
        offset >>= 7;
        if(offset == 0) {
          write(0x80 | x);
          break;
        }
        write(x);
        offset--;
      }
    }

    uint64_t decptr() {
      uint64_t offset = 0, shift = 1;
      while(true) {
        uint8_t x = read();
        offset += (x & 0x7f) * shift;
        if(x & 0x80) break;
        shift <<= 7;
        offset += shift;
      }
      return offset;
    }
  };
}

#endif
