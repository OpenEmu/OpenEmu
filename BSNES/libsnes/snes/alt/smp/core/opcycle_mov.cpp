case 0x7d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = regs.x;
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xdd: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = regs.y;
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x5d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.x = regs.a;
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xfd: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.y = regs.a;
    regs.p.n = !!(regs.y & 0x80);
    regs.p.z = (regs.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x9d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.x = regs.sp;
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xbd: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.sp = regs.x;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe8: {
  switch(opcode_cycle++) {
  case 1:
    regs.a = op_readpc();
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xcd: {
  switch(opcode_cycle++) {
  case 1:
    regs.x = op_readpc();
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x8d: {
  switch(opcode_cycle++) {
  case 1:
    regs.y = op_readpc();
    regs.p.n = !!(regs.y & 0x80);
    regs.p.z = (regs.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe6: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    regs.a = op_readdp(regs.x);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xbf: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    regs.a = op_readdp(regs.x++);
    break;
  case 3:
    op_io();
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe4: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.a = op_readdp(sp);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf8: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.x = op_readdp(sp);
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xeb: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.y = op_readdp(sp);
    regs.p.n = !!(regs.y & 0x80);
    regs.p.z = (regs.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf4: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    regs.a = op_readdp(sp + regs.x);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf9: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    regs.x = op_readdp(sp + regs.y);
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xfb: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    regs.y = op_readdp(sp + regs.x);
    regs.p.n = !!(regs.y & 0x80);
    regs.p.z = (regs.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe5: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    sp |= op_readpc() << 8;
    break;
  case 3:
    regs.a = op_readaddr(sp);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe9: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    sp |= op_readpc() << 8;
    break;
  case 3:
    regs.x = op_readaddr(sp);
    regs.p.n = !!(regs.x & 0x80);
    regs.p.z = (regs.x == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xec: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    sp |= op_readpc() << 8;
    break;
  case 3:
    regs.y = op_readaddr(sp);
    regs.p.n = !!(regs.y & 0x80);
    regs.p.z = (regs.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf5: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    sp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    regs.a = op_readaddr(sp + regs.x);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf6: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    sp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    regs.a = op_readaddr(sp + regs.y);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe7: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc() + regs.x;
    break;
  case 2:
    op_io();
    break;
  case 3:
    sp  = op_readdp(dp);
    break;
  case 4:
    sp |= op_readdp(dp + 1) << 8;
    break;
  case 5:
    regs.a = op_readaddr(sp);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf7: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    sp  = op_readdp(dp);
    break;
  case 4:
    sp |= op_readdp(dp + 1) << 8;
    break;
  case 5:
    regs.a = op_readaddr(sp + regs.y);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xfa: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    rd = op_readdp(sp);
    break;
  case 3:
    dp = op_readpc();
    break;
  case 4:
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x8f: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    dp = op_readpc();
    break;
  case 3:
    op_readdp(dp);
    break;
  case 4:
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc6: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_readdp(regs.x);
    break;
  case 3:
    op_writedp(regs.x, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xaf: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_writedp(regs.x++, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc4: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd8: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xcb: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, regs.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd4: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    op_io();
    dp += regs.x;
    break;
  case 3:
    op_readdp(dp);
    break;
  case 4:
    op_writedp(dp, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd9: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    op_io();
    dp += regs.y;
    break;
  case 3:
    op_readdp(dp);
    break;
  case 4:
    op_writedp(dp, regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xdb: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    op_io();
    dp += regs.x;
    break;
  case 3:
    op_readdp(dp);
    break;
  case 4:
    op_writedp(dp, regs.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc5: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_readaddr(dp);
    break;
  case 4:
    op_writeaddr(dp, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc9: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_readaddr(dp);
    break;
  case 4:
    op_writeaddr(dp, regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xcc: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_readaddr(dp);
    break;
  case 4:
    op_writeaddr(dp, regs.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd5: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    dp += regs.x;
    break;
  case 4:
    op_readaddr(dp);
    break;
  case 5:
    op_writeaddr(dp, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd6: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    dp += regs.y;
    break;
  case 4:
    op_readaddr(dp);
    break;
  case 5:
    op_writeaddr(dp, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc7: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    op_io();
    sp += regs.x;
    break;
  case 3:
    dp  = op_readdp(sp);
    break;
  case 4:
    dp |= op_readdp(sp + 1) << 8;
    break;
  case 5:
    op_readaddr(dp);
    break;
  case 6:
    op_writeaddr(dp, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd7: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    dp  = op_readdp(sp);
    break;
  case 3:
    dp |= op_readdp(sp + 1) << 8;
    break;
  case 4:
    op_io();
    dp += regs.y;
    break;
  case 5:
    op_readaddr(dp);
    break;
  case 6:
    op_writeaddr(dp, regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xba: {
  switch(opcode_cycle++) {
  case 1:
    sp = op_readpc();
    break;
  case 2:
    regs.a = op_readdp(sp);
    break;
  case 3:
    op_io();
    break;
  case 4:
    regs.y = op_readdp(sp + 1);
    regs.p.n = !!(regs.ya & 0x8000);
    regs.p.z = (regs.ya == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xda: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_readdp(dp);
    break;
  case 3:
    op_writedp(dp,     regs.a);
    break;
  case 4:
    op_writedp(dp + 1, regs.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xaa: {
  switch(opcode_cycle++) {
  case 1:
    sp  = op_readpc();
    break;
  case 2:
    sp |= op_readpc() << 8;
    break;
  case 3:
    bit = sp >> 13;
    sp &= 0x1fff;
    rd = op_readaddr(sp);
    regs.p.c = !!(rd & (1 << bit));
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xca: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    bit = dp >> 13;
    dp &= 0x1fff;
    rd = op_readaddr(dp);
    if(regs.p.c)rd |=  (1 << bit);
    else        rd &= ~(1 << bit);
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

