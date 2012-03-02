case 0x88: {
  rd = op_readpc();
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x28: {
  rd = op_readpc();
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x68: {
  rd = op_readpc();
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0xc8: {
  rd = op_readpc();
  regs.x = op_cmp(regs.x, rd);
  break;
}

case 0xad: {
  rd = op_readpc();
  regs.y = op_cmp(regs.y, rd);
  break;
}

case 0x48: {
  rd = op_readpc();
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x08: {
  rd = op_readpc();
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xa8: {
  rd = op_readpc();
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x86: {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x26: {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x66: {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x46: {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x06: {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xa6: {
  op_io();
  rd = op_readdp(regs.x);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x84: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x24: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x64: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x3e: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.x = op_cmp(regs.x, rd);
  break;
}

case 0x7e: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.y = op_cmp(regs.y, rd);
  break;
}

case 0x44: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x04: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xa4: {
  dp = op_readpc();
  rd = op_readdp(dp);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x94: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x34: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x74: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x54: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x14: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xb4: {
  dp = op_readpc();
  op_io();
  rd = op_readdp(dp + regs.x);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x85: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x25: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x65: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x1e: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.x = op_cmp(regs.x, rd);
  break;
}

case 0x5e: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.y = op_cmp(regs.y, rd);
  break;
}

case 0x45: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x05: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xa5: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  rd = op_readaddr(dp);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x95: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x96: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.y);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x35: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x36: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.y);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x75: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x76: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.y);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x55: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x56: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.y);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x15: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0x16: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.y);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xb5: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.x);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0xb6: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  rd = op_readaddr(dp + regs.y);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x87: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x27: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x67: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x47: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x07: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xa7: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x97: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.y);
  regs.a = op_adc(regs.a, rd);
  break;
}

case 0x37: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.y);
  regs.a = op_and(regs.a, rd);
  break;
}

case 0x77: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.y);
  regs.a = op_cmp(regs.a, rd);
  break;
}

case 0x57: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.y);
  regs.a = op_eor(regs.a, rd);
  break;
}

case 0x17: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.y);
  regs.a = op_or(regs.a, rd);
  break;
}

case 0xb7: {
  dp  = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  rd = op_readaddr(sp + regs.y);
  regs.a = op_sbc(regs.a, rd);
  break;
}

case 0x99: {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = op_adc(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x39: {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = op_and(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x79: {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = op_cmp(wr, rd);
  (0) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x59: {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = op_eor(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x19: {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = op_or(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0xb9: {
  op_io();
  rd = op_readdp(regs.y);
  wr = op_readdp(regs.x);
  wr = op_sbc(wr, rd);
  (1) ? op_writedp(regs.x, wr) : op_io();
  break;
}

case 0x89: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_adc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x29: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_and(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x69: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_cmp(wr, rd);
  (0) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x49: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_eor(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x09: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_or(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0xa9: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_sbc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x98: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_adc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x38: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_and(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x78: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_cmp(wr, rd);
  (0) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x58: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_eor(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x18: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_or(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0xb8: {
  rd = op_readpc();
  dp = op_readpc();
  wr = op_readdp(dp);
  wr = op_sbc(wr, rd);
  (1) ? op_writedp(dp, wr) : op_io();
  break;
}

case 0x7a: {
  dp  = op_readpc();
  rd  = op_readdp(dp);
  op_io();
  rd |= op_readdp(dp + 1) << 8;
  regs.ya = op_addw(regs.ya, rd);
  break;
}

case 0x9a: {
  dp  = op_readpc();
  rd  = op_readdp(dp);
  op_io();
  rd |= op_readdp(dp + 1) << 8;
  regs.ya = op_subw(regs.ya, rd);
  break;
}

case 0x5a: {
  dp  = op_readpc();
  rd  = op_readdp(dp);
  rd |= op_readdp(dp + 1) << 8;
  op_cmpw(regs.ya, rd);
  break;
}

case 0x4a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  regs.p.c = regs.p.c & !!(rd & (1 << bit));
  break;
}

case 0x6a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  regs.p.c = regs.p.c & !(rd & (1 << bit));
  break;
}

case 0x8a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c ^ !!(rd & (1 << bit));
  break;
}

case 0xea: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  rd ^= (1 << bit);
  op_writeaddr(dp, rd);
  break;
}

case 0x0a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c | !!(rd & (1 << bit));
  break;
}

case 0x2a: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  op_io();
  regs.p.c = regs.p.c | !(rd & (1 << bit));
  break;
}

