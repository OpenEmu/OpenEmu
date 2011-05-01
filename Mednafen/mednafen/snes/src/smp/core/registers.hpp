struct regya_t {
  uint8_t &hi, &lo;

  inline operator uint16_t() const {
    return (hi << 8) + lo;
  }

  inline regya_t& operator=(uint16_t data) {
    hi = data >> 8;
    lo = data;
    return *this;
  }

  regya_t(uint8_t &hi_, uint8_t &lo_) : hi(hi_), lo(lo_) {}
};

struct flag_t {
  bool n, v, p, b, h, i, z, c;

  inline operator unsigned() const {
    return (n << 7) + (v << 6) + (p << 5) + (b << 4)
         + (h << 3) + (i << 2) + (z << 1) + (c << 0);
  }

  inline unsigned operator=(uint8_t data) {
    n = data & 0x80; v = data & 0x40; p = data & 0x20; b = data & 0x10;
    h = data & 0x08; i = data & 0x04; z = data & 0x02; c = data & 0x01;
    return data;
  }

  inline unsigned operator|=(unsigned data) { return operator=(operator unsigned() | data); }
  inline unsigned operator^=(unsigned data) { return operator=(operator unsigned() ^ data); }
  inline unsigned operator&=(unsigned data) { return operator=(operator unsigned() & data); }

  flag_t() : n(0), v(0), p(0), b(0), h(0), i(0), z(0), c(0) {}
};

struct regs_t {
  uint16_t pc;
  uint8_t r[4], &a, &x, &y, &sp;
  regya_t ya;
  flag_t p;
  regs_t() : a(r[0]), x(r[1]), y(r[2]), sp(r[3]), ya(r[2], r[0]) {}
};
