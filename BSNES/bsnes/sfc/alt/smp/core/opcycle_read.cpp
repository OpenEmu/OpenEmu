case 0x88: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x28: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x68: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc8: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.x = op_cmp(regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xad: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.y = op_cmp(regs.y, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x48: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x08: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa8: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x86: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.x);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x26: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.x);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x66: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.x);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x46: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.x);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x06: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.x);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa6: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.x);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x84: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x24: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x64: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x3e: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.x = op_cmp(regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x7e: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.y = op_cmp(regs.y, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x44: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x04: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa4: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x94: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x34: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x74: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x54: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x14: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb4: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x85: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x25: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x65: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x1e: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.x = op_cmp(regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x5e: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.y = op_cmp(regs.y, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x45: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x05: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa5: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x95: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.x);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x96: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.y);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x35: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.x);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x36: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.y);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x75: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.x);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x76: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.y);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x55: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.x);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x56: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.y);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x15: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.x);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x16: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.y);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb5: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.x);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb6: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd = op_readaddr(dp + regs.y);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x87: {
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
    rd = op_readaddr(sp);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x27: {
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
    rd = op_readaddr(sp);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x67: {
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
    rd = op_readaddr(sp);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x47: {
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
    rd = op_readaddr(sp);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x07: {
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
    rd = op_readaddr(sp);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa7: {
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
    rd = op_readaddr(sp);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x97: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
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
    rd = op_readaddr(sp + regs.y);
    regs.a = op_adc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x37: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
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
    rd = op_readaddr(sp + regs.y);
    regs.a = op_and(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x77: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
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
    rd = op_readaddr(sp + regs.y);
    regs.a = op_cmp(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x57: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
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
    rd = op_readaddr(sp + regs.y);
    regs.a = op_eor(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x17: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
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
    rd = op_readaddr(sp + regs.y);
    regs.a = op_or(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb7: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
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
    rd = op_readaddr(sp + regs.y);
    regs.a = op_sbc(regs.a, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x99: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.y);
    break;
  case 3:
    wr = op_readdp(regs.x);
    wr = op_adc(wr, rd);
    break;
  case 4:
    (1) ? op_writedp(regs.x, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x39: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.y);
    break;
  case 3:
    wr = op_readdp(regs.x);
    wr = op_and(wr, rd);
    break;
  case 4:
    (1) ? op_writedp(regs.x, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x79: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.y);
    break;
  case 3:
    wr = op_readdp(regs.x);
    wr = op_cmp(wr, rd);
    break;
  case 4:
    (0) ? op_writedp(regs.x, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x59: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.y);
    break;
  case 3:
    wr = op_readdp(regs.x);
    wr = op_eor(wr, rd);
    break;
  case 4:
    (1) ? op_writedp(regs.x, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x19: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.y);
    break;
  case 3:
    wr = op_readdp(regs.x);
    wr = op_or(wr, rd);
    break;
  case 4:
    (1) ? op_writedp(regs.x, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb9: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    rd = op_readdp(regs.y);
    break;
  case 3:
    wr = op_readdp(regs.x);
    wr = op_sbc(wr, rd);
    break;
  case 4:
    (1) ? op_writedp(regs.x, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x89: {
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
    wr = op_readdp(dp);
    break;
  case 5:
    wr = op_adc(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x29: {
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
    wr = op_readdp(dp);
    break;
  case 5:
    wr = op_and(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x69: {
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
    wr = op_readdp(dp);
    break;
  case 5:
    wr = op_cmp(wr, rd);
    (0) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x49: {
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
    wr = op_readdp(dp);
    break;
  case 5:
    wr = op_eor(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x09: {
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
    wr = op_readdp(dp);
    break;
  case 5:
    wr = op_or(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa9: {
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
    wr = op_readdp(dp);
    break;
  case 5:
    wr = op_sbc(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x98: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    dp = op_readpc();
    break;
  case 3:
    wr = op_readdp(dp);
    break;
  case 4:
    wr = op_adc(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x38: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    dp = op_readpc();
    break;
  case 3:
    wr = op_readdp(dp);
    break;
  case 4:
    wr = op_and(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x78: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    dp = op_readpc();
    break;
  case 3:
    wr = op_readdp(dp);
    break;
  case 4:
    wr = op_cmp(wr, rd);
    (0) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x58: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    dp = op_readpc();
    break;
  case 3:
    wr = op_readdp(dp);
    break;
  case 4:
    wr = op_eor(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x18: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    dp = op_readpc();
    break;
  case 3:
    wr = op_readdp(dp);
    break;
  case 4:
    wr = op_or(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb8: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    dp = op_readpc();
    break;
  case 3:
    wr = op_readdp(dp);
    break;
  case 4:
    wr = op_sbc(wr, rd);
    (1) ? op_writedp(dp, wr) : op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x7a: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    rd  = op_readdp(dp);
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd |= op_readdp(dp + 1) << 8;
    regs.ya = op_addw(regs.ya, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x9a: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    rd  = op_readdp(dp);
    break;
  case 3:
    op_io();
    break;
  case 4:
    rd |= op_readdp(dp + 1) << 8;
    regs.ya = op_subw(regs.ya, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x5a: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    rd  = op_readdp(dp);
    break;
  case 3:
    rd |= op_readdp(dp + 1) << 8;
    op_cmpw(regs.ya, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x4a: {
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
    regs.p.c = regs.p.c & !!(rd & (1 << bit));
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x6a: {
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
    regs.p.c = regs.p.c & !(rd & (1 << bit));
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x8a: {
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
    break;
  case 4:
    op_io();
    regs.p.c = regs.p.c ^ !!(rd & (1 << bit));
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xea: {
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
    rd ^= (1 << bit);
    break;
  case 4:
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x0a: {
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
    break;
  case 4:
    op_io();
    regs.p.c = regs.p.c | !!(rd & (1 << bit));
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x2a: {
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
    break;
  case 4:
    op_io();
    regs.p.c = regs.p.c | !(rd & (1 << bit));
    opcode_cycle = 0;
    break;
  }
  break;
}

