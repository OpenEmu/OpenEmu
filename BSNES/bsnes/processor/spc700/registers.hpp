struct flag_t {
  bool n, v, p, b, h, i, z, c;

  inline operator unsigned() const {
    return (n << 7) | (v << 6) | (p << 5) | (b << 4)
         | (h << 3) | (i << 2) | (z << 1) | (c << 0);
  }

  inline unsigned operator=(uint8 data) {
    n = data & 0x80; v = data & 0x40; p = data & 0x20; b = data & 0x10;
    h = data & 0x08; i = data & 0x04; z = data & 0x02; c = data & 0x01;
    return data;
  }

  inline unsigned operator|=(uint8 data) { return operator=(operator unsigned() | data); }
  inline unsigned operator^=(uint8 data) { return operator=(operator unsigned() ^ data); }
  inline unsigned operator&=(uint8 data) { return operator=(operator unsigned() & data); }
};

struct word_t {
  union {
    uint16 w;
    struct { uint8 order_lsb2(l, h); };
  };

  inline operator unsigned() const { return w; }
  inline unsigned operator=(unsigned data) { return w = data; }

  inline unsigned operator++() { return ++w; }
  inline unsigned operator--() { return --w; }

  inline unsigned operator++(int) { unsigned data = w++; return data; }
  inline unsigned operator--(int) { unsigned data = w--; return data; }

  inline unsigned operator+=(unsigned data) { return w += data;; }
  inline unsigned operator-=(unsigned data) { return w -= data;; }

  inline unsigned operator|=(unsigned data) { return w |= data; }
  inline unsigned operator^=(unsigned data) { return w ^= data; }
  inline unsigned operator&=(unsigned data) { return w &= data; }
};

struct regs_t {
  word_t pc;
  union {
    uint16 ya;
    struct { uint8 order_lsb2(a, y); };
  };
  uint8 x, s;
  flag_t p;
};
