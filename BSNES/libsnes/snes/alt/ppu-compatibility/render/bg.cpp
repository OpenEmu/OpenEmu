#ifdef PPU_CPP

//called once at the start of every rendered scanline
void PPU::update_bg_info() {
  const unsigned hires = (regs.bg_mode == 5 || regs.bg_mode == 6);
  const unsigned width = (!hires ? 256 : 512);

  for(unsigned bg = 0; bg < 4; bg++) {
    bg_info[bg].th = (regs.bg_tilesize[bg] ? 4 : 3);
    bg_info[bg].tw = (hires ? 4 : bg_info[bg].th);

    bg_info[bg].mx = (bg_info[bg].th == 4 ? (width << 1) : width);
    bg_info[bg].my = bg_info[bg].mx;
    if(regs.bg_scsize[bg] & 0x01) bg_info[bg].mx <<= 1;
    if(regs.bg_scsize[bg] & 0x02) bg_info[bg].my <<= 1;
    bg_info[bg].mx--;
    bg_info[bg].my--;

    bg_info[bg].scy = (regs.bg_scsize[bg] & 0x02) ? (32 << 5) : 0;
    bg_info[bg].scx = (regs.bg_scsize[bg] & 0x01) ? (32 << 5) : 0;
    if(regs.bg_scsize[bg] == 3) bg_info[bg].scy <<= 1;
  }
}

template<unsigned bg>
uint16 PPU::bg_get_tile(uint16 x, uint16 y) {
  x = (x & bg_info[bg].mx) >> bg_info[bg].tw;
  y = (y & bg_info[bg].my) >> bg_info[bg].th;

  uint16 pos = ((y & 0x1f) << 5) + (x & 0x1f);
  if(y & 0x20) pos += bg_info[bg].scy;
  if(x & 0x20) pos += bg_info[bg].scx;

  const uint16 addr = regs.bg_scaddr[bg] + (pos << 1);
  return vram[addr] + (vram[addr + 1] << 8);
}

#define setpixel_main(x) \
  if(pixel_cache[x].pri_main < tile_pri) { \
    pixel_cache[x].pri_main = tile_pri; \
    pixel_cache[x].bg_main  = bg; \
    pixel_cache[x].src_main = col; \
    pixel_cache[x].ce_main  = false; \
  }

#define setpixel_sub(x) \
  if(pixel_cache[x].pri_sub < tile_pri) { \
    pixel_cache[x].pri_sub = tile_pri; \
    pixel_cache[x].bg_sub  = bg; \
    pixel_cache[x].src_sub = col; \
    pixel_cache[x].ce_sub  = false; \
  }

