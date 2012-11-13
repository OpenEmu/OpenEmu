#ifdef PPU_CPP

void PPUcounter::serialize(serializer &s) {
  s.integer(status.interlace);
  s.integer(status.field);
  s.integer(status.vcounter);
  s.integer(status.hcounter);

  s.array(history.field);
  s.array(history.vcounter);
  s.array(history.hcounter);
  s.integer(history.index);
}

void PPU::serialize(serializer &s) {
  Thread::serialize(s);
  PPUcounter::serialize(s);

  s.array(vram);
  s.array(oam);
  s.array(cgram);

  s.integer(ppu1_version);
  s.integer(ppu2_version);

  s.integer(region);
  s.integer(line);

  s.integer(display.interlace);
  s.integer(display.overscan);

  s.integer(cache.oam_basesize);
  s.integer(cache.oam_nameselect);
  s.integer(cache.oam_tdaddr);

  s.integer(regs.ppu1_mdr);
  s.integer(regs.ppu2_mdr);
  for(unsigned n = 0; n < 4; n++) s.integer(regs.bg_y[n]);

  s.integer(regs.ioamaddr);
  s.integer(regs.icgramaddr);

  s.integer(regs.display_disabled);
  s.integer(regs.display_brightness);

  s.integer(regs.oam_basesize);
  s.integer(regs.oam_nameselect);
  s.integer(regs.oam_tdaddr);

  s.integer(regs.oam_baseaddr);
  s.integer(regs.oam_addr);
  s.integer(regs.oam_priority);
  s.integer(regs.oam_firstsprite);

  s.integer(regs.oam_latchdata);

  for(unsigned n = 0; n < 4; n++) s.integer(regs.bg_tilesize[n]);
  s.integer(regs.bg3_priority);
  s.integer(regs.bg_mode);

  s.integer(regs.mosaic_size);
  for(unsigned n = 0; n < 4; n++) s.integer(regs.mosaic_enabled[n]);
  s.integer(regs.mosaic_countdown);

  for(unsigned n = 0; n < 4; n++) s.integer(regs.bg_scaddr[n]);
  for(unsigned n = 0; n < 4; n++) s.integer(regs.bg_scsize[n]);

  for(unsigned n = 0; n < 4; n++) s.integer(regs.bg_tdaddr[n]);

  s.integer(regs.bg_ofslatch);
  s.integer(regs.m7_hofs);
  s.integer(regs.m7_vofs);
  for(unsigned n = 0; n < 4; n++) s.integer(regs.bg_hofs[n]);
  for(unsigned n = 0; n < 4; n++) s.integer(regs.bg_vofs[n]);

  s.integer(regs.vram_incmode);
  s.integer(regs.vram_mapping);
  s.integer(regs.vram_incsize);

  s.integer(regs.vram_addr);

  s.integer(regs.mode7_repeat);
  s.integer(regs.mode7_vflip);
  s.integer(regs.mode7_hflip);

  s.integer(regs.m7_latch);
  s.integer(regs.m7a);
  s.integer(regs.m7b);
  s.integer(regs.m7c);
  s.integer(regs.m7d);
  s.integer(regs.m7x);
  s.integer(regs.m7y);

  s.integer(regs.cgram_addr);

  s.integer(regs.cgram_latchdata);

  for(unsigned n = 0; n < 6; n++) s.integer(regs.window1_enabled[n]);
  for(unsigned n = 0; n < 6; n++) s.integer(regs.window1_invert [n]);
  for(unsigned n = 0; n < 6; n++) s.integer(regs.window2_enabled[n]);
  for(unsigned n = 0; n < 6; n++) s.integer(regs.window2_invert [n]);

  s.integer(regs.window1_left);
  s.integer(regs.window1_right);
  s.integer(regs.window2_left);
  s.integer(regs.window2_right);

  for(unsigned n = 0; n < 6; n++) s.integer(regs.window_mask[n]);
  for(unsigned n = 0; n < 5; n++) s.integer(regs.bg_enabled[n]);
  for(unsigned n = 0; n < 5; n++) s.integer(regs.bgsub_enabled[n]);
  for(unsigned n = 0; n < 5; n++) s.integer(regs.window_enabled[n]);
  for(unsigned n = 0; n < 5; n++) s.integer(regs.sub_window_enabled[n]);

  s.integer(regs.color_mask);
  s.integer(regs.colorsub_mask);
  s.integer(regs.addsub_mode);
  s.integer(regs.direct_color);

  s.integer(regs.color_mode);
  s.integer(regs.color_halve);
  for(unsigned n = 0; n < 6; n++) s.integer(regs.color_enabled[n]);

  s.integer(regs.color_r);
  s.integer(regs.color_g);
  s.integer(regs.color_b);
  s.integer(regs.color_rgb);

  s.integer(regs.mode7_extbg);
  s.integer(regs.pseudo_hires);
  s.integer(regs.overscan);
  s.integer(regs.scanlines);
  s.integer(regs.oam_interlace);
  s.integer(regs.interlace);

  s.integer(regs.hcounter);
  s.integer(regs.vcounter);
  s.integer(regs.latch_hcounter);
  s.integer(regs.latch_vcounter);
  s.integer(regs.counters_latched);

  s.integer(regs.vram_readbuffer);

  s.integer(regs.time_over);
  s.integer(regs.range_over);
  s.integer(regs.oam_itemcount);
  s.integer(regs.oam_tilecount);

  for(unsigned n = 0; n < 256; n++) {
    s.integer(pixel_cache[n].src_main);
    s.integer(pixel_cache[n].src_sub);
    s.integer(pixel_cache[n].bg_main);
    s.integer(pixel_cache[n].bg_sub);
    s.integer(pixel_cache[n].ce_main);
    s.integer(pixel_cache[n].ce_sub);
    s.integer(pixel_cache[n].pri_main);
    s.integer(pixel_cache[n].pri_sub);
  }

  //better to just take a small speed hit than store all of bg_tiledata[3][] ...
  flush_tiledata_cache();

  for(unsigned n = 0; n < 6; n++) {
    s.array(window[n].main, 256);
    s.array(window[n].sub,  256);
  }

  for(unsigned n = 0; n < 4; n++) {
    s.integer(bg_info[n].tw);
    s.integer(bg_info[n].th);
    s.integer(bg_info[n].mx);
    s.integer(bg_info[n].my);
    s.integer(bg_info[n].scx);
    s.integer(bg_info[n].scy);
  }

  for(unsigned n = 0; n < 128; n++) {
    s.integer(sprite_list[n].width);
    s.integer(sprite_list[n].height);
    s.integer(sprite_list[n].x);
    s.integer(sprite_list[n].y);
    s.integer(sprite_list[n].character);
    s.integer(sprite_list[n].use_nameselect);
    s.integer(sprite_list[n].vflip);
    s.integer(sprite_list[n].hflip);
    s.integer(sprite_list[n].palette);
    s.integer(sprite_list[n].priority);
    s.integer(sprite_list[n].size);
  }
  s.integer(sprite_list_valid);
  s.integer(active_sprite);

  s.array(oam_itemlist, 32);

  for(unsigned n = 0; n < 34; n++) {
    s.integer(oam_tilelist[n].x);
    s.integer(oam_tilelist[n].y);
    s.integer(oam_tilelist[n].pri);
    s.integer(oam_tilelist[n].pal);
    s.integer(oam_tilelist[n].tile);
    s.integer(oam_tilelist[n].hflip);
  }

  s.array(oam_line_pal, 256);
  s.array(oam_line_pri, 256);
}

#endif
