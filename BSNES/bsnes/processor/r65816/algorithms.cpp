inline void R65816::op_adc_b() {
  int result;

  if(!regs.p.d) {
    result = regs.a.l + rd.l + regs.p.c;
  } else {
    result = (regs.a.l & 0x0f) + (rd.l & 0x0f) + (regs.p.c << 0);
    if(result > 0x09) result += 0x06;
    regs.p.c = result > 0x0f;
    result = (regs.a.l & 0xf0) + (rd.l & 0xf0) + (regs.p.c << 4) + (result & 0x0f);
  }

  regs.p.v = ~(regs.a.l ^ rd.l) & (regs.a.l ^ result) & 0x80;
  if(regs.p.d && result > 0x9f) result += 0x60;
  regs.p.c = result > 0xff;
  regs.p.n = result & 0x80;
  regs.p.z = (uint8_t)result == 0;

  regs.a.l = result;
}

inline void R65816::op_adc_w() {
  int result;

  if(!regs.p.d) {
    result = regs.a.w + rd.w + regs.p.c;
  } else {
    result = (regs.a.w & 0x000f) + (rd.w & 0x000f) + (regs.p.c <<  0);
    if(result > 0x0009) result += 0x0006;
    regs.p.c = result > 0x000f;
    result = (regs.a.w & 0x00f0) + (rd.w & 0x00f0) + (regs.p.c <<  4) + (result & 0x000f);
    if(result > 0x009f) result += 0x0060;
    regs.p.c = result > 0x00ff;
    result = (regs.a.w & 0x0f00) + (rd.w & 0x0f00) + (regs.p.c <<  8) + (result & 0x00ff);
    if(result > 0x09ff) result += 0x0600;
    regs.p.c = result > 0x0fff;
    result = (regs.a.w & 0xf000) + (rd.w & 0xf000) + (regs.p.c << 12) + (result & 0x0fff);
  }

  regs.p.v = ~(regs.a.w ^ rd.w) & (regs.a.w ^ result) & 0x8000;
  if(regs.p.d && result > 0x9fff) result += 0x6000;
  regs.p.c = result > 0xffff;
  regs.p.n = result & 0x8000;
  regs.p.z = (uint16_t)result == 0;

  regs.a.w = result;
}

inline void R65816::op_and_b() {
  regs.a.l &= rd.l;
  regs.p.n = regs.a.l & 0x80;
  regs.p.z = regs.a.l == 0;
}

inline void R65816::op_and_w() {
  regs.a.w &= rd.w;
  regs.p.n = regs.a.w & 0x8000;
  regs.p.z = regs.a.w == 0;
}

inline void R65816::op_bit_b() {
  regs.p.n = rd.l & 0x80;
  regs.p.v = rd.l & 0x40;
  regs.p.z = (rd.l & regs.a.l) == 0;
}

inline void R65816::op_bit_w() {
  regs.p.n = rd.w & 0x8000;
  regs.p.v = rd.w & 0x4000;
  regs.p.z = (rd.w & regs.a.w) == 0;
}

inline void R65816::op_cmp_b() {
  int r = regs.a.l - rd.l;
  regs.p.n = r & 0x80;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
}

inline void R65816::op_cmp_w() {
  int r = regs.a.w - rd.w;
  regs.p.n = r & 0x8000;
  regs.p.z = (uint16)r == 0;
  regs.p.c = r >= 0;
}

inline void R65816::op_cpx_b() {
  int r = regs.x.l - rd.l;
  regs.p.n = r & 0x80;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
}

inline void R65816::op_cpx_w() {
  int r = regs.x.w - rd.w;
  regs.p.n = r & 0x8000;
  regs.p.z = (uint16)r == 0;
  regs.p.c = r >= 0;
}

inline void R65816::op_cpy_b() {
  int r = regs.y.l - rd.l;
  regs.p.n = r & 0x80;
  regs.p.z = (uint8)r == 0;
  regs.p.c = r >= 0;
}

inline void R65816::op_cpy_w() {
  int r = regs.y.w - rd.w;
  regs.p.n = r & 0x8000;
  regs.p.z = (uint16)r == 0;
  regs.p.c = r >= 0;
}

inline void R65816::op_eor_b() {
  regs.a.l ^= rd.l;
  regs.p.n = regs.a.l & 0x80;
  regs.p.z = regs.a.l == 0;
}

inline void R65816::op_eor_w() {
  regs.a.w ^= rd.w;
  regs.p.n = regs.a.w & 0x8000;
  regs.p.z = regs.a.w == 0;
}

inline void R65816::op_lda_b() {
  regs.a.l = rd.l;
  regs.p.n = regs.a.l & 0x80;
  regs.p.z = regs.a.l == 0;
}

inline void R65816::op_lda_w() {
  regs.a.w = rd.w;
  regs.p.n = regs.a.w & 0x8000;
  regs.p.z = regs.a.w == 0;
}

inline void R65816::op_ldx_b() {
  regs.x.l = rd.l;
  regs.p.n = regs.x.l & 0x80;
  regs.p.z = regs.x.l == 0;
}

inline void R65816::op_ldx_w() {
  regs.x.w = rd.w;
  regs.p.n = regs.x.w & 0x8000;
  regs.p.z = regs.x.w == 0;
}

inline void R65816::op_ldy_b() {
  regs.y.l = rd.l;
  regs.p.n = regs.y.l & 0x80;
  regs.p.z = regs.y.l == 0;
}

inline void R65816::op_ldy_w() {
  regs.y.w = rd.w;
  regs.p.n = regs.y.w & 0x8000;
  regs.p.z = regs.y.w == 0;
}

inline void R65816::op_ora_b() {
  regs.a.l |= rd.l;
  regs.p.n = regs.a.l & 0x80;
  regs.p.z = regs.a.l == 0;
}

