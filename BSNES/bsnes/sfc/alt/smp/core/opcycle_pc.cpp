case 0x2f: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(0){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf0: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(!regs.p.z){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd0: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(regs.p.z){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb0: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(!regs.p.c){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x90: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(regs.p.c){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x70: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(!regs.p.v){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x50: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(regs.p.v){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x30: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(!regs.p.n){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x10: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    if(regs.p.n){ opcode_cycle = 0; break; }
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x03: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x01) != 0x01){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x13: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x01) == 0x01){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x23: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x02) != 0x02){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x33: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x02) == 0x02){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x43: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x04) != 0x04){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x53: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x04) == 0x04){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x63: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x08) != 0x08){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x73: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x08) == 0x08){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x83: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x10) != 0x10){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x93: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x10) == 0x10){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa3: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x20) != 0x20){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb3: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x20) == 0x20){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc3: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x40) != 0x40){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd3: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x40) == 0x40){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe3: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x80) != 0x80){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf3: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if((sp & 0x80) == 0x80){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x2e: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    sp = op_readdp(dp);
    break;
  case 3:
    rd = op_readpc();
    break;
  case 4:
    op_io();
    if(regs.a == sp){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xde: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    sp = op_readdp(dp + regs.x);
    break;
  case 4:
    rd = op_readpc();
    break;
  case 5:
    op_io();
    if(regs.a == sp){ opcode_cycle = 0; break; }
    break;
  case 6:
    op_io();
    break;
  case 7:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x6e: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    wr = op_readdp(dp);
    break;
  case 3:
    op_writedp(dp, --wr);
    break;
  case 4:
    rd = op_readpc();
    if(wr == 0x00){ opcode_cycle = 0; break; }
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xfe: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    op_io();
    regs.y--;
    break;
  case 3:
    op_io();
    if(regs.y == 0x00){ opcode_cycle = 0; break; }
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    regs.pc += (int8)rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x5f: {
  switch(opcode_cycle++) {
  case 1:
    rd  = op_readpc();
    break;
  case 2:
    rd |= op_readpc() << 8;
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x1f: {
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
    rd  = op_readaddr(dp);
    break;
  case 5:
    rd |= op_readaddr(dp + 1) << 8;
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x3f: {
  switch(opcode_cycle++) {
  case 1:
    rd  = op_readpc();
    break;
  case 2:
    rd |= op_readpc() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x4f: {
  switch(opcode_cycle++) {
  case 1:
    rd = op_readpc();
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_writestack(regs.pc >> 8);
    break;
  case 5:
    op_writestack(regs.pc);
    regs.pc = 0xff00 | rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x01: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (0 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x11: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (1 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x21: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (2 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x31: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (3 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x41: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (4 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x51: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (5 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x61: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (6 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x71: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (7 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x81: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (8 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x91: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (9 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa1: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (10 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb1: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (11 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc1: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (12 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd1: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (13 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe1: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (14 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf1: {
  switch(opcode_cycle++) {
  case 1:
    dp = 0xffde - (15 << 1);
    rd  = op_readaddr(dp);
    break;
  case 2:
    rd |= op_readaddr(dp + 1) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    break;
  case 6:
    op_writestack(regs.pc >> 8);
    break;
  case 7:
    op_writestack(regs.pc);
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x0f: {
  switch(opcode_cycle++) {
  case 1:
    rd  = op_readaddr(0xffde);
    break;
  case 2:
    rd |= op_readaddr(0xffdf) << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_writestack(regs.pc >> 8);
    break;
  case 6:
    op_writestack(regs.pc);
    break;
  case 7:
    op_writestack(regs.p);
    regs.pc = rd;
    regs.p.b = 1;
    regs.p.i = 0;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x6f: {
  switch(opcode_cycle++) {
  case 1:
    rd  = op_readstack();
    break;
  case 2:
    rd |= op_readstack() << 8;
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x7f: {
  switch(opcode_cycle++) {
  case 1:
    regs.p = op_readstack();
    break;
  case 2:
    rd  = op_readstack();
    break;
  case 3:
    rd |= op_readstack() << 8;
    break;
  case 4:
    op_io();
    break;
  case 5:
    op_io();
    regs.pc = rd;
    opcode_cycle = 0;
    break;
  }
  break;
}

