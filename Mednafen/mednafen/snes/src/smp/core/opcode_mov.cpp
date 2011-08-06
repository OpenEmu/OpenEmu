#ifdef SMPCORE_CPP

template<int to, int from> void SMPcore::op_mov_reg_reg() {
  op_io();
  regs.r[to] = regs.r[from];
  regs.p.n = (regs.r[to] & 0x80);
  regs.p.z = (regs.r[to] == 0);
}

void SMPcore::op_mov_sp_x() {
  op_io();
  regs.sp = regs.x;
}

template<int n> void SMPcore::op_mov_reg_const() {
  regs.r[n] = op_readpc();
  regs.p.n = (regs.r[n] & 0x80);
  regs.p.z = (regs.r[n] == 0);
}

void SMPcore::op_mov_a_ix() {
  op_io();
  regs.a = op_readdp(regs.x);
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SMPcore::op_mov_a_ixinc() {
  op_io();
  regs.a = op_readdp(regs.x++);
  op_io();
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

template<int n> void SMPcore::op_mov_reg_dp() {
  sp = op_readpc();
  regs.r[n] = op_readdp(sp);
  regs.p.n = (regs.r[n] & 0x80);
  regs.p.z = (regs.r[n] == 0);
}

template<int n, int i> void SMPcore::op_mov_reg_dpr() {
  sp = op_readpc();
  op_io();
  regs.r[n] = op_readdp(sp + regs.r[i]);
  regs.p.n = (regs.r[n] & 0x80);
  regs.p.z = (regs.r[n] == 0);
}

template<int n> void SMPcore::op_mov_reg_addr() {
  sp  = op_readpc() << 0;
  sp |= op_readpc() << 8;
  regs.r[n] = op_readaddr(sp);
  regs.p.n = (regs.r[n] & 0x80);
  regs.p.z = (regs.r[n] == 0);
}

template<int i> void SMPcore::op_mov_a_addrr() {
  sp  = op_readpc() << 0;
  sp |= op_readpc() << 8;
  op_io();
  regs.a = op_readaddr(sp + regs.r[i]);
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SMPcore::op_mov_a_idpx() {
  dp  = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp + 0) << 0;
  sp |= op_readdp(dp + 1) << 8;
  regs.a = op_readaddr(sp);
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SMPcore::op_mov_a_idpy() {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp + 0) << 0;
  sp |= op_readdp(dp + 1) << 8;
  regs.a = op_readaddr(sp + regs.y);
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SMPcore::op_mov_dp_dp() {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  op_writedp(dp, rd);
}

void SMPcore::op_mov_dp_const() {
  rd = op_readpc();
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, rd);
}

void SMPcore::op_mov_ix_a() {
  op_io();
  op_readdp(regs.x);
  op_writedp(regs.x, regs.a);
}

void SMPcore::op_mov_ixinc_a() {
  op_io();
  op_io();
  op_writedp(regs.x++, regs.a);
}

template<int n> void SMPcore::op_mov_dp_reg() {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, regs.r[n]);
}

template<int n, int i> void SMPcore::op_mov_dpr_reg() {
  dp  = op_readpc();
  op_io();
  dp += regs.r[i];
  op_readdp(dp);
  op_writedp(dp, regs.r[n]);
}

template<int n> void SMPcore::op_mov_addr_reg() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  op_readaddr(dp);
  op_writeaddr(dp, regs.r[n]);
}

template<int i> void SMPcore::op_mov_addrr_a() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  op_io();
  dp += regs.r[i];
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
}

void SMPcore::op_mov_idpx_a() {
  sp  = op_readpc();
  op_io();
  sp += regs.x;
  dp  = op_readdp(sp + 0) << 0;
  dp |= op_readdp(sp + 1) << 8;
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
}

void SMPcore::op_mov_idpy_a() {
  sp  = op_readpc();
  dp  = op_readdp(sp + 0) << 0;
  dp |= op_readdp(sp + 1) << 8;
  op_io();
  dp += regs.y;
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
}

void SMPcore::op_movw_ya_dp() {
  sp = op_readpc();
  regs.a = op_readdp(sp + 0);
  op_io();
  regs.y = op_readdp(sp + 1);
  regs.p.n = (regs.ya & 0x8000);
  regs.p.z = (regs.ya == 0);
}

void SMPcore::op_movw_dp_ya() {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp + 0, regs.a);
  op_writedp(dp + 1, regs.y);
}

void SMPcore::op_mov1_c_bit() {
  sp  = op_readpc() << 0;
  sp |= op_readpc() << 8;
  bit = sp >> 13;
  sp &= 0x1fff;
  rd = op_readaddr(sp);
  regs.p.c = (rd & (1 << bit));
}

void SMPcore::op_mov1_bit_c() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  (regs.p.c) ? rd |= (1 << bit) : rd &= ~(1 << bit);
  op_io();
  op_writeaddr(dp, rd);
}

#endif
