#ifndef NALL_LZSS_HPP
#define NALL_LZSS_HPP

#include <nall/array.hpp>
#include <nall/new.hpp>
#include <nall/stdint.hpp>

namespace nall {
  class lzss {
  public:
    static bool encode(uint8_t *&output, unsigned &outlength, const uint8_t *input, unsigned inlength) {
      output = new(zeromemory) uint8_t[inlength * 9 / 8 + 9];

      unsigned i = 0, o = 0;
      while(i < inlength) {
        unsigned flagoffset = o++;
        uint8_t flag = 0x00;

        for(unsigned b = 0; b < 8 && i < inlength; b++) {
          unsigned longest = 0, pointer;
          for(unsigned index = 1; index < 4096; index++) {
            unsigned count = 0;
            while(true) {
              if(count >= 15 + 3) break;                               //verify pattern match is not longer than max length
              if(i + count >= inlength) break;                         //verify pattern match does not read past end of input
              if(i + count < index) break;                             //verify read is not before start of input
              if(input[i + count] != input[i + count - index]) break;  //verify pattern still matches
              count++;
            }

            if(count > longest) {
              longest = count;
              pointer = index;
            }
          }

          if(longest < 3) output[o++] = input[i++];
          else {
            flag |= 1 << b;
            uint16_t x = ((longest - 3) << 12) + pointer;
            output[o++] = x;
            output[o++] = x >> 8;
            i += longest;
          }
        }

        output[flagoffset] = flag;
      }

      outlength = o;
      return true;
    }

    static bool decode(uint8_t *&output, const uint8_t *input, unsigned length) {
      output = new(zeromemory) uint8_t[length];

      unsigned i = 0, o = 0;
      while(o < length) {
        uint8_t flag = input[i++];

        for(unsigned b = 0; b < 8 && o < length; b++) {
          if(!(flag & (1 << b))) output[o++] = input[i++];
          else {
            uint16_t offset = input[i++];
            offset += input[i++] << 8;
            uint16_t lookuplength = (offset >> 12) + 3;
            offset &= 4095;
            for(unsigned index = 0; index < lookuplength && o + index < length; index++) {
              output[o + index] = output[o + index - offset];
            }
            o += lookuplength;
          }
        }
      }

      return true;
    }
  };
}

#endif
