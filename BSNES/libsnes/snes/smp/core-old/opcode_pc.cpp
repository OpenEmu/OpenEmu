#ifdef SMPCORE_CPP

void SMPcore::op_bra() {
  rd = op_readpc();
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

template<int flag, int value> void SMPcore::op_branch() {
  rd = op_readpc();
  if((bool)(regs.p & flag) != value) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

template<int mask, int value> void SMPcore::op_bitbranch() {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((bool)(sp & mask) != value) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_cbne_dp() {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if(regs.a == sp) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_cbne_dpx() {
  dp = op_readpc();
  op_io();
  sp = op_readdp(dp + regs.x);
  rd = op_readpc();
  op_io();
  if(regs.a == sp) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;   
}

void SMPcore::op_dbnz_dp() {
  dp = op_readpc();
  wr = op_readdp(dp);
  op_writedp(dp, --wr);
  rd = op_readpc();
  if(wr == 0) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_dbnz_y() {
  rd = op_readpc();
  op_io();
  regs.y--;
  op_io();
  if(regs.y == 0) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_jmp_addr() {
  rd  = op_readpc() << 0;
  rd |= op_readpc() << 8;
  regs.pc = rd;
}

void SMPcore::op_jmp_iaddrx() {
  dp  = op_readpc() << 0;
  dp |= op_readpc() << 8;
  op_io();
  dp += regs.x;
  rd  = op_read(dp + 0) << 0;
  rd |= op_read(dp + 1) << 8;
  regs.pc = rd;
}

void SMPcore::op_call() {
  rd  = op_readpc() << 0;
  rd |= op_readpc() << 8;
  op_io();
  op_io();
  op_io();
  op_writesp(regs.pc >> 8);
  op_writesp(regs.pc >> 0);
  regs.pc = rd;
}

void SMPcore::op_pcall() {
  rd = op_readpc();
  op_io();
  op_io();
  op_writesp(regs.pc >> 8);
  op_writesp(regs.pc >> 0);
  regs.pc = 0xff00 | rd;
}

template<int n> void SMPcore::op_tcall() {
  dp  = 0xffde - (n << 1);
  rd  = op_read(dp + 0) << 0;
  rd |= op_read(dp + 1) << 8;
  op_io();
  op_io();
  op_io();
  op_writesp(regs.pc >> 8);
  op_writesp(regs.pc >> 0);
  regs.pc = rd;
}

void SMPcore::op_brk() {
  rd  = op_read(0xffde) << 0;
  rd |= op_read(0xffdf) << 8;
  op_io();
  op_io();
  op_writesp(regs.pc >> 8);
  op_writesp(regs.pc >> 0);
  op_writesp(regs.p);
  regs.pc = rd;
  regs.p.b = 1;
  regs.p.i = 0;
}

void SMPcore::op_ret() {
  rd  = op_readsp() << 0;
  rd |= op_readsp() << 8;
  op_io();
  op_io();
  regs.pc = rd;
}

void SMPcore::op_reti() {
  regs.p = op_readsp();
  rd  = op_readsp() << 0;
  rd |= op_readsp() << 8;
  op_io();
  op_io();
  regs.pc = rd;
}

#endif
