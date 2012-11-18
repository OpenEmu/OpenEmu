case 0xbc: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = op_inc(regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x3d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.x = op_inc(regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xfc: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.y = op_inc(regs.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x9c: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = op_dec(regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x1d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.x = op_dec(regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xdc: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.y = op_dec(regs.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x1c: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = op_asl(regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x5c: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = op_lsr(regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x3c: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = op_rol(regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x7c: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.a = op_ror(regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xab: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd = op_inc(rd);
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x8b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd = op_dec(rd);
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x0b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd = op_asl(rd);
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x4b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd = op_lsr(rd);
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x2b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd = op_rol(rd);
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x6b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd = op_ror(rd);
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xbb: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    break;
  case 4:
    rd = op_inc(rd);
    op_writedp(dp + regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x9b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    break;
  case 4:
    rd = op_dec(rd);
    op_writedp(dp + regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x1b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    break;
  case 4:
    rd = op_asl(rd);
    op_writedp(dp + regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x5b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    break;
  case 4:
    rd = op_lsr(rd);
    op_writedp(dp + regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x3b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    break;
  case 4:
    rd = op_rol(rd);
    op_writedp(dp + regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x7b: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    rd = op_readdp(dp + regs.x);
    break;
  case 4:
    rd = op_ror(rd);
    op_writedp(dp + regs.x, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xac: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    break;
  case 4:
    rd = op_inc(rd);
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x8c: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    break;
  case 4:
    rd = op_dec(rd);
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x0c: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    break;
  case 4:
    rd = op_asl(rd);
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x4c: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    break;
  case 4:
    rd = op_lsr(rd);
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x2c: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    break;
  case 4:
    rd = op_rol(rd);
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x6c: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    break;
  case 4:
    rd = op_ror(rd);
    op_writeaddr(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x0e: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.p.n = !!((regs.a - rd) & 0x80);
    regs.p.z = ((regs.a - rd) == 0);
    break;
  case 4:
    op_readaddr(dp);
    break;
  case 5:
    op_writeaddr(dp, rd | regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x4e: {
  switch(opcode_cycle++) {
  case 1:
    dp  = op_readpc();
    break;
  case 2:
    dp |= op_readpc() << 8;
    break;
  case 3:
    rd = op_readaddr(dp);
    regs.p.n = !!((regs.a - rd) & 0x80);
    regs.p.z = ((regs.a - rd) == 0);
    break;
  case 4:
    op_readaddr(dp);
    break;
  case 5:
    op_writeaddr(dp, rd &~ regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x3a: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    rd++;
    break;
  case 3:
    op_writedp(dp++, rd);
    break;
  case 4:
    rd += op_readdp(dp) << 8;
    break;
  case 5:
    op_writedp(dp, rd >> 8);
    regs.p.n = !!(rd & 0x8000);
    regs.p.z = (rd == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x1a: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    rd--;
    break;
  case 3:
    op_writedp(dp++, rd);
    break;
  case 4:
    rd += op_readdp(dp) << 8;
    break;
  case 5:
    op_writedp(dp, rd >> 8);
    regs.p.n = !!(rd & 0x8000);
    regs.p.z = (rd == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

