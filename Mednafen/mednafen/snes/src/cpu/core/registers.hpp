struct flag_t {
  bool n, v, m, x, d, i, z, c;

  inline operator unsigned() const {
    return (n << 7) + (v << 6) + (m << 5) + (x << 4)
         + (d << 3) + (i << 2) + (z << 1) + (c << 0);
  }

  inline unsigned operator=(uint8 data) {
    n = data & 0x80; v = data & 0x40; m = data & 0x20; x = data & 0x10;
    d = data & 0x08; i = data & 0x04; z = data & 0x02; c = data & 0x01;
    return data;
  }

  inline unsigned operator|=(unsigned data) { return operator=(operator unsigned() | data); }
  inline unsigned operator^=(unsigned data) { return operator=(operator unsigned() ^ data); }
  inline unsigned operator&=(unsigned data) { return operator=(operator unsigned() & data); }

  flag_t() : n(0), v(0), m(0), x(0), d(0), i(0), z(0), c(0) {}
};

struct reg16_t {
  union {
    uint16 w;
    struct { uint8 order_lsb2(l, h); };
  };

  inline operator unsigned() const { return w; }
  inline unsigned operator   = (unsigned i) { return w   = i; }
  inline unsigned operator  |= (unsigned i) { return w  |= i; }
  inline unsigned operator  ^= (unsigned i) { return w  ^= i; }
  inline unsigned operator  &= (unsigned i) { return w  &= i; }
  inline unsigned operator <<= (unsigned i) { return w <<= i; }
  inline unsigned operator >>= (unsigned i) { return w >>= i; }
  inline unsigned operator  += (unsigned i) { return w  += i; }
  inline unsigned operator  -= (unsigned i) { return w  -= i; }
  inline unsigned operator  *= (unsigned i) { return w  *= i; }
  inline unsigned operator  /= (unsigned i) { return w  /= i; }
  inline unsigned operator  %= (unsigned i) { return w  %= i; }

  reg16_t() : w(0) {}
};

struct reg24_t {
  union {
    uint32 d;
    struct { uint16 order_lsb2(w, wh); };
    struct { uint8  order_lsb4(l, h, b, bh); };
  };

  inline operator unsigned() const { return d; }
  inline unsigned operator   = (unsigned i) { return d = uclip<24>(i); }
  inline unsigned operator  |= (unsigned i) { return d = uclip<24>(d  | i); }
  inline unsigned operator  ^= (unsigned i) { return d = uclip<24>(d  ^ i); }
  inline unsigned operator  &= (unsigned i) { return d = uclip<24>(d  & i); }
  inline unsigned operator <<= (unsigned i) { return d = uclip<24>(d << i); }
  inline unsigned operator >>= (unsigned i) { return d = uclip<24>(d >> i); }
  inline unsigned operator  += (unsigned i) { return d = uclip<24>(d  + i); }
  inline unsigned operator  -= (unsigned i) { return d = uclip<24>(d  - i); }
  inline unsigned operator  *= (unsigned i) { return d = uclip<24>(d  * i); }
  inline unsigned operator  /= (unsigned i) { return d = uclip<24>(d  / i); }
  inline unsigned operator  %= (unsigned i) { return d = uclip<24>(d  % i); }

  reg24_t() : d(0) {}
};

struct regs_t {
  reg24_t pc;
  reg16_t r[6], &a, &x, &y, &z, &s, &d;
  flag_t p;
  uint8 db;
  bool e;

  bool irq;   //IRQ pin (0 = low, 1 = trigger)
  bool wai;   //raised during wai, cleared after interrupt triggered
  uint8 mdr;  //memory data register

  regs_t() : a(r[0]), x(r[1]), y(r[2]), z(r[3]), s(r[4]), d(r[5]), db(0), e(false), irq(false), wai(false), mdr(0) {
    z = 0;
  }
};
