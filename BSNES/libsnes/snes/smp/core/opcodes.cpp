#ifdef SMPCORE_CPP
#define call (this->*op)

template<uint8 (SMPcore::*op)(uint8)>
void SMPcore::op_adjust(uint8 &r) {
  op_io();
  r = call(r);
}

template<uint8 (SMPcore::*op)(uint8)>
void SMPcore::op_adjust_addr() {
  dp.l = op_readpc();
  dp.h = op_readpc();
  rd = op_read(dp);
  rd = call(rd);
  op_write(dp, rd);
}

template<uint8 (SMPcore::*op)(uint8)>
void SMPcore::op_adjust_dp() {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = call(rd);
  op_writedp(dp, rd);
}

void SMPcore::op_adjust_dpw(signed n) {
  dp = op_readpc();
  rd.w = op_readdp(dp) + n;
  op_writedp(dp++, rd.l);
  rd.h += op_readdp(dp);
  op_writedp(dp++, rd.h);
  regs.p.n = rd & 0x8000;
  regs.p.z = rd == 0;
}

template<uint8 (SMPcore::*op)(uint8)>
void SMPcore::op_adjust_dpx() {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  rd = call(rd);
  op_writedp(dp + regs.x, rd);
}

void SMPcore::op_branch(bool condition) {
  rd = op_readpc();
  if(condition == false) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_branch_bit() {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if((bool)(sp & (1 << (opcode >> 5))) == (bool)(opcode & 0x10)) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_pull(uint8 &r) {
  op_io();
  op_io();
  r = op_readsp();
}

void SMPcore::op_push(uint8 r) {
  op_io();
  op_io();
  op_writesp(r);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_addr(uint8 &r) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  rd = op_read(dp);
  r = call(r, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_addri(uint8 &r) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_io();
  rd = op_read(dp + r);
  regs.a = call(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_const(uint8 &r) {
  rd = op_readpc();
  r = call(r, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_dp(uint8 &r) {
  dp = op_readpc();
  rd = op_readdp(dp);
  r = call(r, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_dpi(uint8 &r, uint8 &i) {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + i);
  r = call(r, rd);
}

template<uint16 (SMPcore::*op)(uint16, uint16)>
void SMPcore::op_read_dpw() {
  dp = op_readpc();
  rd.l = op_readdp(dp++);
  if(op != &SMPcore::op_cpw) op_io();
  rd.h = op_readdp(dp++);
  regs.ya = call(regs.ya, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_idpx() {
  dp = op_readpc() + regs.x;
  op_io();
  sp.l = op_readdp(dp++);
  sp.h = op_readdp(dp++);
  rd = op_read(sp);
  regs.a = call(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_idpy() {
  dp = op_readpc();
  op_io();
  sp.l = op_readdp(dp++);
  sp.h = op_readdp(dp++);
  rd = op_read(sp + regs.y);
  regs.a = call(regs.a, rd);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_read_ix() {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = call(regs.a, rd);
}

void SMPcore::op_set_addr_bit() {
  dp.l = op_readpc();
  dp.h = op_readpc();
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_read(dp);
  switch(opcode >> 5) {
  case 0:  //orc  addr:bit
  case 1:  //orc !addr:bit
    op_io();
    regs.p.c |= (rd & (1 << bit)) ^ (bool)(opcode & 0x20);
    break;
  case 2:  //and  addr:bit
  case 3:  //and !addr:bit
    regs.p.c &= (rd & (1 << bit)) ^ (bool)(opcode & 0x20);
    break;
  case 4:  //eor  addr:bit
    op_io();
    regs.p.c ^= (bool)(rd & (1 << bit));
    break;
  case 5:  //ldc  addr:bit
    regs.p.c  = (rd & (1 << bit));
    break;
  case 6:  //stc  addr:bit
    op_io();
    rd = (rd & ~(1 << bit)) | (regs.p.c << bit);
    op_write(dp, rd);
    break;
  case 7:  //not  addr:bit
    rd ^= 1 << bit;
    op_write(dp, rd);
    break;
  }
}

void SMPcore::op_set_bit() {
  dp = op_readpc();
  rd = op_readdp(dp) & ~(1 << (opcode >> 5));
  op_writedp(dp, rd | (!(opcode & 0x10) << (opcode >> 5)));
}

void SMPcore::op_set_flag(bool &flag, bool data) {
  op_io();
  if(&flag == &regs.p.i) op_io();
  flag = data;
}

void SMPcore::op_test_addr(bool set) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  rd = op_read(dp);
  regs.p.n = (regs.a - rd) & 0x80;
  regs.p.z = (regs.a - rd) == 0;
  op_read(dp);
  op_write(dp, set ? rd | regs.a : rd & ~regs.a);
}

void SMPcore::op_transfer(uint8 &from, uint8 &to) {
  op_io();
  to = from;
  if(&to == &regs.s) return;
  regs.p.n = (to & 0x80);
  regs.p.z = (to == 0);
}

void SMPcore::op_write_addr(uint8 &r) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_read(dp);
  op_write(dp, r);
}

void SMPcore::op_write_addri(uint8 &i) {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_io();
  dp += i;
  op_read(dp);
  op_write(dp, regs.a);
}

void SMPcore::op_write_dp(uint8 &r) {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, r);
}

void SMPcore::op_write_dpi(uint8 &r, uint8 &i) {
  dp = op_readpc() + i;
  op_io();
  op_readdp(dp);
  op_writedp(dp, r);
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_write_dp_const() {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = call(wr, rd);
  op != &SMPcore::op_cmp ? op_writedp(dp, wr) : op_io();
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_write_dp_dp() {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  if(op != &SMPcore::op_st) wr = op_readdp(dp);
  wr = call(wr, rd);
  op != &SMPcore::op_cmp ? op_writedp(dp, wr) : op_io();
}

template<uint8 (SMPcore::*op)(uint8, uint8)>
void SMPcore::op_write_ix_iy() {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = call(wr, rd);
  op != &SMPcore::op_cmp ? op_writedp(regs.x, wr) : op_io();
}

//

void SMPcore::op_bne_dp() {
  dp = op_readpc();
  sp = op_readdp(dp);
  rd = op_readpc();
  op_io();
  if(regs.a == sp) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_bne_dpdec() {
  dp = op_readpc();
  wr = op_readdp(dp);
  op_writedp(dp, --wr);
  rd = op_readpc();
  if(wr == 0) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_bne_dpx() {
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

void SMPcore::op_bne_ydec() {
  rd = op_readpc();
  op_io();
  op_io();
  if(--regs.y == 0) return;
  op_io();
  op_io();
  regs.pc += (int8)rd;
}

void SMPcore::op_brk() {
  rd.l = op_read(0xffde);
  rd.h = op_read(0xffdf);
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  op_writesp(regs.p);
  regs.pc = rd;
  regs.p.b = 1;
  regs.p.i = 0;
}

void SMPcore::op_clv() {
  op_io();
  regs.p.v = 0;
  regs.p.h = 0;
}

void SMPcore::op_cmc() {
  op_io();
  op_io();
  regs.p.c = !regs.p.c;
}

void SMPcore::op_daa() {
  op_io();
  op_io();
  if(regs.p.c || (regs.a) > 0x99) {
    regs.a += 0x60;
    regs.p.c = 1;
  }
  if(regs.p.h || (regs.a & 15) > 0x09) {
    regs.a += 0x06;
  }
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SMPcore::op_das() {
  op_io();
  op_io();
  if(!regs.p.c || (regs.a) > 0x99) {
    regs.a -= 0x60;
    regs.p.c = 0;
  }
  if(!regs.p.h || (regs.a & 15) > 0x09) {
    regs.a -= 0x06;
  }
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SMPcore::op_div_ya_x() {
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  ya = regs.ya;
  //overflow set if quotient >= 256
  regs.p.v = (regs.y >= regs.x);
  regs.p.h = ((regs.y & 15) >= (regs.x & 15));
  if(regs.y < (regs.x << 1)) {
    //if quotient is <= 511 (will fit into 9-bit result)
    regs.a = ya / regs.x;
    regs.y = ya % regs.x;
  } else {
    //otherwise, the quotient won't fit into regs.p.v + regs.a
    //this emulates the odd behavior of the S-SMP in this case
    regs.a = 255    - (ya - (regs.x << 9)) / (256 - regs.x);
    regs.y = regs.x + (ya - (regs.x << 9)) % (256 - regs.x);
  }
  //result is set based on a (quotient) only
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void SMPcore::op_jmp_addr() {
  rd.l = op_readpc();
  rd.h = op_readpc();
  regs.pc = rd;
}

void SMPcore::op_jmp_iaddrx() {
  dp.l = op_readpc();
  dp.h = op_readpc();
  op_io();
  dp += regs.x;
  rd.l = op_read(dp++);
  rd.h = op_read(dp++);
  regs.pc = rd;
}

void SMPcore::op_jsp_dp() {
  rd = op_readpc();
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  regs.pc = 0xff00 | rd;
}

void SMPcore::op_jsr_addr() {
  rd.l = op_readpc();
  rd.h = op_readpc();
  op_io();
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  regs.pc = rd;
}

void SMPcore::op_jst() {
  dp = 0xffde - ((opcode >> 4) << 1);
  rd.l = op_read(dp++);
  rd.h = op_read(dp++);
  op_io();
  op_io();
  op_io();
  op_writesp(regs.pc.h);
  op_writesp(regs.pc.l);
  regs.pc = rd;
}

void SMPcore::op_lda_ixinc() {
  op_io();
  regs.a = op_readdp(regs.x++);
  op_io();
  regs.p.n = regs.a & 0x80;
  regs.p.z = regs.a == 0;
}

void SMPcore::op_mul_ya() {
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  op_io();
  ya = regs.y * regs.a;
  regs.a = ya;
  regs.y = ya >> 8;
  //result is set based on y (high-byte) only
  regs.p.n = (regs.y & 0x80);
  regs.p.z = (regs.y == 0);
}

void SMPcore::op_nop() {
  op_io();
}

void SMPcore::op_plp() {
  op_io();
  op_io();
  regs.p = op_readsp();
}

void SMPcore::op_rti() {
  regs.p = op_readsp();
  rd.l = op_readsp();
  rd.h = op_readsp();
  op_io();
  op_io();
  regs.pc = rd;
}

void SMPcore::op_rts() {
  rd.l = op_readsp();
  rd.h = op_readsp();
  op_io();
  op_io();
  regs.pc = rd;
}

void SMPcore::op_sta_idpx() {
  sp = op_readpc() + regs.x;
  op_io();
  dp.l = op_readdp(sp++);
  dp.h = op_readdp(sp++);
  op_read(dp);
  op_write(dp, regs.a);
}

void SMPcore::op_sta_idpy() {
  sp = op_readpc();
  dp.l = op_readdp(sp++);
  dp.h = op_readdp(sp++);
  op_io();
  dp += regs.y;
  op_read(dp);
  op_write(dp, regs.a);
}

void SMPcore::op_sta_ix() {
  op_io();
  op_readdp(regs.x);
  op_writedp(regs.x, regs.a);
}

void SMPcore::op_sta_ixinc() {
  op_io();
  op_io();
  op_writedp(regs.x++, regs.a);
}

void SMPcore::op_stw_dp() {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp++, regs.a);
  op_writedp(dp++, regs.y);
}

void SMPcore::op_wait() {
  while(true) {
    op_io();
    op_io();
  }
}

void SMPcore::op_xcn() {
  op_io();
  op_io();
  op_io();
  op_io();
  regs.a = (regs.a >> 4) | (regs.a << 4);
  regs.p.n = regs.a & 0x80;
  regs.p.z = regs.a == 0;
}

#undef call
#endif
