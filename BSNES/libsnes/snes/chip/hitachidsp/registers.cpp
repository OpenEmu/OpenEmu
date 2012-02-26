#ifdef HITACHIDSP_CPP

unsigned HitachiDSP::reg_read(unsigned n) const {
  switch(n) {
  case 0x00: return regs.a;
  case 0x01: return regs.acch;
  case 0x02: return regs.accl;
  case 0x03: return regs.busdata;
  case 0x08: return regs.romdata;
  case 0x0c: return regs.ramdata;
  case 0x13: return regs.busaddr;
  case 0x1c: return regs.ramaddr;
  case 0x50: return 0x000000;
  case 0x51: return 0xffffff;
  case 0x52: return 0x00ff00;
  case 0x53: return 0xff0000;
  case 0x54: return 0x00ffff;
  case 0x55: return 0xffff00;
  case 0x56: return 0x800000;
  case 0x57: return 0x7fffff;
  case 0x58: return 0x008000;
  case 0x59: return 0x007fff;
  case 0x5a: return 0xff7fff;
  case 0x5b: return 0xffff7f;
  case 0x5c: return 0x010000;
  case 0x5d: return 0xfeffff;
  case 0x5e: return 0x000100;
  case 0x5f: return 0x00feff;
  case 0x60: return regs.gpr[ 0];
  case 0x61: return regs.gpr[ 1];
  case 0x62: return regs.gpr[ 2];
  case 0x63: return regs.gpr[ 3];
  case 0x64: return regs.gpr[ 4];
  case 0x65: return regs.gpr[ 5];
  case 0x66: return regs.gpr[ 6];
  case 0x67: return regs.gpr[ 7];
  case 0x68: return regs.gpr[ 8];
  case 0x69: return regs.gpr[ 9];
  case 0x6a: return regs.gpr[10];
  case 0x6b: return regs.gpr[11];
  case 0x6c: return regs.gpr[12];
  case 0x6d: return regs.gpr[13];
  case 0x6e: return regs.gpr[14];
  case 0x6f: return regs.gpr[15];
  }
  return 0x000000;
}

void HitachiDSP::reg_write(unsigned n, unsigned data) {
  switch(n) {
  case 0x00: regs.a = data; return;
  case 0x01: regs.acch = data; return;
  case 0x02: regs.accl = data; return;
  case 0x03: regs.busdata = data; return;
  case 0x08: regs.romdata = data; return;
  case 0x0c: regs.ramdata = data; return;
  case 0x13: regs.busaddr = data; return;
  case 0x1c: regs.ramaddr = data; return;
  case 0x60: regs.gpr[ 0] = data; return;
  case 0x61: regs.gpr[ 1] = data; return;
  case 0x62: regs.gpr[ 2] = data; return;
  case 0x63: regs.gpr[ 3] = data; return;
  case 0x64: regs.gpr[ 4] = data; return;
  case 0x65: regs.gpr[ 5] = data; return;
  case 0x66: regs.gpr[ 6] = data; return;
  case 0x67: regs.gpr[ 7] = data; return;
  case 0x68: regs.gpr[ 8] = data; return;
  case 0x69: regs.gpr[ 9] = data; return;
  case 0x6a: regs.gpr[10] = data; return;
  case 0x6b: regs.gpr[11] = data; return;
  case 0x6c: regs.gpr[12] = data; return;
  case 0x6d: regs.gpr[13] = data; return;
  case 0x6e: regs.gpr[14] = data; return;
  case 0x6f: regs.gpr[15] = data; return;
  }
}

#endif
