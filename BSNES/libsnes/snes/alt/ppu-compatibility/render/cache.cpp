#ifdef PPU_CPP

#define render_bg_tile_line_2bpp(mask) \
  col  = !!(d0 & mask) << 0; \
  col += !!(d1 & mask) << 1; \
  *dest++ = col

#define render_bg_tile_line_4bpp(mask) \
  col  = !!(d0 & mask) << 0; \
  col += !!(d1 & mask) << 1; \
  col += !!(d2 & mask) << 2; \
  col += !!(d3 & mask) << 3; \
  *dest++ = col

#define render_bg_tile_line_8bpp(mask) \
  col  = !!(d0 & mask) << 0; \
  col += !!(d1 & mask) << 1; \
  col += !!(d2 & mask) << 2; \
  col += !!(d3 & mask) << 3; \
  col += !!(d4 & mask) << 4; \
  col += !!(d5 & mask) << 5; \
  col += !!(d6 & mask) << 6; \
  col += !!(d7 & mask) << 7; \
  *dest++ = col

template<unsigned color_depth>
void PPU::render_bg_tile(uint16 tile_num) {
  uint8 col, d0, d1, d2, d3, d4, d5, d6, d7;

  if(color_depth == COLORDEPTH_4) {
    uint8 *dest = (uint8*)bg_tiledata[TILE_2BIT] + tile_num * 64;
    unsigned pos = tile_num * 16;
    unsigned y = 8;
    while(y--) {
      d0 = vram[pos    ];
      d1 = vram[pos + 1];
      render_bg_tile_line_2bpp(0x80);
      render_bg_tile_line_2bpp(0x40);
      render_bg_tile_line_2bpp(0x20);
      render_bg_tile_line_2bpp(0x10);
      render_bg_tile_line_2bpp(0x08);
      render_bg_tile_line_2bpp(0x04);
      render_bg_tile_line_2bpp(0x02);
      render_bg_tile_line_2bpp(0x01);
      pos += 2;
    }
    bg_tiledata_state[TILE_2BIT][tile_num] = 0;
  }

  if(color_depth == COLORDEPTH_16) {
    uint8 *dest = (uint8*)bg_tiledata[TILE_4BIT] + tile_num * 64;
    unsigned pos = tile_num * 32;
    unsigned y = 8;
    while(y--) {
      d0 = vram[pos     ];
      d1 = vram[pos +  1];
      d2 = vram[pos + 16];
      d3 = vram[pos + 17];
      render_bg_tile_line_4bpp(0x80);
      render_bg_tile_line_4bpp(0x40);
      render_bg_tile_line_4bpp(0x20);
      render_bg_tile_line_4bpp(0x10);
      render_bg_tile_line_4bpp(0x08);
      render_bg_tile_line_4bpp(0x04);
      render_bg_tile_line_4bpp(0x02);
      render_bg_tile_line_4bpp(0x01);
      pos += 2;
    }
    bg_tiledata_state[TILE_4BIT][tile_num] = 0;
  }

  if(color_depth == COLORDEPTH_256) {
    uint8 *dest = (uint8*)bg_tiledata[TILE_8BIT] + tile_num * 64;
    unsigned pos = tile_num * 64;
    unsigned y = 8;
    while(y--) {
      d0 = vram[pos     ];
      d1 = vram[pos +  1];
      d2 = vram[pos + 16];
      d3 = vram[pos + 17];
      d4 = vram[pos + 32];
      d5 = vram[pos + 33];
      d6 = vram[pos + 48];
      d7 = vram[pos + 49];
      render_bg_tile_line_8bpp(0x80);
      render_bg_tile_line_8bpp(0x40);
      render_bg_tile_line_8bpp(0x20);
      render_bg_tile_line_8bpp(0x10);
      render_bg_tile_line_8bpp(0x08);
      render_bg_tile_line_8bpp(0x04);
      render_bg_tile_line_8bpp(0x02);
      render_bg_tile_line_8bpp(0x01);
      pos += 2;
    }
    bg_tiledata_state[TILE_8BIT][tile_num] = 0;
  }
}

#undef render_bg_tile_line_2bpp
#undef render_bg_tile_line_4bpp
#undef render_bg_tile_line_8bpp

void PPU::flush_pixel_cache() {
  uint16 main = get_palette(0);
  uint16 sub  = (regs.pseudo_hires || regs.bg_mode == 5 || regs.bg_mode == 6)
              ? main
              : regs.color_rgb;

  unsigned i = 255;
  do {
    pixel_cache[i].src_main = main;
    pixel_cache[i].src_sub  = sub;
    pixel_cache[i].bg_main  = BACK;
    pixel_cache[i].bg_sub   = BACK;
    pixel_cache[i].ce_main  = false;
    pixel_cache[i].ce_sub   = false;
    pixel_cache[i].pri_main = 0;
    pixel_cache[i].pri_sub  = 0;
  } while(i--);
}

void PPU::alloc_tiledata_cache() {
  bg_tiledata[TILE_2BIT]       = new uint8_t[262144]();
  bg_tiledata[TILE_4BIT]       = new uint8_t[131072]();
  bg_tiledata[TILE_8BIT]       = new uint8_t[ 65536]();
  bg_tiledata_state[TILE_2BIT] = new uint8_t[  4096]();
  bg_tiledata_state[TILE_4BIT] = new uint8_t[  2048]();
  bg_tiledata_state[TILE_8BIT] = new uint8_t[  1024]();
}

//marks all tiledata cache entries as dirty
void PPU::flush_tiledata_cache() {
  for(unsigned i = 0; i < 4096; i++) bg_tiledata_state[TILE_2BIT][i] = 1;
  for(unsigned i = 0; i < 2048; i++) bg_tiledata_state[TILE_4BIT][i] = 1;
  for(unsigned i = 0; i < 1024; i++) bg_tiledata_state[TILE_8BIT][i] = 1;
}

void PPU::free_tiledata_cache() {
  delete[] bg_tiledata[TILE_2BIT];
  delete[] bg_tiledata[TILE_4BIT];
  delete[] bg_tiledata[TILE_8BIT];
  delete[] bg_tiledata_state[TILE_2BIT];
  delete[] bg_tiledata_state[TILE_4BIT];
  delete[] bg_tiledata_state[TILE_8BIT];
}

#endif
