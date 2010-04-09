#ifdef SMPCORE_CPP

uint8 SMPcore::op_adc(uint8 x, uint8 y) {
  int r = x + y + regs.p.c;
  regs.p.n = r & 0x80;
  regs.p.v = ~(x ^ y) & (x ^ r) & 0x80;
  regs.p.h = (x ^ y ^ r) & 0x10;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r > 0xff;
  return r;
}

uint16 SMPcore::op_addw(uint16 x, uint16 y) {
  uint16 r;
  regs.p.c = 0;
  r  = op_adc(x, y);
  r |= op_adc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}

uint8 SMPcore::op_and(uint8 x, uint8 y) {
  x &= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_cmp(uint8 x, uint8 y) {
  int r = x - y;
  regs.p.n = r & 0x80;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint16 SMPcore::op_cmpw(uint16 x, uint16 y) {
  int r = x - y;
  regs.p.n = r & 0x8000;
  regs.p.z = (uint16)r == 0;
  regs.p.c = r >= 0;
  return x;
}

uint8 SMPcore::op_eor(uint8 x, uint8 y) {
  x ^= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_or(uint8 x, uint8 y) {
  x |= y;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_sbc(uint8 x, uint8 y) {
  int r = x - y - !regs.p.c;
  regs.p.n = r & 0x80;
  regs.p.v = (x ^ y) & (x ^ r) & 0x80;
  regs.p.h = !((x ^ y ^ r) & 0x10);
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
  return r;
}

uint16 SMPcore::op_subw(uint16 x, uint16 y) {
  uint16 r;
  regs.p.c = 1;
  r  = op_sbc(x, y);
  r |= op_sbc(x >> 8, y >> 8) << 8;
  regs.p.z = r == 0;
  return r;
}

uint8 SMPcore::op_inc(uint8 x) {
  x++;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_dec(uint8 x) {
  x--;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_asl(uint8 x) {
  regs.p.c = x & 0x80;
  x <<= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_lsr(uint8 x) {
  regs.p.c = x & 0x01;
  x >>= 1;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_rol(uint8 x) {
  unsigned carry = (unsigned)regs.p.c;
  regs.p.c = x & 0x80;
  x = (x << 1) | carry;
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

uint8 SMPcore::op_ror(uint8 x) {
  unsigned carry = (unsigned)regs.p.c << 7;
  regs.p.c = x & 0x01;
  x = carry | (x >> 1);
  regs.p.n = x & 0x80;
  regs.p.z = x == 0;
  return x;
}

#endif
