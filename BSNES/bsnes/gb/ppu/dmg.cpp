#ifdef PPU_CPP

void PPU::dmg_render() {
  for(unsigned n = 0; n < 160; n++) {
    line[n] = 0x00;
    origin[n] = Origin::None;
  }

  if(status.display_enable) {
    if(status.bg_enable) dmg_render_bg();
    if(status.window_display_enable) dmg_render_window();
    if(status.ob_enable) dmg_render_ob();
  }

  uint32 *output = screen + status.ly * 160;
  for(unsigned n = 0; n < 160; n++) output[n] = video.palette[line[n]];
  interface->lcdScanline();
}

uint16 PPU::dmg_read_tile(bool select, unsigned x, unsigned y) {
  unsigned tmaddr = 0x1800 + (select << 10), tdaddr;
  tmaddr += (((y >> 3) << 5) + (x >> 3)) & 0x03ff;
  if(status.bg_tiledata_select == 0) {
    tdaddr = 0x1000 + ((int8)vram[tmaddr] << 4);
  } else {
    tdaddr = 0x0000 + (vram[tmaddr] << 4);
  }
  tdaddr += (y & 7) << 1;
  return (vram[tdaddr + 0] << 0) | (vram[tdaddr + 1] << 8);
}

void PPU::dmg_render_bg() {
  unsigned iy = (status.ly + status.scy) & 255;
  unsigned ix = status.scx, tx = ix & 7;
  unsigned data = dmg_read_tile(status.bg_tilemap_select, ix, iy);

  for(unsigned ox = 0; ox < 160; ox++) {
    uint8 palette = ((data & (0x0080 >> tx)) ? 1 : 0)
                  | ((data & (0x8000 >> tx)) ? 2 : 0);

    line[ox] = bgp[palette];
    origin[ox] = Origin::BG;

    ix = (ix + 1) & 255;
    tx = (tx + 1) & 7;

    if(tx == 0) data = dmg_read_tile(status.bg_tilemap_select, ix, iy);
  }
}

void PPU::dmg_render_window() {
  if(status.ly - status.wy >= 144u) return;
  if(status.wx >= 167u) return;
  unsigned iy = status.wyc++;
  unsigned ix = (7 - status.wx) & 255, tx = ix & 7;
  unsigned data = dmg_read_tile(status.window_tilemap_select, ix, iy);

  for(unsigned ox = 0; ox < 160; ox++) {
    uint8 palette = ((data & (0x0080 >> tx)) ? 1 : 0)
                  | ((data & (0x8000 >> tx)) ? 2 : 0);
    if(ox - (status.wx - 7) < 160u) {
      line[ox] = bgp[palette];
      origin[ox] = Origin::BG;
    }

    ix = (ix + 1) & 255;
    tx = (tx + 1) & 7;

    if(tx == 0) data = dmg_read_tile(status.window_tilemap_select, ix, iy);
  }
}

//Attributes:
//0x80: 0 = OBJ above BG, 1 = BG above OBJ
//0x40: vertical flip
//0x20: horizontal flip
//0x10: palette#
void PPU::dmg_render_ob() {
  const unsigned Height = (status.ob_size == 0 ? 8 : 16);
  unsigned sprite[10], sprites = 0;

  //find first ten sprites on this scanline
  for(unsigned s = 0; s < 40; s++) {
    unsigned sy = oam[(s << 2) + 0] - 16;
    unsigned sx = oam[(s << 2) + 1] -  8;

    sy = status.ly - sy;
    if(sy >= Height) continue;

    sprite[sprites++] = s;
    if(sprites == 10) break;
  }

  //sort by X-coordinate, when equal, lower address comes first
  for(unsigned x = 0; x < sprites; x++) {
    for(unsigned y = x + 1; y < sprites; y++) {
      signed sx = oam[(sprite[x] << 2) + 1] - 8;
      signed sy = oam[(sprite[y] << 2) + 1] - 8;
      if(sy < sx) {
        sprite[x] ^= sprite[y];
        sprite[y] ^= sprite[x];
        sprite[x] ^= sprite[y];
      }
    }
  }

  //render backwards, so that first sprite has highest priority
  for(signed s = sprites - 1; s >= 0; s--) {
    unsigned n = sprite[s] << 2;
    unsigned sy = oam[n + 0] - 16;
    unsigned sx = oam[n + 1] -  8;
    unsigned tile = oam[n + 2] & ~status.ob_size;
    unsigned attr = oam[n + 3];

    sy = status.ly - sy;
    if(sy >= Height) continue;
    if(attr & 0x40) sy ^= (Height - 1);

    unsigned tdaddr = (tile << 4) + (sy << 1), data = 0;
    data |= vram[tdaddr++] << 0;
    data |= vram[tdaddr++] << 8;
    if(attr & 0x20) data = hflip(data);

    for(unsigned tx = 0; tx < 8; tx++) {
      uint8 palette = ((data & (0x0080 >> tx)) ? 1 : 0)
                    | ((data & (0x8000 >> tx)) ? 2 : 0);
      if(palette == 0) continue;

      palette = obp[(bool)(attr & 0x10)][palette];
      unsigned ox = sx + tx;

      if(ox < 160) {
        if(attr & 0x80) {
          if(origin[ox] == Origin::BG) {
            if(line[ox] > 0) continue;
          }
        }
        line[ox] = palette;
        origin[ox] = Origin::OB;
      }
    }
  }
}

#endif
