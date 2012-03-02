#ifndef NALL_BASE64_HPP
#define NALL_BASE64_HPP

#include <string.h>
#include <nall/stdint.hpp>

namespace nall {
  class base64 {
  public:
    static bool encode(char *&output, const uint8_t* input, unsigned inlength) {
      output = new char[inlength * 8 / 6 + 6]();

      unsigned i = 0, o = 0;
      while(i < inlength) {
        switch(i % 3) {
          case 0: {
            output[o++] = enc(input[i] >> 2);
            output[o] = enc((input[i] & 3) << 4);
          } break;

          case 1: {
            uint8_t prev = dec(output[o]);
            output[o++] = enc(prev + (input[i] >> 4));
            output[o] = enc((input[i] & 15) << 2);
          } break;

          case 2: {
            uint8_t prev = dec(output[o]);
            output[o++] = enc(prev + (input[i] >> 6));
            output[o++] = enc(input[i] & 63);
          } break;
        }

        i++;
      }

      return true;
    }

    static bool decode(uint8_t *&output, unsigned &outlength, const char *input) {
      unsigned inlength = strlen(input), infix = 0;
      output = new uint8_t[inlength]();

      unsigned i = 0, o = 0;
      while(i < inlength) {
        uint8_t x = dec(input[i]);

        switch(i++ & 3) {
          case 0: {
            output[o] = x << 2;
          } break;

          case 1: {
            output[o++] |= x >> 4;
            output[o] = (x & 15) << 4;
          } break;

          case 2: {
            output[o++] |= x >> 2;
            output[o] = (x & 3) << 6;
          } break;

          case 3: {
            output[o++] |= x;
          } break;
        }
      }

      outlength = o;
      return true;
    }

  private:
    static char enc(uint8_t n) {
      static char lookup_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
      return lookup_table[n & 63];
    }

    static uint8_t dec(char n) {
      if(n >= 'A' && n <= 'Z') return n - 'A';
      if(n >= 'a' && n <= 'z') return n - 'a' + 26;
      if(n >= '0' && n <= '9') return n - '0' + 52;
      if(n == '-') return 62;
      if(n == '_') return 63;
      return 0;
    }
  };
}

#endif
