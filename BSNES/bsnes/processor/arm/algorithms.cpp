bool ARM::condition(uint4 condition) {
  switch(condition) {
  case  0: return cpsr().z == 1;                          //EQ (equal)
  case  1: return cpsr().z == 0;                          //NE (not equal)
  case  2: return cpsr().c == 1;                          //CS (carry set)
  case  3: return cpsr().c == 0;                          //CC (carry clear)
  case  4: return cpsr().n == 1;                          //MI (negative)
  case  5: return cpsr().n == 0;                          //PL (positive)
  case  6: return cpsr().v == 1;                          //VS (overflow)
  case  7: return cpsr().v == 0;                          //VC (no overflow)
  case  8: return cpsr().c == 1 && cpsr().z == 0;         //HI (unsigned higher)
  case  9: return cpsr().c == 0 || cpsr().z == 1;         //LS (unsigned lower or same)
  case 10: return cpsr().n == cpsr().v;                   //GE (signed greater than or equal)
  case 11: return cpsr().n != cpsr().v;                   //LT (signed less than)
  case 12: return cpsr().z == 0 && cpsr().n == cpsr().v;  //GT (signed greater than)
  case 13: return cpsr().z == 1 || cpsr().n != cpsr().v;  //LE (signed less than or equal)
  case 14: return true;                                   //AL (always)
  case 15: return false;                                  //NV (never)
  }
}

uint32 ARM::bit(uint32 result) {
  if(cpsr().t || instruction() & (1 << 20)) {
    cpsr().n = result >> 31;
    cpsr().z = result == 0;
    cpsr().c = carryout();
  }
  return result;
}

uint32 ARM::add(uint32 source, uint32 modify, bool carry) {
  uint32 result = source + modify + carry;
  if(cpsr().t || instruction() & (1 << 20)) {
    uint32 overflow = ~(source ^ modify) & (source ^ result);
    cpsr().n = result >> 31;
    cpsr().z = result == 0;
    cpsr().c = (1u << 31) & (overflow ^ source ^ modify ^ result);
    cpsr().v = (1u << 31) & (overflow);
  }
  return result;
}

uint32 ARM::sub(uint32 source, uint32 modify, bool carry) {
  return add(source, ~modify, carry);
}

uint32 ARM::mul(uint32 product, uint32 multiplicand, uint32 multiplier) {
  product += multiplicand * multiplier;

  if(cpsr().t || instruction() & (1 << 20)) {
    cpsr().n = product >> 31;
    cpsr().z = product == 0;
  }

  return product;
}

uint32 ARM::lsl(uint32 source, uint8 shift) {
  carryout() = cpsr().c;
  if(shift == 0) return source;

  carryout() = shift > 32 ? 0 : source & (1 << 32 - shift);
  source     = shift > 31 ? 0 : source << shift;
  return source;
}

uint32 ARM::lsr(uint32 source, uint8 shift) {
  carryout() = cpsr().c;
  if(shift == 0) return source;

  carryout() = shift > 32 ? 0 : source & (1 << shift - 1);
  source     = shift > 31 ? 0 : source >> shift;
  return source;
}

uint32 ARM::asr(uint32 source, uint8 shift) {
  carryout() = cpsr().c;
  if(shift == 0) return source;

  carryout() = shift > 32 ? source & (1 << 31) : source & (1 << shift - 1);
  source     = shift > 31 ? (int32)source >> 31 : (int32)source >> shift;
  return source;
}

uint32 ARM::ror(uint32 source, uint8 shift) {
  carryout() = cpsr().c;
  if(shift == 0) return source;

  if(shift &= 31)
  source     = source << 32 - shift | source >> shift;
  carryout() = source & (1 << 31);
  return source;
}

uint32 ARM::rrx(uint32 source) {
  carryout() = source & 1;
  return (cpsr().c << 31) | (source >> 1);
}
