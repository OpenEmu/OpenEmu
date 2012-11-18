void PPU::serialize(serializer &s) {
  Thread::serialize(s);

  s.integer(status.mdr);

  s.integer(status.field);
  s.integer(status.lx);
  s.integer(status.ly);

  s.integer(status.bus_data);

  s.integer(status.address_latch);

  s.integer(status.vaddr);
  s.integer(status.taddr);
  s.integer(status.xaddr);

  s.integer(status.nmi_hold);
  s.integer(status.nmi_flag);

  s.integer(status.nmi_enable);
  s.integer(status.master_select);
  s.integer(status.sprite_size);
  s.integer(status.bg_addr);
  s.integer(status.sprite_addr);
  s.integer(status.vram_increment);

  s.integer(status.emphasis);
  s.integer(status.sprite_enable);
  s.integer(status.bg_enable);
  s.integer(status.sprite_edge_enable);
  s.integer(status.bg_edge_enable);
  s.integer(status.grayscale);

  s.integer(status.sprite_zero_hit);
  s.integer(status.sprite_overflow);

  s.integer(status.oam_addr);

  s.integer(raster.nametable);
  s.integer(raster.attribute);
  s.integer(raster.tiledatalo);
  s.integer(raster.tiledatahi);

  s.integer(raster.oam_iterator);
  s.integer(raster.oam_counter);

  for(unsigned n = 0; n < 8; n++) {
    s.integer(raster.oam[n].id);
    s.integer(raster.oam[n].y);
    s.integer(raster.oam[n].tile);
    s.integer(raster.oam[n].attr);
    s.integer(raster.oam[n].x);

    s.integer(raster.oam[n].tiledatalo);
    s.integer(raster.oam[n].tiledatahi);
  }

  for(unsigned n = 0; n < 8; n++) {
    s.integer(raster.soam[n].id);
    s.integer(raster.soam[n].y);
    s.integer(raster.soam[n].tile);
    s.integer(raster.soam[n].attr);
    s.integer(raster.soam[n].x);

    s.integer(raster.soam[n].tiledatalo);
    s.integer(raster.soam[n].tiledatahi);
  }

  s.array(buffer);
  s.array(ciram);
  s.array(cgram);
  s.array(oam);
}
