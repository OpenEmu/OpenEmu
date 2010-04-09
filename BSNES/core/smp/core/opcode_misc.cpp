void SMPcore::op_nop() {
  op_io();
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
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
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
  regs.p.n = !!(regs.a & 0x80);
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
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

template<int mask, int value> void SMPcore::op_setbit() {
  op_io();
  regs.p = (regs.p & ~mask) | value;
}

void SMPcore::op_notc() {
  op_io();
  op_io();
  regs.p.c = !regs.p.c;
}

template<int value> void SMPcore::op_seti() {
  op_io();
  op_io();
  regs.p.i = value;
}

template<int op, int value> void SMPcore::op_setbit_dp() {
  dp = op_readpc();
  rd = op_readdp(dp);
  rd = (op ? rd | value : rd & ~value);
  op_writedp(dp, rd);
}

template<int n> void SMPcore::op_push_reg() {
  op_io();
  op_io();
  op_writestack(regs.r[n]);
}

void SMPcore::op_push_p() {
  op_io();
  op_io();
  op_writestack(regs.p);
}

template<int n> void SMPcore::op_pop_reg() {
  op_io();
  op_io();
  regs.r[n] = op_readstack();
}

void SMPcore::op_pop_p() {
  op_io();
  op_io();
  regs.p = op_readstack();
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
  regs.p.n = !!(regs.y & 0x80);
  regs.p.z = (regs.y == 0);
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
  regs.p.v = !!(regs.y >= regs.x);
  regs.p.h = !!((regs.y & 15) >= (regs.x & 15));
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
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}
