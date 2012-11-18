#ifdef PPU_CPP

#include "list.cpp"

void PPU::Sprite::address_reset() {
  self.regs.oam_addr = self.regs.oam_baseaddr;
  set_first_sprite();
}

void PPU::Sprite::set_first_sprite() {
  regs.first_sprite = (self.regs.oam_priority == false ? 0 : (self.regs.oam_addr >> 2) & 127);
}

void PPU::Sprite::frame() {
  regs.time_over = false;
  regs.range_over = false;
}

void PPU::Sprite::scanline() {
  t.x = 0;
  t.y = self.vcounter();

  t.item_count = 0;
  t.tile_count = 0;

  t.active = !t.active;
  auto oam_item = t.item[t.active];
  auto oam_tile = t.tile[t.active];

  if(t.y == (!self.regs.overscan ? 225 : 240) && self.regs.display_disable == false) address_reset();
  if(t.y >= (!self.regs.overscan ? 224 : 239)) return;

  memset(oam_item, 0xff, 32);  //default to invalid
  for(unsigned i = 0; i < 34; i++) oam_tile[i].x = 0xffff;  //default to invalid

  for(unsigned i = 0; i < 128; i++) {
    unsigned sprite = (regs.first_sprite + i) & 127;
    if(on_scanline(list[sprite]) == false) continue;
    if(t.item_count++ >= 32) break;
    oam_item[t.item_count - 1] = sprite;
  }

  if(t.item_count > 0 && oam_item[t.item_count - 1] != 0xff) {
    ppu.regs.oam_iaddr = 0x0200 + (oam_item[t.item_count - 1] >> 2);
  }
}

bool PPU::Sprite::on_scanline(SpriteItem &sprite) {
  if(sprite.x > 256 && (sprite.x + sprite.width() - 1) < 512) return false;
  signed height = (regs.interlace == false ? sprite.height() : (sprite.height() >> 1));
  if(t.y >= sprite.y && t.y < (sprite.y + height)) return true;
  if((sprite.y + height) >= 256 && t.y < ((sprite.y + height) & 255)) return true;
  return false;
}

void PPU::Sprite::run() {
  output.main.priority = 0;
  output.sub.priority = 0;

  auto oam_tile = t.tile[!t.active];
  unsigned priority_table[] = { regs.priority0, regs.priority1, regs.priority2, regs.priority3 };
  unsigned x = t.x++;

  for(unsigned n = 0; n < 34; n++) {
    auto tile = oam_tile[n];
    if(tile.x == 0xffff) break;

    int px = x - sclip<9>(tile.x);
    if(px & ~7) continue;

    unsigned mask = 0x80 >> (tile.hflip == false ? px : 7 - px);
    unsigned color;
    color  = ((bool)(tile.d0 & mask)) << 0;
    color |= ((bool)(tile.d1 & mask)) << 1;
    color |= ((bool)(tile.d2 & mask)) << 2;
    color |= ((bool)(tile.d3 & mask)) << 3;

    if(color) {
      if(regs.main_enable) {
        output.main.palette = tile.palette + color;
        output.main.priority = priority_table[tile.priority];
      }

      if(regs.sub_enable) {
        output.sub.palette = tile.palette + color;
        output.sub.priority = priority_table[tile.priority];
      }
    }
  }
}

void PPU::Sprite::tilefetch() {
  auto oam_item = t.item[t.active];
  auto oam_tile = t.tile[t.active];

  for(signed i = 31; i >= 0; i--) {
    if(oam_item[i] == 0xff) continue;
    auto sprite = list[oam_item[i]];

    unsigned tile_width = sprite.width() >> 3;
    signed x = sprite.x;
    signed y = (t.y - sprite.y) & 0xff;
    if(regs.interlace) y <<= 1;

    if(sprite.vflip) {
      if(sprite.width() == sprite.height()) {
        y = (sprite.height() - 1) - y;
      } else if(y < sprite.width()) {
        y = (sprite.width() - 1) - y;
      } else {
        y = sprite.width() + ((sprite.width() - 1) - (y - sprite.width()));
      }
    }

    if(regs.interlace) {
      y = (sprite.vflip == false ? y + self.field() : y - self.field());
    }

    x &= 511;
    y &= 255;

    uint16 tiledata_addr = regs.tiledata_addr;
    uint16 chrx = (sprite.character >> 0) & 15;
    uint16 chry = (sprite.character >> 4) & 15;
    if(sprite.nameselect) {
      tiledata_addr += (256 * 32) + (regs.nameselect << 13);
    }
    chry  += (y >> 3);
    chry  &= 15;
    chry <<= 4;

    for(unsigned tx = 0; tx < tile_width; tx++) {
      unsigned sx = (x + (tx << 3)) & 511;
      if(x != 256 && sx >= 256 && (sx + 7) < 512) continue;
      if(t.tile_count++ >= 34) break;

      unsigned n = t.tile_count - 1;
      oam_tile[n].x = sx;
      oam_tile[n].priority = sprite.priority;
      oam_tile[n].palette = 128 + (sprite.palette << 4);
      oam_tile[n].hflip = sprite.hflip;

      unsigned mx = (sprite.hflip == false) ? tx : ((tile_width - 1) - tx);
      unsigned pos = tiledata_addr + ((chry + ((chrx + mx) & 15)) << 5);
      uint16 addr = (pos & 0xffe0) + ((y & 7) * 2);

      oam_tile[n].d0 = ppu.vram[addr +  0];
      oam_tile[n].d1 = ppu.vram[addr +  1];
      self.add_clocks(2);

      oam_tile[n].d2 = ppu.vram[addr + 16];
      oam_tile[n].d3 = ppu.vram[addr + 17];
      self.add_clocks(2);
    }
  }

  if(t.tile_count < 34) self.add_clocks((34 - t.tile_count) * 4);
  regs.time_over  |= (t.tile_count > 34);
  regs.range_over |= (t.item_count > 32);
}

void PPU::Sprite::reset() {
  for(unsigned i = 0; i < 128; i++) {
    list[i].x = 0;
    list[i].y = 0;
    list[i].character = 0;
    list[i].nameselect = 0;
    list[i].vflip = 0;
    list[i].hflip = 0;
    list[i].priority = 0;
    list[i].palette = 0;
    list[i].size = 0;
  }
  synchronize();

  t.x = 0;
  t.y = 0;

  t.item_count = 0;
  t.tile_count = 0;

  t.active = 0;
  for(unsigned n = 0; n < 2; n++) {
    memset(t.item[n], 0, 32);
    for(unsigned i = 0; i < 34; i++) {
      t.tile[n][i].x = 0;
      t.tile[n][i].priority = 0;
      t.tile[n][i].palette = 0;
      t.tile[n][i].hflip = 0;
      t.tile[n][i].d0 = 0;
      t.tile[n][i].d1 = 0;
      t.tile[n][i].d2 = 0;
      t.tile[n][i].d3 = 0;
    }
  }

  regs.main_enable = random(false);
  regs.sub_enable = random(false);
  regs.interlace = random(false);

  regs.base_size = random(0);
  regs.nameselect = random(0);
  regs.tiledata_addr = (random(0x0000) & 3) << 14;
  regs.first_sprite = 0;

  regs.priority0 = 0;
  regs.priority1 = 0;
  regs.priority2 = 0;
  regs.priority3 = 0;

  regs.time_over = false;
  regs.range_over = false;

  output.main.palette = 0;
  output.main.priority = 0;
  output.sub.palette = 0;
  output.sub.priority = 0;
}

PPU::Sprite::Sprite(PPU &self) : self(self) {
}

#endif
