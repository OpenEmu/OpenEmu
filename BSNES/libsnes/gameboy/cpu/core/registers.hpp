enum {
  A, F, AF,
  B, C, BC,
  D, E, DE,
  H, L, HL,
  SP, PC,
};

enum {
  ZF, NF, HF, CF,
};

//register base class
//the idea here is to have all registers derive from a single base class.
//this allows construction of opcodes that can take any register as input or output,
//despite the fact that behind-the-scenes, special handling is done for eg: F, AF, HL, etc.
//registers can also be chained together: eg af = 0x0000 writes both a and f.
struct Register {
  virtual operator unsigned() const = 0;
  virtual unsigned operator=(unsigned x) = 0;
  Register& operator=(const Register &x) { operator=((unsigned)x); return *this; }

  unsigned operator++(int) { unsigned r = *this; operator=(*this + 1); return r; }
  unsigned operator--(int) { unsigned r = *this; operator=(*this - 1); return r; }
  unsigned operator++() { return operator=(*this + 1); }
  unsigned operator--() { return operator=(*this - 1); }

  unsigned operator |=(unsigned x) { return operator=(*this | x); }
  unsigned operator ^=(unsigned x) { return operator=(*this ^ x); }
  unsigned operator &=(unsigned x) { return operator=(*this & x); }

  unsigned operator<<=(unsigned x) { return operator=(*this << x); }
  unsigned operator>>=(unsigned x) { return operator=(*this >> x); }

  unsigned operator +=(unsigned x) { return operator=(*this + x); }
  unsigned operator -=(unsigned x) { return operator=(*this - x); }
  unsigned operator *=(unsigned x) { return operator=(*this * x); }
  unsigned operator /=(unsigned x) { return operator=(*this / x); }
  unsigned operator %=(unsigned x) { return operator=(*this % x); }
};

struct Register8 : Register {
  uint8 data;
  operator unsigned() const { return data; }
  unsigned operator=(unsigned x) { return data = x; }
};

struct RegisterF : Register {
  bool z, n, h, c;
  operator unsigned() const { return (z << 7) | (n << 6) | (h << 5) | (c << 4); }
  unsigned operator=(unsigned x) { z = x & 0x80; n = x & 0x40; h = x & 0x20; c = x & 0x10; return *this; }
  bool& operator[](unsigned r) {
    static bool* table[] = { &z, &n, &h, &c };
    return *table[r];
  }
};

struct Register16 : Register {
  uint16 data;
  operator unsigned() const { return data; }
  unsigned operator=(unsigned x) { return data = x; }
};

struct RegisterAF : Register {
  Register8 &hi;
  RegisterF &lo;
  operator unsigned() const { return (hi << 8) | (lo << 0); }
  unsigned operator=(unsigned x) { hi = x >> 8; lo = x >> 0; return *this; }
  RegisterAF(Register8 &hi, RegisterF &lo) : hi(hi), lo(lo) {}
};

struct RegisterW : Register {
  Register8 &hi, &lo;
  operator unsigned() const { return (hi << 8) | (lo << 0); }
  unsigned operator=(unsigned x) { hi = x >> 8; lo = x >> 0; return *this; }
  RegisterW(Register8 &hi, Register8 &lo) : hi(hi), lo(lo) {}
};

struct Registers {
  Register8  a;
  RegisterF  f;
  RegisterAF af;
  Register8  b;
  Register8  c;
  RegisterW  bc;
  Register8  d;
  Register8  e;
  RegisterW  de;
  Register8  h;
  Register8  l;
  RegisterW  hl;
  Register16 sp;
  Register16 pc;

  Register& operator[](unsigned r) {
    static Register* table[] = { &a, &f, &af, &b, &c, &bc, &d, &e, &de, &h, &l, &hl, &sp, &pc };
    return *table[r];
  }

  Registers() : af(a, f), bc(b, c), de(d, e), hl(h, l) {}
} r;
