#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

struct UTF8 {
  unsigned size;       //size of encoded codepoint
  uint64_t data;       //encoded codepoint
  unsigned codepoint;  //decoded codepoint
};

inline UTF8 utf8_read(const char *s) {
  UTF8 utf8;

       if((*s & 0xfe) == 0xfc) utf8.size = 6;
  else if((*s & 0xfc) == 0xf8) utf8.size = 5;
  else if((*s & 0xf8) == 0xf0) utf8.size = 4;
  else if((*s & 0xf0) == 0xe0) utf8.size = 3;
  else if((*s & 0xe0) == 0xc0) utf8.size = 2;
  else                         utf8.size = 1;

  utf8.data = 0;
  for(unsigned n = 0; n < utf8.size; n++) {
    utf8.data = (utf8.data << 8) | (uint8_t)s[n];
  }

  static uint8_t mask[] = { 0, 0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
  utf8.codepoint = s[0] & mask[utf8.size];
  for(unsigned n = 1; n < utf8.size; n++) {
    utf8.codepoint = (utf8.codepoint << 6) | (s[n] & 0x3f);
  }

  return utf8;
}

inline void utf8_write(char *s, const UTF8 &utf8) {
  for(signed n = utf8.size - 1, shift = 0; n >= 0; n--, shift += 8) {
    s[n] = utf8.data >> shift;
  }
}

}

#endif