inline void R65816::op_ora_w() {
  regs.a.w |= rd.w;
  regs.p.n = regs.a.w & 0x8000;
  regs.p.z = regs.a.w == 0;
}

inline void R65816::op_sbc_b() {
  int result;
  rd.l ^= 0xff;

  if(!regs.p.d) {
    result = regs.a.l + rd.l + regs.p.c;
  } else {
    result = (regs.a.l & 0x0f) + (rd.l & 0x0f) + (regs.p.c << 0);
    if(result <= 0x0f) result -= 0x06;
    regs.p.c = result > 0x0f;
    result = (regs.a.l & 0xf0) + (rd.l & 0xf0) + (regs.p.c << 4) + (result & 0x0f);
  }

  regs.p.v = ~(regs.a.l ^ rd.l) & (regs.a.l ^ result) & 0x80;
  if(regs.p.d && result <= 0xff) result -= 0x60;
  regs.p.c = result > 0xff;
  regs.p.n = result & 0x80;
  regs.p.z = (uint8_t)result == 0;

  regs.a.l = result;
}

inline void R65816::op_sbc_w() {
  int result;
  rd.w ^= 0xffff;

  if(!regs.p.d) {
    result = regs.a.w + rd.w + regs.p.c;
  } else {
    result = (regs.a.w & 0x000f) + (rd.w & 0x000f) + (regs.p.c <<  0);
    if(result <= 0x000f) result -= 0x0006;
    regs.p.c = result > 0x000f;
    result = (regs.a.w & 0x00f0) + (rd.w & 0x00f0) + (regs.p.c <<  4) + (result & 0x000f);
    if(result <= 0x00ff) result -= 0x0060;
    regs.p.c = result > 0x00ff;
    result = (regs.a.w & 0x0f00) + (rd.w & 0x0f00) + (regs.p.c <<  8) + (result & 0x00ff);
    if(result <= 0x0fff) result -= 0x0600;
    regs.p.c = result > 0x0fff;
    result = (regs.a.w & 0xf000) + (rd.w & 0xf000) + (regs.p.c << 12) + (result & 0x0fff);
  }

  regs.p.v = ~(regs.a.w ^ rd.w) & (regs.a.w ^ result) & 0x8000;
  if(regs.p.d && result <= 0xffff) result -= 0x6000;
  regs.p.c = result > 0xffff;
  regs.p.n = result & 0x8000;
  regs.p.z = (uint16_t)result == 0;

  regs.a.w = result;
}

inline void R65816::op_inc_b() {
  rd.l++;
  regs.p.n = rd.l & 0x80;
  regs.p.z = rd.l == 0;
}

inline void R65816::op_inc_w() {
  rd.w++;
  regs.p.n = rd.w & 0x8000;
  regs.p.z = rd.w == 0;
}

inline void R65816::op_dec_b() {
  rd.l--;
  regs.p.n = rd.l & 0x80;
  regs.p.z = rd.l == 0;
}

inline void R65816::op_dec_w() {
  rd.w--;
  regs.p.n = rd.w & 0x8000;
  regs.p.z = rd.w == 0;
}

inline void R65816::op_asl_b() {
  regs.p.c = rd.l & 0x80;
  rd.l <<= 1;
  regs.p.n = rd.l & 0x80;
  regs.p.z = rd.l == 0;
}

inline void R65816::op_asl_w() {
  regs.p.c = rd.w & 0x8000;
  rd.w <<= 1;
  regs.p.n = rd.w & 0x8000;
  regs.p.z = rd.w == 0;
}

inline void R65816::op_lsr_b() {
  regs.p.c = rd.l & 1;
  rd.l >>= 1;
  regs.p.n = rd.l & 0x80;
  regs.p.z = rd.l == 0;
}

inline void R65816::op_lsr_w() {
  regs.p.c = rd.w & 1;
  rd.w >>= 1;
  regs.p.n = rd.w & 0x8000;
  regs.p.z = rd.w == 0;
}

inline void R65816::op_rol_b() {
  unsigned carry = (unsigned)regs.p.c;
  regs.p.c = rd.l & 0x80;
  rd.l = (rd.l << 1) | carry;
  regs.p.n = rd.l & 0x80;
  regs.p.z = rd.l == 0;
}

inline void R65816::op_rol_w() {
  unsigned carry = (unsigned)regs.p.c;
  regs.p.c = rd.w & 0x8000;
  rd.w = (rd.w << 1) | carry;
  regs.p.n = rd.w & 0x8000;
  regs.p.z = rd.w == 0;
}

inline void R65816::op_ror_b() {
  unsigned carry = (unsigned)regs.p.c << 7;
  regs.p.c = rd.l & 1;
  rd.l = carry | (rd.l >> 1);
  regs.p.n = rd.l & 0x80;
  regs.p.z = rd.l == 0;
}

inline void R65816::op_ror_w() {
  unsigned carry = (unsigned)regs.p.c << 15;
  regs.p.c = rd.w & 1;
  rd.w = carry | (rd.w >> 1);
  regs.p.n = rd.w & 0x8000;
  regs.p.z = rd.w == 0;
}

inline void R65816::op_trb_b() {
  regs.p.z = (rd.l & regs.a.l) == 0;
  rd.l &= ~regs.a.l;
}

inline void R65816::op_trb_w() {
  regs.p.z = (rd.w & regs.a.w) == 0;
  rd.w &= ~regs.a.w;
}

inline void R65816::op_tsb_b() {
  regs.p.z = (rd.l & regs.a.l) == 0;
  rd.l |= regs.a.l;
}

inline void R65816::op_tsb_w() {
  regs.p.z = (rd.w & regs.a.w) == 0;
  rd.w |= regs.a.w;
}
