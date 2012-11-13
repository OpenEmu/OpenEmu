case 0x7d: {
  op_io();
  regs.a = regs.x;
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xdd: {
  op_io();
  regs.a = regs.y;
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0x5d: {
  op_io();
  regs.x = regs.a;
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0xfd: {
  op_io();
  regs.y = regs.a;
  regs.p.n = !!(regs.y & 0x80);
  regs.p.z = (regs.y == 0);
  break;
}

case 0x9d: {
  op_io();
  regs.x = regs.sp;
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0xbd: {
  op_io();
  regs.sp = regs.x;
  break;
}

case 0xe8: {
  regs.a = op_readpc();
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xcd: {
  regs.x = op_readpc();
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0x8d: {
  regs.y = op_readpc();
  regs.p.n = !!(regs.y & 0x80);
  regs.p.z = (regs.y == 0);
  break;
}

case 0xe6: {
  op_io();
  regs.a = op_readdp(regs.x);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xbf: {
  op_io();
  regs.a = op_readdp(regs.x++);
  op_io();
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xe4: {
  sp = op_readpc();
  regs.a = op_readdp(sp);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xf8: {
  sp = op_readpc();
  regs.x = op_readdp(sp);
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0xeb: {
  sp = op_readpc();
  regs.y = op_readdp(sp);
  regs.p.n = !!(regs.y & 0x80);
  regs.p.z = (regs.y == 0);
  break;
}

case 0xf4: {
  sp = op_readpc();
  op_io();
  regs.a = op_readdp(sp + regs.x);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xf9: {
  sp = op_readpc();
  op_io();
  regs.x = op_readdp(sp + regs.y);
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0xfb: {
  sp = op_readpc();
  op_io();
  regs.y = op_readdp(sp + regs.x);
  regs.p.n = !!(regs.y & 0x80);
  regs.p.z = (regs.y == 0);
  break;
}

case 0xe5: {
  sp  = op_readpc();
  sp |= op_readpc() << 8;
  regs.a = op_readaddr(sp);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xe9: {
  sp  = op_readpc();
  sp |= op_readpc() << 8;
  regs.x = op_readaddr(sp);
  regs.p.n = !!(regs.x & 0x80);
  regs.p.z = (regs.x == 0);
  break;
}

case 0xec: {
  sp  = op_readpc();
  sp |= op_readpc() << 8;
  regs.y = op_readaddr(sp);
  regs.p.n = !!(regs.y & 0x80);
  regs.p.z = (regs.y == 0);
  break;
}

case 0xf5: {
  sp  = op_readpc();
  sp |= op_readpc() << 8;
  op_io();
  regs.a = op_readaddr(sp + regs.x);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xf6: {
  sp  = op_readpc();
  sp |= op_readpc() << 8;
  op_io();
  regs.a = op_readaddr(sp + regs.y);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xe7: {
  dp = op_readpc() + regs.x;
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  regs.a = op_readaddr(sp);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xf7: {
  dp = op_readpc();
  op_io();
  sp  = op_readdp(dp);
  sp |= op_readdp(dp + 1) << 8;
  regs.a = op_readaddr(sp + regs.y);
  regs.p.n = !!(regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  break;
}

case 0xfa: {
  sp = op_readpc();
  rd = op_readdp(sp);
  dp = op_readpc();
  op_writedp(dp, rd);
  break;
}

case 0x8f: {
  rd = op_readpc();
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, rd);
  break;
}

case 0xc6: {
  op_io();
  op_readdp(regs.x);
  op_writedp(regs.x, regs.a);
  break;
}

case 0xaf: {
  op_io();
  op_io();
  op_writedp(regs.x++, regs.a);
  break;
}

case 0xc4: {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, regs.a);
  break;
}

case 0xd8: {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, regs.x);
  break;
}

case 0xcb: {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp, regs.y);
  break;
}

case 0xd4: {
  dp  = op_readpc();
  op_io();
  dp += regs.x;
  op_readdp(dp);
  op_writedp(dp, regs.a);
  break;
}

case 0xd9: {
  dp  = op_readpc();
  op_io();
  dp += regs.y;
  op_readdp(dp);
  op_writedp(dp, regs.x);
  break;
}

case 0xdb: {
  dp  = op_readpc();
  op_io();
  dp += regs.x;
  op_readdp(dp);
  op_writedp(dp, regs.y);
  break;
}

case 0xc5: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
  break;
}

case 0xc9: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_readaddr(dp);
  op_writeaddr(dp, regs.x);
  break;
}

case 0xcc: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_readaddr(dp);
  op_writeaddr(dp, regs.y);
  break;
}

case 0xd5: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  dp += regs.x;
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
  break;
}

case 0xd6: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  op_io();
  dp += regs.y;
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
  break;
}

case 0xc7: {
  sp  = op_readpc();
  op_io();
  sp += regs.x;
  dp  = op_readdp(sp);
  dp |= op_readdp(sp + 1) << 8;
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
  break;
}

case 0xd7: {
  sp  = op_readpc();
  dp  = op_readdp(sp);
  dp |= op_readdp(sp + 1) << 8;
  op_io();
  dp += regs.y;
  op_readaddr(dp);
  op_writeaddr(dp, regs.a);
  break;
}

case 0xba: {
  sp = op_readpc();
  regs.a = op_readdp(sp);
  op_io();
  regs.y = op_readdp(sp + 1);
  regs.p.n = !!(regs.ya & 0x8000);
  regs.p.z = (regs.ya == 0);
  break;
}

case 0xda: {
  dp = op_readpc();
  op_readdp(dp);
  op_writedp(dp,     regs.a);
  op_writedp(dp + 1, regs.y);
  break;
}

case 0xaa: {
  sp  = op_readpc();
  sp |= op_readpc() << 8;
  bit = sp >> 13;
  sp &= 0x1fff;
  rd = op_readaddr(sp);
  regs.p.c = !!(rd & (1 << bit));
  break;
}

case 0xca: {
  dp  = op_readpc();
  dp |= op_readpc() << 8;
  bit = dp >> 13;
  dp &= 0x1fff;
  rd = op_readaddr(dp);
  if(regs.p.c)rd |=  (1 << bit);
  else        rd &= ~(1 << bit);
  op_io();
  op_writeaddr(dp, rd);
  break;
}

