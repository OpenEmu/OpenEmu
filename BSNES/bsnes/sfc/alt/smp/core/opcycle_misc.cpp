case 0x00: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xef: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    regs.pc--;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xff: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    regs.pc--;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x9f: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_io();
    break;
  case 4:
    op_io();
    regs.a = (regs.a >> 4) | (regs.a << 4);
    regs.p.n = !!(regs.a & 0x80);
    regs.p.z = (regs.a == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xdf: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
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
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xbe: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
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
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x60: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.p.c = 0;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x20: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.p.p = 0;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x80: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.p.c = 1;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x40: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.p.p = 1;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe0: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    regs.p.v = 0;
    regs.p.h = 0;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xed: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    regs.p.c = !regs.p.c;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa0: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    regs.p.i = 1;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc0: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    regs.p.i = 0;
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x02: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x01;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x12: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x01;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x22: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x02;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x32: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x02;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x42: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x04;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x52: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x04;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x62: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x08;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x72: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x08;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x82: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x10;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x92: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x10;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xa2: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x20;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xb2: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x20;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xc2: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x40;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xd2: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x40;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xe2: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd |=  0x80;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xf2: {
  switch(opcode_cycle++) {
  case 1:
    dp = op_readpc();
    break;
  case 2:
    rd = op_readdp(dp);
    break;
  case 3:
    rd &= ~0x80;
    op_writedp(dp, rd);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x2d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_writestack(regs.a);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x4d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_writestack(regs.x);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x6d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_writestack(regs.y);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x0d: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    op_writestack(regs.p);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xae: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    regs.a = op_readstack();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xce: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    regs.x = op_readstack();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xee: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    regs.y = op_readstack();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x8e: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
    break;
  case 3:
    regs.p = op_readstack();
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0xcf: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
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
    op_io();
    break;
  case 7:
    op_io();
    break;
  case 8:
    op_io();
    ya = regs.y * regs.a;
    regs.a = ya;
    regs.y = ya >> 8;
    //result is set based on y (high-byte) only
    regs.p.n = !!(regs.y & 0x80);
    regs.p.z = (regs.y == 0);
    opcode_cycle = 0;
    break;
  }
  break;
}

case 0x9e: {
  switch(opcode_cycle++) {
  case 1:
    op_io();
    break;
  case 2:
    op_io();
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
    op_io();
    break;
  case 7:
    op_io();
    break;
  case 8:
    op_io();
    break;
  case 9:
    op_io();
    break;
  case 10:
    op_io();
    break;
  case 11:
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
    opcode_cycle = 0;
    break;
  }
  break;
}

