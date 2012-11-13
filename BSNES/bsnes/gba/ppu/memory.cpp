uint32 PPU::vram_read(uint32 addr, uint32 size) {
  addr &= (addr & 0x10000) ? 0x17fff : 0x0ffff;

  switch(size) {
  case Word:
    addr &= ~3;
    return vram[addr + 0] << 0 | vram[addr + 1] << 8 | vram[addr + 2] << 16 | vram[addr + 3] << 24;
  case Half:
    addr &= ~1;
    return vram[addr + 0] << 0 | vram[addr + 1] << 8;
  case Byte:
    return vram[addr];
  }
}

void PPU::vram_write(uint32 addr, uint32 size, uint32 word) {
  addr &= (addr & 0x10000) ? 0x17fff : 0x0ffff;

  switch(size) {
  case Word:
    addr &= ~3;
    vram[addr + 0] = word >>  0;
    vram[addr + 1] = word >>  8;
    vram[addr + 2] = word >> 16;
    vram[addr + 3] = word >> 24;
    break;
  case Half:
    addr &= ~1;
    vram[addr + 0] = word >>  0;
    vram[addr + 1] = word >>  8;
    break;
  case Byte:
    addr &= ~1;
    vram[addr + 0] = word;
    vram[addr + 1] = word;
    break;
  }
}

uint32 PPU::pram_read(uint32 addr, uint32 size) {
  if(size == Word) return pram_read(addr & ~2, Half) << 0 | pram_read(addr | 2, Half) << 16;
  if(size == Byte) return pram_read(addr, Half) >> ((addr & 1) * 8);
  return pram[addr >> 1 & 511];
}

void PPU::pram_write(uint32 addr, uint32 size, uint32 word) {
  if(size == Word) {
    pram_write(addr & ~2, Half, word >>  0);
    pram_write(addr |  2, Half, word >> 16);
    return;
  }

  if(size == Byte) {
    return pram_write(addr, Half, word << 8 | word << 0);
  }

  pram[addr >> 1 & 511] = word & 0x7fff;
}

uint32 PPU::oam_read(uint32 addr, uint32 size) {
  if(size == Word) return oam_read(addr & ~2, Half) << 0 | oam_read(addr | 2, Half) << 16;
  if(size == Byte) return oam_read(addr, Half) >> ((addr & 1) * 8);

  auto &obj = object[addr >> 3 & 127];
  auto &par = objectparam[addr >> 5 & 31];

  switch(addr & 6) {

  case 0: return (
    (obj.y          <<  0)
  | (obj.affine     <<  8)
  | (obj.affinesize <<  9)
  | (obj.mode       << 10)
  | (obj.mosaic     << 12)
  | (obj.colors     << 13)
  | (obj.shape      << 14)
  );

  case 2: return (
    (obj.x           <<  0)
  | (obj.affineparam <<  9)
  | (obj.hflip       << 12)
  | (obj.vflip       << 13)
  | (obj.size        << 14)
  );

  case 4: return (
    (obj.character <<  0)
  | (obj.priority  << 10)
  | (obj.palette   << 12)
  );

  case 6:
    switch(addr >> 3 & 3) {
    case 0: return par.pa;
    case 1: return par.pb;
    case 2: return par.pc;
    case 3: return par.pd;
    }

  }
}

void PPU::oam_write(uint32 addr, uint32 size, uint32 word) {
  if(size == Word) {
    oam_write(addr & ~2, Half, word >>  0);
    oam_write(addr |  2, Half, word >> 16);
    return;
  }

  if(size == Byte) {
    return oam_write(addr, Half, word << 8 | word << 0);
  }

  auto &obj = object[addr >> 3 & 127];
  auto &par = objectparam[addr >> 5 & 31];
  switch(addr & 6) {

  case 0:
    obj.y          = word >>  0;
    obj.affine     = word >>  8;
    obj.affinesize = word >>  9;
    obj.mode       = word >> 10;
    obj.mosaic     = word >> 12;
    obj.colors     = word >> 13;
    obj.shape      = word >> 14;
    break;

  case 2:
    obj.x           = word >>  0;
    obj.affineparam = word >>  9;
    obj.hflip       = word >> 12;
    obj.vflip       = word >> 13;
    obj.size        = word >> 14;
    break;

  case 4:
    obj.character = word >>  0;
    obj.priority  = word >> 10;
    obj.palette   = word >> 12;
    break;

  case 6:
    switch(addr >> 3 & 3) {
    case 0: par.pa = word; break;
    case 1: par.pb = word; break;
    case 2: par.pc = word; break;
    case 3: par.pd = word; break;
    }

  }

  static unsigned widths[] = {
     8, 16, 32, 64,
    16, 32, 32, 64,
     8,  8, 16, 32,
     8,  8,  8,  8,  //invalid modes
  };

  static unsigned heights[] = {
     8, 16, 32, 64,
     8,  8, 16, 32,
    16, 32, 32, 64,
     8,  8,  8,  8,  //invalid modes
  };

  obj.width  = widths [obj.shape * 4 + obj.size];
  obj.height = heights[obj.shape * 4 + obj.size];
}
