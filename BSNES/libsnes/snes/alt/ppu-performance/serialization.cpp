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
  Processor::serialize(s);
  PPUcounter::serialize(s);

  s.array(vram);
  s.array(oam);
  s.array(cgram);

  cache.serialize(s);
  bg1.serialize(s);
  bg2.serialize(s);
  bg3.serialize(s);
  bg4.serialize(s);
  sprite.serialize(s);
  screen.serialize(s);

  s.integer(display.interlace);
  s.integer(display.overscan);
  s.integer(display.width);
  s.integer(display.height);

  s.integer(regs.ppu1_mdr);
  s.integer(regs.ppu2_mdr);

  s.integer(regs.vram_readbuffer);
  s.integer(regs.oam_latchdata);
  s.integer(regs.cgram_latchdata);
  s.integer(regs.bgofs_latchdata);
  s.integer(regs.mode7_latchdata);

  s.integer(regs.counters_latched);
  s.integer(regs.latch_hcounter);
  s.integer(regs.latch_vcounter);

  s.integer(regs.display_disable);
  s.integer(regs.display_brightness);

  s.integer(regs.oam_baseaddr);
  s.integer(regs.oam_addr);
  s.integer(regs.oam_priority);

  s.integer(regs.bg3_priority);
  s.integer(regs.bgmode);

  s.integer(regs.mode7_hoffset);

  s.integer(regs.mode7_voffset);

  s.integer(regs.vram_incmode);
  s.integer(regs.vram_mapping);
  s.integer(regs.vram_incsize);

  s.integer(regs.vram_addr);

  s.integer(regs.mode7_repeat);
  s.integer(regs.mode7_vflip);
  s.integer(regs.mode7_hflip);

  s.integer(regs.m7a);
  s.integer(regs.m7b);
  s.integer(regs.m7c);
  s.integer(regs.m7d);
  s.integer(regs.m7x);
  s.integer(regs.m7y);

  s.integer(regs.cgram_addr);

  s.integer(regs.window_one_left);
  s.integer(regs.window_one_right);
  s.integer(regs.window_two_left);
  s.integer(regs.window_two_right);

  s.integer(regs.mode7_extbg);
  s.integer(regs.pseudo_hires);
  s.integer(regs.overscan);
  s.integer(regs.interlace);

  s.integer(regs.hcounter);

  s.integer(regs.vcounter);
}

void PPU::Cache::serialize(serializer &s) {
  //rather than save ~512KB worth of cached tiledata, invalidate it all
  for(unsigned i = 0; i < 4096; i++) tilevalid[0][i] = false;
  for(unsigned i = 0; i < 2048; i++) tilevalid[1][i] = false;
  for(unsigned i = 0; i < 1024; i++) tilevalid[2][i] = false;
}

void PPU::Background::serialize(serializer &s) {
  s.integer(regs.mode);
  s.integer(regs.priority0);
  s.integer(regs.priority1);

  s.integer(regs.tile_size);
  s.integer(regs.mosaic);

  s.integer(regs.screen_addr);
  s.integer(regs.screen_size);
  s.integer(regs.tiledata_addr);

  s.integer(regs.hoffset);
  s.integer(regs.voffset);

  s.integer(regs.main_enable);
  s.integer(regs.sub_enable);

  s.integer(hires);
  s.integer(width);

  s.integer(tile_width);
  s.integer(tile_height);

  s.integer(mask_x);
  s.integer(mask_y);

  s.integer(scx);
  s.integer(scy);

  s.integer(hscroll);
  s.integer(vscroll);

  s.integer(mosaic_vcounter);
  s.integer(mosaic_voffset);

  window.serialize(s);
}

void PPU::Sprite::serialize(serializer &s) {
  s.integer(regs.priority0);
  s.integer(regs.priority1);
  s.integer(regs.priority2);
  s.integer(regs.priority3);

  s.integer(regs.base_size);
  s.integer(regs.nameselect);
  s.integer(regs.tiledata_addr);
  s.integer(regs.first_sprite);

  s.integer(regs.main_enable);
  s.integer(regs.sub_enable);

  s.integer(regs.interlace);

  s.integer(regs.time_over);
  s.integer(regs.range_over);

  for(unsigned i = 0; i < 128; i++) {
    s.integer(list[i].width);
    s.integer(list[i].height);
    s.integer(list[i].x);
    s.integer(list[i].y);
    s.integer(list[i].character);
    s.integer(list[i].use_nameselect);
    s.integer(list[i].vflip);
    s.integer(list[i].hflip);
    s.integer(list[i].palette);
    s.integer(list[i].priority);
    s.integer(list[i].size);
  }
  s.integer(list_valid);

  s.array(itemlist);
  for(unsigned i = 0; i < 34; i++) {
    s.integer(tilelist[i].x);
    s.integer(tilelist[i].y);
    s.integer(tilelist[i].priority);
    s.integer(tilelist[i].palette);
    s.integer(tilelist[i].tile);
    s.integer(tilelist[i].hflip);
  }

  s.array(output.palette);
  s.array(output.priority);

  window.serialize(s);
}

void PPU::Screen::serialize(serializer &s) {
  s.integer(regs.addsub_mode);
  s.integer(regs.direct_color);

  s.integer(regs.color_mode);
  s.integer(regs.color_halve);
  s.array(regs.color_enable);

  s.integer(regs.color_b);
  s.integer(regs.color_g);
  s.integer(regs.color_r);
  s.integer(regs.color);

  for(unsigned i = 0; i < 256; i++) {
    s.integer(output.main[i].color);
    s.integer(output.main[i].priority);
    s.integer(output.main[i].source);

    s.integer(output.sub[i].color);
    s.integer(output.sub[i].priority);
    s.integer(output.sub[i].source);
  }

  window.serialize(s);
}

void PPU::LayerWindow::serialize(serializer &s) {
  s.integer(one_enable);
  s.integer(one_invert);
  s.integer(two_enable);
  s.integer(two_invert);

  s.integer(mask);

  s.integer(main_enable);
  s.integer(sub_enable);

  s.array(main);
  s.array(sub);
}

void PPU::ColorWindow::serialize(serializer &s) {
  s.integer(one_enable);
  s.integer(one_invert);
  s.integer(two_enable);
  s.integer(two_invert);

  s.integer(mask);

  s.integer(main_mask);
  s.integer(sub_mask);

  s.array(main);
  s.array(sub);
}

#endif