template<unsigned mode, unsigned bg, unsigned color_depth>
void PPU::render_line_bg(uint8 pri0_pos, uint8 pri1_pos) {
  if(layer_enabled[bg][0] == false) pri0_pos = 0;
  if(layer_enabled[bg][1] == false) pri1_pos = 0;
  if(pri0_pos + pri1_pos == 0) return;

  if(regs.bg_enabled[bg] == false && regs.bgsub_enabled[bg] == false) return;

  const bool bg_enabled    = regs.bg_enabled[bg];
  const bool bgsub_enabled = regs.bgsub_enabled[bg];

  const uint16 opt_valid_bit = (bg == BG1) ? 0x2000 : (bg == BG2) ? 0x4000 : 0x0000;
  const uint8  bgpal_index   = (mode == 0 ? (bg << 5) : 0);

  const uint8  pal_size  = 2 << color_depth;       //<<2 (*4), <<4 (*16), <<8 (*256)
  const uint16 tile_mask = 0x0fff >> color_depth;  //0x0fff, 0x07ff, 0x03ff
  //4 + color_depth = >>(4-6) -- / {16, 32, 64 } bytes/tile
  //index is a tile number count to add to base tile number
  const unsigned tiledata_index = regs.bg_tdaddr[bg] >> (4 + color_depth);

  const uint8 *bg_td       = bg_tiledata[color_depth];
  const uint8 *bg_td_state = bg_tiledata_state[color_depth];

  const uint8  tile_width  = bg_info[bg].tw;
  const uint8  tile_height = bg_info[bg].th;
  const uint16 mask_x      = bg_info[bg].mx;  //screen width  mask
  const uint16 mask_y      = bg_info[bg].my;  //screen height mask

  uint16 y       = regs.bg_y[bg];
  uint16 hscroll = regs.bg_hofs[bg];
  uint16 vscroll = regs.bg_vofs[bg];

  const unsigned hires = (mode == 5 || mode == 6);
  const unsigned width = (!hires ? 256 : 512);

  if(hires) {
    hscroll <<= 1;
    if(regs.interlace) y = (y << 1) + field();
  }

  uint16 hval, vval;
  uint16 tile_pri, tile_num;
  uint8  pal_index, pal_num;
  uint16 hoffset, voffset, opt_x, col;
  bool   mirror_x, mirror_y;

  const uint8  *tile_ptr;
  const uint16 *mtable = mosaic_table[regs.mosaic_enabled[bg] ? regs.mosaic_size : 0];
  const bool   is_opt_mode = (mode == 2 || mode == 4 || mode == 6);
  const bool   is_direct_color_mode = (regs.direct_color == true && bg == BG1 && (mode == 3 || mode == 4));

  build_window_tables(bg);
  const uint8 *wt_main = window[bg].main;
  const uint8 *wt_sub  = window[bg].sub;

  uint16 prev_x = 0xffff, prev_y = 0xffff, prev_optx = 0xffff;
  for(uint16 x = 0; x < width; x++) {
    hoffset = mtable[x] + hscroll;
    voffset = y + vscroll;

    if(is_opt_mode) {
      opt_x = (x + (hscroll & 7));

      //tile 0 is unaffected by OPT mode...
      if(opt_x >= 8) {
        //cache tile data in hval, vval if possible
        if((opt_x >> 3) != (prev_optx >> 3)) {
          prev_optx = opt_x;

          hval = bg_get_tile<BG3>((opt_x - 8) + (regs.bg_hofs[BG3] & ~7), regs.bg_vofs[BG3]);
          if(mode != 4) {
            vval = bg_get_tile<BG3>((opt_x - 8) + (regs.bg_hofs[BG3] & ~7), regs.bg_vofs[BG3] + 8);
          }
        }

        if(mode == 4) {
          if(hval & opt_valid_bit) {
            if(!(hval & 0x8000)) {
              hoffset = opt_x + (hval & ~7);
            } else {
              voffset = y + hval;
            }
          }
        } else {
          if(hval & opt_valid_bit) {
            hoffset = opt_x + (hval & ~7);
          }
          if(vval & opt_valid_bit) {
            voffset = y + vval;
          }
        }
      }
    }

    hoffset &= mask_x;
    voffset &= mask_y;

    if((hoffset >> 3) != prev_x || (voffset >> 3) != prev_y) {
      prev_x = (hoffset >> 3);
      prev_y = (voffset >> 3);

      tile_num  = bg_get_tile<bg>(hoffset, voffset);  //format = vhopppcc cccccccc
      mirror_y  = (tile_num & 0x8000);
      mirror_x  = (tile_num & 0x4000);
      tile_pri  = (tile_num & 0x2000) ? pri1_pos : pri0_pos;
      pal_num   = ((tile_num >> 10) & 7);
      pal_index = bgpal_index + (pal_num << pal_size);

      if(tile_width  == 4) {  //16x16 horizontal tile mirroring
        if((bool)(hoffset & 8) != mirror_x) tile_num++;
      }

      if(tile_height == 4) {  //16x16 vertical tile mirroring
        if((bool)(voffset & 8) != mirror_y) tile_num += 16;
      }

      tile_num &= 0x03ff;
      tile_num += tiledata_index;
      tile_num &= tile_mask;

      if(bg_td_state[tile_num] == 1) {
        render_bg_tile<color_depth>(tile_num);
      }

      if(mirror_y) voffset ^= 7;  //invert y tile pos
      tile_ptr = bg_td + (tile_num * 64) + ((voffset & 7) * 8);
    }

    if(mirror_x) hoffset ^= 7;  //invert x tile pos
    col = *(tile_ptr + (hoffset & 7));
    if(col) {
      if(is_direct_color_mode) {
        col = get_direct_color(pal_num, col);
      } else {
        col = get_palette(col + pal_index);
      }

      if(!hires) {
        if(bg_enabled    == true && !wt_main[x]) { setpixel_main(x); }
        if(bgsub_enabled == true && !wt_sub[x])  { setpixel_sub(x);  }
      } else {
        int hx = x >> 1;
        if(x & 1) {
          if(bg_enabled    == true && !wt_main[hx]) { setpixel_main(hx); }
        } else {
          if(bgsub_enabled == true && !wt_sub[hx])  { setpixel_sub(hx);  }
        }
      }
    }
  }
}

#undef setpixel_main
#undef setpixel_sub

#endif
