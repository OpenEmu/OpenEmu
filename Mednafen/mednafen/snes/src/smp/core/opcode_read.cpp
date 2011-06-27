#ifdef SMPCORE_CPP

template<uint8 (SMPcore::*op)(uint8, uint8), int n>
void SMPcore::op_read_reg_const() {
  rd = op_readpc();
  regs.r[n] = (this->*op)(regs.r[n], rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_a_ix() {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = (this->*op)(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8), int n>
void SMPcore::op_read_reg_dp() {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.r[n] = (this->*op)(regs.r[n], rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_a_dpx() {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.a = (this->*op)(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8), int n>
void SMPcore::op_read_reg_addr() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  rd  = op_readaddr(dp);
  regs.r[n] = (this->*op)(regs.r[n], rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8), int i>
void SMPcore::op_read_a_addrr() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.r[i]);
  regs.a = (this->*op)(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_a_idpx() {
  dp  = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp + 0) << 0;
  sp |= op_readdp(dp + 1) << 8;
  rd  = op_readaddr(sp);
  regs.a = (this->*op)(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_a_idpy() {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp + 0) << 0;
  sp |= op_readdp(dp + 1) << 8;
  rd  = op_readaddr(sp + regs.y);
  regs.a = (this->*op)(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_ix_iy() {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = (this->*op)(wr, rd);
  static uint8 (SMPcore::*cmp)(uint8, uint8) = &SMPcore::op_cmp;
  (op != cmp) ? op_writedp(regs.x, wr) : op_io();
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_dp_dp() {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = (this->*op)(wr, rd);
  static uint8 (SMPcore::*cmp)(uint8, uint8) = &SMPcore::op_cmp;
  (op != cmp) ? op_writedp(dp, wr) : op_io();
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_dp_const() {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = (this->*op)(wr, rd);
  static uint8 (SMPcore::*cmp)(uint8, uint8) = &SMPcore::op_cmp;
  (op != cmp) ? op_writedp(dp, wr) : op_io();
}

template<uint16 (SMPcore::*op)(uint16, uint16)>
void SMPcore::op_read_ya_dp() {
  dp  = op_readpc();
  rd  = op_readdp(dp + 0) << 0;
  op_io();
  rd |= op_readdp(dp + 1) << 8;
  regs.ya = (this->*op)(regs.ya, rd);
}

void SMPcore::op_cmpw_ya_dp() {
  dp  = op_readpc();
  rd  = op_readdp(dp + 0) << 0;
  rd |= op_readdp(dp + 1) << 8;
  op_cmpw(regs.ya, rd);
}

template<int op> void SMPcore::op_and1_bit() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd  = op_readaddr(dp);
  regs.p.c = regs.p.c & ((bool)(rd & (1 << bit)) ^ op);
}

void SMPcore::op_eor1_bit() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd  = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c ^ (bool)(rd & (1 << bit));
}

void SMPcore::op_not1_bit() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd  = op_readaddr(dp);
  rd ^= 1 << bit;
  op_writeaddr(dp, rd);
}

template<int op> void SMPcore::op_or1_bit() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd  = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c | ((bool)(rd & (1 << bit)) ^ op);
}

#endif
