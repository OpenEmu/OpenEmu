//$00 stop
void GSU::op_stop() {
  if(regs.cfgr.irq == 0) {
    regs.sfr.irq = 1;
    stop();
  }

  regs.sfr.g = 0;
  regs.pipeline = 0x01;
  regs.reset();
}

//$01 nop
void GSU::op_nop() {
  regs.reset();
}

//$02 cache
void GSU::op_cache() {
  if(regs.cbr != (regs.r[15] & 0xfff0)) {
    regs.cbr = regs.r[15] & 0xfff0;
    cache_flush();
  }
  regs.reset();
}

//$03 lsr
void GSU::op_lsr() {
  regs.sfr.cy = (regs.sr() & 1);
  regs.dr() = regs.sr() >> 1;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$04 rol
void GSU::op_rol() {
  bool carry = (regs.sr() & 0x8000);
  regs.dr() = (regs.sr() << 1) | regs.sfr.cy;
  regs.sfr.s  = (regs.dr() & 0x8000);
  regs.sfr.cy = carry;
  regs.sfr.z  = (regs.dr() == 0);
  regs.reset();
}

//$05 bra e
void GSU::op_bra() {
  regs.r[15] += (int8)pipe();
}

//$06 blt e
void GSU::op_blt() {
  int e = (int8)pipe();
  if((regs.sfr.s ^ regs.sfr.ov) == 0) regs.r[15] += e;
}

//$07 bge e
void GSU::op_bge() {
  int e = (int8)pipe();
  if((regs.sfr.s ^ regs.sfr.ov) == 1) regs.r[15] += e;
}

//$08 bne e
void GSU::op_bne() {
  int e = (int8)pipe();
  if(regs.sfr.z == 0) regs.r[15] += e;
}

//$09 beq e
void GSU::op_beq() {
  int e = (int8)pipe();
  if(regs.sfr.z == 1) regs.r[15] += e;
}

//$0a bpl e
void GSU::op_bpl() {
  int e = (int8)pipe();
  if(regs.sfr.s == 0) regs.r[15] += e;
}

//$0b bmi e
void GSU::op_bmi() {
  int e = (int8)pipe();
  if(regs.sfr.s == 1) regs.r[15] += e;
}

//$0c bcc e
void GSU::op_bcc() {
  int e = (int8)pipe();
  if(regs.sfr.cy == 0) regs.r[15] += e;
}

//$0d bcs e
void GSU::op_bcs() {
  int e = (int8)pipe();
  if(regs.sfr.cy == 1) regs.r[15] += e;
}

//$0e bvc e
void GSU::op_bvc() {
  int e = (int8)pipe();
  if(regs.sfr.ov == 0) regs.r[15] += e;
}

//$0f bvs e
void GSU::op_bvs() {
  int e = (int8)pipe();
  if(regs.sfr.ov == 1) regs.r[15] += e;
}

//$10-1f(b0): to rN
//$10-1f(b1): move rN
template<int n> void GSU::op_to_r() {
  if(regs.sfr.b == 0) {
    regs.dreg = n;
  } else {
    regs.r[n] = regs.sr();
    regs.reset();
  }
}

//$20-2f: with rN
template<int n> void GSU::op_with_r() {
  regs.sreg = n;
  regs.dreg = n;
  regs.sfr.b = 1;
}

//$30-3b(alt0): stw (rN)
template<int n> void GSU::op_stw_ir() {
  regs.ramaddr = regs.r[n];
  rambuffer_write(regs.ramaddr ^ 0, regs.sr() >> 0);
  rambuffer_write(regs.ramaddr ^ 1, regs.sr() >> 8);
  regs.reset();
}

//$30-3b(alt1): stb (rN)
template<int n> void GSU::op_stb_ir() {
  regs.ramaddr = regs.r[n];
  rambuffer_write(regs.ramaddr, regs.sr());
  regs.reset();
}

//$3c loop
void GSU::op_loop() {
  regs.r[12]--;
  regs.sfr.s = (regs.r[12] & 0x8000);
  regs.sfr.z = (regs.r[12] == 0);
  if(!regs.sfr.z) regs.r[15] = regs.r[13];
  regs.reset();
}

//$3d alt1
void GSU::op_alt1() {
  regs.sfr.b = 0;
  regs.sfr.alt1 = 1;
}

//$3e alt2
void GSU::op_alt2() {
  regs.sfr.b = 0;
  regs.sfr.alt2 = 1;
}

//$3f alt3
void GSU::op_alt3() {
  regs.sfr.b = 0;
  regs.sfr.alt1 = 1;
  regs.sfr.alt2 = 1;
}

//$40-4b(alt0): ldw (rN)
template<int n> void GSU::op_ldw_ir() {
  regs.ramaddr = regs.r[n];
  uint16_t data;
  data  = rambuffer_read(regs.ramaddr ^ 0) << 0;
  data |= rambuffer_read(regs.ramaddr ^ 1) << 8;
  regs.dr() = data;
  regs.reset();
}

//$40-4b(alt1): ldb (rN)
template<int n> void GSU::op_ldb_ir() {
  regs.ramaddr = regs.r[n];
  regs.dr() = rambuffer_read(regs.ramaddr);
  regs.reset();
}

//$4c(alt0): plot
void GSU::op_plot() {
  plot(regs.r[1], regs.r[2]);
  regs.r[1]++;
  regs.reset();
}

//$4c(alt1): rpix
void GSU::op_rpix() {
  regs.dr() = rpix(regs.r[1], regs.r[2]);
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$4d: swap
void GSU::op_swap() {
  regs.dr() = (regs.sr() >> 8) | (regs.sr() << 8);
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$4e(alt0): color
void GSU::op_color() {
  regs.colr = color(regs.sr());
  regs.reset();
}

//$4e(alt1): cmode
void GSU::op_cmode() {
  regs.por = regs.sr();
  regs.reset();
}

//$4f: not
void GSU::op_not() {
  regs.dr() = ~regs.sr();
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$50-5f(alt0): add rN
template<int n> void GSU::op_add_r() {
  int r = regs.sr() + regs.r[n];
  regs.sfr.ov = ~(regs.sr() ^ regs.r[n]) & (regs.r[n] ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0x10000);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$50-5f(alt1): adc rN
template<int n> void GSU::op_adc_r() {
  int r = regs.sr() + regs.r[n] + regs.sfr.cy;
  regs.sfr.ov = ~(regs.sr() ^ regs.r[n]) & (regs.r[n] ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0x10000);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$50-5f(alt2): add #N
template<int n> void GSU::op_add_i() {
  int r = regs.sr() + n;
  regs.sfr.ov = ~(regs.sr() ^ n) & (n ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0x10000);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$50-5f(alt3): adc #N
template<int n> void GSU::op_adc_i() {
  int r = regs.sr() + n + regs.sfr.cy;
  regs.sfr.ov = ~(regs.sr() ^ n) & (n ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0x10000);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$60-6f(alt0): sub rN
template<int n> void GSU::op_sub_r() {
  int r = regs.sr() - regs.r[n];
  regs.sfr.ov = (regs.sr() ^ regs.r[n]) & (regs.sr() ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$60-6f(alt1): sbc rN
template<int n> void GSU::op_sbc_r() {
  int r = regs.sr() - regs.r[n] - !regs.sfr.cy;
  regs.sfr.ov = (regs.sr() ^ regs.r[n]) & (regs.sr() ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$60-6f(alt2): sub #N
template<int n> void GSU::op_sub_i() {
  int r = regs.sr() - n;
  regs.sfr.ov = (regs.sr() ^ n) & (regs.sr() ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$60-6f(alt3): cmp rN
template<int n> void GSU::op_cmp_r() {
  int r = regs.sr() - regs.r[n];
  regs.sfr.ov = (regs.sr() ^ regs.r[n]) & (regs.sr() ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.reset();
}

//$70: merge
void GSU::op_merge() {
  regs.dr() = (regs.r[7] & 0xff00) | (regs.r[8] >> 8);
  regs.sfr.ov = (regs.dr() & 0xc0c0);
  regs.sfr.s  = (regs.dr() & 0x8080);
  regs.sfr.cy = (regs.dr() & 0xe0e0);
  regs.sfr.z  = (regs.dr() & 0xf0f0);
  regs.reset();
}

//$71-7f(alt0): and rN
template<int n> void GSU::op_and_r() {
  regs.dr() = regs.sr() & regs.r[n];
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$71-7f(alt1): bic rN
template<int n> void GSU::op_bic_r() {
  regs.dr() = regs.sr() & ~regs.r[n];
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$71-7f(alt2): and #N
template<int n> void GSU::op_and_i() {
  regs.dr() = regs.sr() & n;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$71-7f(alt3): bic #N
template<int n> void GSU::op_bic_i() {
  regs.dr() = regs.sr() & ~n;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$80-8f(alt0): mult rN
template<int n> void GSU::op_mult_r() {
  regs.dr() = (int8)regs.sr() * (int8)regs.r[n];
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
  if(!regs.cfgr.ms0) step(2);
}

//$80-8f(alt1): umult rN
template<int n> void GSU::op_umult_r() {
  regs.dr() = (uint8)regs.sr() * (uint8)regs.r[n];
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
  if(!regs.cfgr.ms0) step(2);
}

//$80-8f(alt2): mult #N
template<int n> void GSU::op_mult_i() {
  regs.dr() = (int8)regs.sr() * (int8)n;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
  if(!regs.cfgr.ms0) step(2);
}

//$80-8f(alt3): umult #N
template<int n> void GSU::op_umult_i() {
  regs.dr() = (uint8)regs.sr() * (uint8)n;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
  if(!regs.cfgr.ms0) step(2);
}

//$90: sbk
void GSU::op_sbk() {
  rambuffer_write(regs.ramaddr ^ 0, regs.sr() >> 0);
  rambuffer_write(regs.ramaddr ^ 1, regs.sr() >> 8);
  regs.reset();
}

//$91-94: link #N
template<int n> void GSU::op_link() {
  regs.r[11] = regs.r[15] + n;
  regs.reset();
}

//$95: sex
void GSU::op_sex() {
  regs.dr() = (int8)regs.sr();
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$96(alt0): asr
void GSU::op_asr() {
  regs.sfr.cy = (regs.sr() & 1);
  regs.dr() = (int16_t)regs.sr() >> 1;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$96(alt1): div2
void GSU::op_div2() {
  regs.sfr.cy = (regs.sr() & 1);
  regs.dr() = ((int16_t)regs.sr() >> 1) + ((regs.sr() + 1) >> 16);
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$97: ror
void GSU::op_ror() {
  bool carry = (regs.sr() & 1);
  regs.dr() = (regs.sfr.cy << 15) | (regs.sr() >> 1);
  regs.sfr.s  = (regs.dr() & 0x8000);
  regs.sfr.cy = carry;
  regs.sfr.z  = (regs.dr() == 0);
  regs.reset();
}

//$98-9d(alt0): jmp rN
template<int n> void GSU::op_jmp_r() {
  regs.r[15] = regs.r[n];
  regs.reset();
}

//$98-9d(alt1): ljmp rN
template<int n> void GSU::op_ljmp_r() {
  regs.pbr = regs.r[n] & 0x7f;
  regs.r[15] = regs.sr();
  regs.cbr = regs.r[15] & 0xfff0;
  cache_flush();
  regs.reset();
}

//$9e: lob
void GSU::op_lob() {
  regs.dr() = regs.sr() & 0xff;
  regs.sfr.s = (regs.dr() & 0x80);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$9f(alt0): fmult
void GSU::op_fmult() {
  uint32_t result = (int16_t)regs.sr() * (int16_t)regs.r[6];
  regs.dr() = result >> 16;
  regs.sfr.s  = (regs.dr() & 0x8000);
  regs.sfr.cy = (result & 0x8000);
  regs.sfr.z  = (regs.dr() == 0);
  regs.reset();
  step(4 + (regs.cfgr.ms0 << 2));
}

//$9f(alt1): lmult
void GSU::op_lmult() {
  uint32_t result = (int16_t)regs.sr() * (int16_t)regs.r[6];
  regs.r[4] = result;
  regs.dr() = result >> 16;
  regs.sfr.s  = (regs.dr() & 0x8000);
  regs.sfr.cy = (result & 0x8000);
  regs.sfr.z  = (regs.dr() == 0);
  regs.reset();
  step(4 + (regs.cfgr.ms0 << 2));
}

//$a0-af(alt0): ibt rN,#pp
template<int n> void GSU::op_ibt_r() {
  regs.r[n] = (int8)pipe();
  regs.reset();
}

//$a0-af(alt1): lms rN,(yy)
template<int n> void GSU::op_lms_r() {
  regs.ramaddr = pipe() << 1;
  uint16_t data;
  data  = rambuffer_read(regs.ramaddr ^ 0) << 0;
  data |= rambuffer_read(regs.ramaddr ^ 1) << 8;
  regs.r[n] = data;
  regs.reset();
}

//$a0-af(alt2): sms (yy),rN
template<int n> void GSU::op_sms_r() {
  regs.ramaddr = pipe() << 1;
  rambuffer_write(regs.ramaddr ^ 0, regs.r[n] >> 0);
  rambuffer_write(regs.ramaddr ^ 1, regs.r[n] >> 8);
  regs.reset();
}

//$b0-bf(b0): from rN
//$b0-bf(b1): moves rN
template<int n> void GSU::op_from_r() {
  if(regs.sfr.b == 0) {
    regs.sreg = n;
  } else {
    regs.dr() = regs.r[n];
    regs.sfr.ov = (regs.dr() & 0x80);
    regs.sfr.s  = (regs.dr() & 0x8000);
    regs.sfr.z  = (regs.dr() == 0);
    regs.reset();
  }
}

//$c0: hib
void GSU::op_hib() {
  regs.dr() = regs.sr() >> 8;
  regs.sfr.s = (regs.dr() & 0x80);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$c1-cf(alt0): or rN
template<int n> void GSU::op_or_r() {
  regs.dr() = regs.sr() | regs.r[n];
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$c1-cf(alt1): xor rN
template<int n> void GSU::op_xor_r() {
  regs.dr() = regs.sr() ^ regs.r[n];
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$c1-cf(alt2): or #N
template<int n> void GSU::op_or_i() {
  regs.dr() = regs.sr() | n;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$c1-cf(alt3): xor #N
template<int n> void GSU::op_xor_i() {
  regs.dr() = regs.sr() ^ n;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$d0-de: inc rN
template<int n> void GSU::op_inc_r() {
  regs.r[n]++;
  regs.sfr.s = (regs.r[n] & 0x8000);
  regs.sfr.z = (regs.r[n] == 0);
  regs.reset();
}

//$df(alt0): getc
void GSU::op_getc() {
  regs.colr = color(rombuffer_read());
  regs.reset();
}

//$df(alt2): ramb
void GSU::op_ramb() {
  rambuffer_sync();
  regs.rambr = regs.sr();
  regs.reset();
}

//$df(alt3): romb
void GSU::op_romb() {
  rombuffer_sync();
  regs.rombr = regs.sr() & 0x7f;
  regs.reset();
}

//$e0-ee: dec rN
template<int n> void GSU::op_dec_r() {
  regs.r[n]--;
  regs.sfr.s = (regs.r[n] & 0x8000);
  regs.sfr.z = (regs.r[n] == 0);
  regs.reset();
}

//$ef(alt0): getb
void GSU::op_getb() {
  regs.dr() = rombuffer_read();
  regs.reset();
}

//$ef(alt1): getbh
void GSU::op_getbh() {
  regs.dr() = (rombuffer_read() << 8) | (regs.sr() & 0x00ff);
  regs.reset();
}

//$ef(alt2): getbl
void GSU::op_getbl() {
  regs.dr() = (regs.sr() & 0xff00) | (rombuffer_read() << 0);
  regs.reset();
}

//$ef(alt3): getbs
void GSU::op_getbs() {
  regs.dr() = (int8)rombuffer_read();
  regs.reset();
}

//$f0-ff(alt0): iwt rN,#xx
template<int n> void GSU::op_iwt_r() {
  uint16_t data;
  data  = pipe() << 0;
  data |= pipe() << 8;
  regs.r[n] = data;
  regs.reset();
}

//$f0-ff(alt1): lm rN,(xx)
template<int n> void GSU::op_lm_r() {
  regs.ramaddr  = pipe() << 0;
  regs.ramaddr |= pipe() << 8;
  uint16_t data;
  data  = rambuffer_read(regs.ramaddr ^ 0) << 0;
  data |= rambuffer_read(regs.ramaddr ^ 1) << 8;
  regs.r[n] = data;
  regs.reset();
}

//$f0-ff(alt2): sm (xx),rN
template<int n> void GSU::op_sm_r() {
  regs.ramaddr  = pipe() << 0;
  regs.ramaddr |= pipe() << 8;
  rambuffer_write(regs.ramaddr ^ 0, regs.r[n] >> 0);
  rambuffer_write(regs.ramaddr ^ 1, regs.r[n] >> 8);
  regs.reset();
}
