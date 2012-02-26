#ifdef PPU_CPP

void PPU::Sprite::update(unsigned addr, uint8 data) {
  ppu.oam[addr] = data;

  if(addr < 0x0200) {
    unsigned n = addr >> 2;
    addr &= 3;
    if(addr == 0) {
      list[n].x = (list[n].x & 0x100) | data;
    } else if(addr == 1) {
      list[n].y = data;
    } else if(addr == 2) {
      list[n].character = data;
    } else {  //(addr == 3)
      list[n].vflip = data & 0x80;
      list[n].hflip = data & 0x40;
      list[n].priority = (data >> 4) & 3;
      list[n].palette = (data >> 1) & 7;
      list[n].nameselect = data & 1;
    }
  } else {
    unsigned n = (addr & 0x1f) << 2;
    list[n + 0].x = ((data & 0x01) << 8) | (list[n + 0].x & 0xff);
    list[n + 0].size = data & 0x02;
    list[n + 1].x = ((data & 0x04) << 6) | (list[n + 1].x & 0xff);
    list[n + 1].size = data & 0x08;
    list[n + 2].x = ((data & 0x10) << 4) | (list[n + 2].x & 0xff);
    list[n + 2].size = data & 0x20;
    list[n + 3].x = ((data & 0x40) << 2) | (list[n + 3].x & 0xff);
    list[n + 3].size = data & 0x80;
  }
}

void PPU::Sprite::synchronize() {
  for(unsigned n = 0; n < 544; n++) update(n, ppu.oam[n]);
}

unsigned PPU::Sprite::SpriteItem::width() const {
  if(size == 0) {
    static unsigned width[] = {  8,  8,  8, 16, 16, 32, 16, 16 };
    return width[ppu.sprite.regs.base_size];
  } else {
    static unsigned width[] = { 16, 32, 64, 32, 64, 64, 32, 32 };
    return width[ppu.sprite.regs.base_size];
  }
}

unsigned PPU::Sprite::SpriteItem::height() const {
  if(size == 0) {
    if(ppu.sprite.regs.interlace && ppu.sprite.regs.base_size >= 6) return 16;
    static unsigned height[] = {  8,  8,  8, 16, 16, 32, 32, 32 };
    return height[ppu.sprite.regs.base_size];
  } else {
    static unsigned height[] = { 16, 32, 64, 32, 64, 64, 64, 32 };
    return height[ppu.sprite.regs.base_size];
  }
}

#endif
