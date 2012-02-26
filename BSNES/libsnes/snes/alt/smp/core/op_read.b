adc_a_const(0x88, adc, a),
and_a_const(0x28, and, a),
cmp_a_const(0x68, cmp, a),
cmp_x_const(0xc8, cmp, x),
cmp_y_const(0xad, cmp, y),
eor_a_const(0x48, eor, a),
or_a_const(0x08, or, a),
sbc_a_const(0xa8, sbc, a) {
1:rd = op_readpc();
  regs.$2 = op_$1(regs.$2, rd);
}

adc_a_ix(0x86, adc),
and_a_ix(0x26, and),
cmp_a_ix(0x66, cmp),
eor_a_ix(0x46, eor),
or_a_ix(0x06, or),
sbc_a_ix(0xa6, sbc) {
1:op_io();
2:rd = op_readdp(regs.x);
  regs.a = op_$1(regs.a, rd);
}

adc_a_dp(0x84, adc, a),
and_a_dp(0x24, and, a),
cmp_a_dp(0x64, cmp, a),
cmp_x_dp(0x3e, cmp, x),
cmp_y_dp(0x7e, cmp, y),
eor_a_dp(0x44, eor, a),
or_a_dp(0x04, or, a),
sbc_a_dp(0xa4, sbc, a) {
1:dp = op_readpc();
2:rd = op_readdp(dp);
  regs.$2 = op_$1(regs.$2, rd);
}

adc_a_dpx(0x94, adc),
and_a_dpx(0x34, and),
cmp_a_dpx(0x74, cmp),
eor_a_dpx(0x54, eor),
or_a_dpx(0x14, or),
sbc_a_dpx(0xb4, sbc) {
1:dp = op_readpc();
2:op_io();
3:rd = op_readdp(dp + regs.x);
  regs.a = op_$1(regs.a, rd);
}

adc_a_addr(0x85, adc, a),
and_a_addr(0x25, and, a),
cmp_a_addr(0x65, cmp, a),
cmp_x_addr(0x1e, cmp, x),
cmp_y_addr(0x5e, cmp, y),
eor_a_addr(0x45, eor, a),
or_a_addr(0x05, or, a),
sbc_a_addr(0xa5, sbc, a) {
1:dp  = op_readpc();
2:dp |= op_readpc() << 8;
3:rd = op_readaddr(dp);
  regs.$2 = op_$1(regs.$2, rd);
}

adc_a_addrx(0x95, adc, x),
adc_a_addry(0x96, adc, y),
and_a_addrx(0x35, and, x),
and_a_addry(0x36, and, y),
cmp_a_addrx(0x75, cmp, x),
cmp_a_addry(0x76, cmp, y),
eor_a_addrx(0x55, eor, x),
eor_a_addry(0x56, eor, y),
or_a_addrx(0x15, or, x),
or_a_addry(0x16, or, y),
sbc_a_addrx(0xb5, sbc, x),
sbc_a_addry(0xb6, sbc, y) {
1:dp  = op_readpc();
2:dp |= op_readpc() << 8;
3:op_io();
4:rd = op_readaddr(dp + regs.$2);
  regs.a = op_$1(regs.a, rd);
}

adc_a_idpx(0x87, adc),
and_a_idpx(0x27, and),
cmp_a_idpx(0x67, cmp),
eor_a_idpx(0x47, eor),
or_a_idpx(0x07, or),
sbc_a_idpx(0xa7, sbc) {
1:dp = op_readpc() + regs.x;
2:op_io();
3:sp  = op_readdp(dp);
4:sp |= op_readdp(dp + 1) << 8;
5:rd = op_readaddr(sp);
  regs.a = op_$1(regs.a, rd);
}

adc_a_idpy(0x97, adc),
and_a_idpy(0x37, and),
cmp_a_idpy(0x77, cmp),
eor_a_idpy(0x57, eor),
or_a_idpy(0x17, or),
sbc_a_idpy(0xb7, sbc) {
1:dp  = op_readpc();
2:op_io();
3:sp  = op_readdp(dp);
4:sp |= op_readdp(dp + 1) << 8;
5:rd = op_readaddr(sp + regs.y);
  regs.a = op_$1(regs.a, rd);
}

adc_ix_iy(0x99, adc, 1),
and_ix_iy(0x39, and, 1),
cmp_ix_iy(0x79, cmp, 0),
eor_ix_iy(0x59, eor, 1),
or_ix_iy(0x19, or, 1),
sbc_ix_iy(0xb9, sbc, 1) {
1:op_io();
2:rd = op_readdp(regs.y);
3:wr = op_readdp(regs.x);
  wr = op_$1(wr, rd);
4:($2) ? op_writedp(regs.x, wr) : op_io();
}

adc_dp_dp(0x89, adc, 1),
and_dp_dp(0x29, and, 1),
cmp_dp_dp(0x69, cmp, 0),
eor_dp_dp(0x49, eor, 1),
or_dp_dp(0x09, or, 1),
sbc_dp_dp(0xa9, sbc, 1) {
1:sp = op_readpc();
2:rd = op_readdp(sp);
3:dp = op_readpc();
4:wr = op_readdp(dp);
5:wr = op_$1(wr, rd);
  ($2) ? op_writedp(dp, wr) : op_io();
}

adc_dp_const(0x98, adc, 1),
and_dp_const(0x38, and, 1),
cmp_dp_const(0x78, cmp, 0),
eor_dp_const(0x58, eor, 1),
or_dp_const(0x18, or, 1),
sbc_dp_const(0xb8, sbc, 1) {
1:rd = op_readpc();
2:dp = op_readpc();
3:wr = op_readdp(dp);
4:wr = op_$1(wr, rd);
  ($2) ? op_writedp(dp, wr) : op_io();
}

addw_ya_dp(0x7a, addw),
subw_ya_dp(0x9a, subw) {
1:dp  = op_readpc();
2:rd  = op_readdp(dp);
3:op_io();
4:rd |= op_readdp(dp + 1) << 8;
  regs.ya = op_$1(regs.ya, rd);
}

cmpw_ya_dp(0x5a) {
1:dp  = op_readpc();
2:rd  = op_readdp(dp);
3:rd |= op_readdp(dp + 1) << 8;
  op_cmpw(regs.ya, rd);
}

and1_bit(0x4a, !!),
and1_notbit(0x6a, !) {
1:dp  = op_readpc();
2:dp |= op_readpc() << 8;
3:bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  regs.p.c = regs.p.c & $1(rd & (1 << bit));
}

eor1_bit(0x8a) {
1:dp  = op_readpc();
2:dp |= op_readpc() << 8;
3:bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
4:op_io();
  regs.p.c = regs.p.c ^ !!(rd & (1 << bit));
}

not1_bit(0xea) {
1:dp  = op_readpc();
2:dp |= op_readpc() << 8;
3:bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  rd ^= (1 << bit);
4:op_writeaddr(dp, rd);
}

or1_bit(0x0a, !!),
or1_notbit(0x2a, !) {
1:dp  = op_readpc();
2:dp |= op_readpc() << 8;
3:bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
4:op_io();
  regs.p.c = regs.p.c | $1(rd & (1 << bit));
}
