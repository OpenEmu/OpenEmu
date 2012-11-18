uint8 PPU::read(uint32 addr) {
  switch(addr) {

  //DISPCNT
  case 0x04000000: return regs.control >> 0;
  case 0x04000001: return regs.control >> 8;

  //GRSWP
  case 0x04000002: return regs.greenswap;
  case 0x04000003: return 0u;

  //DISPSTAT
  case 0x04000004: return regs.status >> 0;
  case 0x04000005: return regs.status >> 8;

  //VCOUNT
  case 0x04000006: return regs.vcounter >> 0;
  case 0x04000007: return regs.vcounter >> 8;

  //BG0CNT,BG1CNT,BG2CNT,BG3CNT
  case 0x04000008: case 0x04000009:
  case 0x0400000a: case 0x0400000b:
  case 0x0400000c: case 0x0400000d:
  case 0x0400000e: case 0x0400000f: {
    auto &bg = regs.bg[(addr >> 1) & 3];
    unsigned shift = (addr & 1) * 8;
    return bg.control >> shift;
  }

  //WININ
  case 0x04000048: return regs.windowflags[In0];
  case 0x04000049: return regs.windowflags[In1];
  case 0x0400004a: return regs.windowflags[Out];
  case 0x0400004b: return regs.windowflags[Obj];

  //BLTCNT
  case 0x04000050: return regs.blend.control >> 0;
  case 0x04000051: return regs.blend.control >> 8;

  }

  return 0u;
}

void PPU::write(uint32 addr, uint8 byte) {
  switch(addr) {

  //DISPCNT
  case 0x04000000: regs.control = (regs.control & 0xff00) | (byte << 0); return;
  case 0x04000001: regs.control = (regs.control & 0x00ff) | (byte << 8); return;

  //GRSWP
  case 0x04000002: regs.greenswap = byte >> 0; return;
  case 0x04000003: return;

  //DISPSTAT
  case 0x04000004:
    regs.status.irqvblank       = byte >> 3;
    regs.status.irqhblank       = byte >> 4;
    regs.status.irqvcoincidence = byte >> 5;
    return;
  case 0x04000005:
    regs.status.vcompare = byte;
    return;

  //BG0CNT,BG1CNT,BG2CNT,BG3CNT
  case 0x04000008: case 0x04000009:
  case 0x0400000a: case 0x0400000b:
  case 0x0400000c: case 0x0400000d:
  case 0x0400000e: case 0x0400000f: {
    auto &bg = regs.bg[(addr >> 1) & 3];
    unsigned shift = (addr & 1) * 8;
    bg.control = (bg.control & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG0HOFS,BG1HOFS,BG2BOFS,BG3HOFS
  case 0x04000010: case 0x04000011:
  case 0x04000014: case 0x04000015:
  case 0x04000018: case 0x04000019:
  case 0x0400001c: case 0x0400001d: {
    auto &bg = regs.bg[(addr >> 2) & 3];
    unsigned shift = (addr & 1) * 8;
    bg.hoffset = (bg.hoffset & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG0VOFS,BG1VOFS,BG2VOFS,BG3VOFS
  case 0x04000012: case 0x04000013:
  case 0x04000016: case 0x04000017:
  case 0x0400001a: case 0x0400001b:
  case 0x0400001e: case 0x0400001f: {
    auto &bg = regs.bg[(addr >> 2) & 3];
    unsigned shift = (addr & 1) * 8;
    bg.voffset = (bg.voffset & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG2PA,BG3PA
  case 0x04000020: case 0x04000021:
  case 0x04000030: case 0x04000031: {
    auto &bg = regs.bg[(addr >> 4) & 3];
    unsigned shift = (addr & 1) * 8;
    bg.pa = (bg.pa & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG2PB,BG3PB
  case 0x04000022: case 0x04000023:
  case 0x04000032: case 0x04000033: {
    auto &bg = regs.bg[(addr >> 4) & 3];
    unsigned shift = (addr & 1) * 8;
    bg.pb = (bg.pb & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG2PC,BG3PC
  case 0x04000024: case 0x04000025:
  case 0x04000034: case 0x04000035: {
    auto &bg = regs.bg[(addr >> 4) & 3];
    unsigned shift = (addr & 1) * 8;
    bg.pc = (bg.pc & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG2PD,BG3PD
  case 0x04000026: case 0x04000027:
  case 0x04000036: case 0x04000037: {
    auto &bg = regs.bg[(addr >> 4) & 3];
    unsigned shift = (addr & 1) * 8;
    bg.pd = (bg.pd & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG2X_L,BG2X_H,BG3X_L,BG3X_H
  case 0x04000028: case 0x04000029: case 0x0400002a: case 0x0400002b:
  case 0x04000038: case 0x04000039: case 0x0400003a: case 0x0400003b: {
    auto &bg = regs.bg[(addr >> 4) & 3];
    unsigned shift = (addr & 3) * 8;
    bg.lx = bg.x = (bg.x & ~(255 << shift)) | (byte << shift);
    return;
  }

  //BG2Y_L,BG2Y_H,BG3Y_L,BG3Y_H
  case 0x0400002c: case 0x0400002d: case 0x0400002e: case 0x0400002f:
  case 0x0400003c: case 0x0400003d: case 0x0400003e: case 0x0400003f: {
    auto &bg = regs.bg[(addr >> 4) & 3];
    unsigned shift = (addr & 3) * 8;
    bg.ly = bg.y = (bg.y & ~(255 << shift)) | (byte << shift);
    return;
  }

  //WIN0H
  case 0x04000040: regs.window[0].x2 = byte; return;
  case 0x04000041: regs.window[0].x1 = byte; return;

  //WIN1H
  case 0x04000042: regs.window[1].x2 = byte; return;
  case 0x04000043: regs.window[1].x1 = byte; return;

  //WIN0V
  case 0x04000044: regs.window[0].y2 = byte; return;
  case 0x04000045: regs.window[0].y1 = byte; return;

  //WIN1V
  case 0x04000046: regs.window[1].y2 = byte; return;
  case 0x04000047: regs.window[1].y1 = byte; return;

  //WININ
  case 0x04000048: regs.windowflags[In0] = byte; return;
  case 0x04000049: regs.windowflags[In1] = byte; return;

  //WINOUT
  case 0x0400004a: regs.windowflags[Out] = byte; return;
  case 0x0400004b: regs.windowflags[Obj] = byte; return;

  //MOSAIC
  case 0x0400004c:
    regs.mosaic.bghsize = byte >> 0;
    regs.mosaic.bgvsize = byte >> 4;
    return;
  case 0x0400004d:
    regs.mosaic.objhsize = byte >> 0;
    regs.mosaic.objvsize = byte >> 4;
    return;

  //BLDCNT
  case 0x04000050: regs.blend.control = (regs.blend.control & 0xff00) | (byte << 0); return;
  case 0x04000051: regs.blend.control = (regs.blend.control & 0x00ff) | (byte << 8); return;

  //BLDALPHA
  case 0x04000052: regs.blend.eva = std::min(16, byte & 0x1f); return;
  case 0x04000053: regs.blend.evb = std::min(16, byte & 0x1f); return;

  //BLDY
  case 0x04000054: regs.blend.evy = std::min(16, byte & 0x1f); return;
  case 0x04000055: return;

  }
}
