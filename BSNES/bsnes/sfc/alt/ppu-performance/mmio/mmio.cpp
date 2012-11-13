#ifdef PPU_CPP

void PPU::latch_counters() {
  regs.hcounter = cpu.hdot();
  regs.vcounter = cpu.vcounter();
  regs.counters_latched = true;
}

bool PPU::interlace() const { return display.interlace; }
bool PPU::overscan() const { return display.overscan; }
bool PPU::hires() const { return regs.pseudo_hires || regs.bgmode == 5 || regs.bgmode == 6; }

uint16 PPU::get_vram_addr() {
  uint16 addr = regs.vram_addr;
  switch(regs.vram_mapping) {
    case 0: break;
    case 1: addr = (addr & 0xff00) | ((addr & 0x001f) << 3) | ((addr >> 5) & 7); break;
    case 2: addr = (addr & 0xfe00) | ((addr & 0x003f) << 3) | ((addr >> 6) & 7); break;
    case 3: addr = (addr & 0xfc00) | ((addr & 0x007f) << 3) | ((addr >> 7) & 7); break;
  }
  return (addr << 1);
}

uint8 PPU::vram_read(unsigned addr) {
  if(regs.display_disable) return vram[addr];
  if(cpu.vcounter() >= display.height) return vram[addr];
  return 0x00;
}

void PPU::vram_write(unsigned addr, uint8 data) {
  if(regs.display_disable || cpu.vcounter() >= display.height) {
    vram[addr] = data;
    cache.tilevalid[0][addr >> 4] = false;
    cache.tilevalid[1][addr >> 5] = false;
    cache.tilevalid[2][addr >> 6] = false;
    return;
  }
}

uint8 PPU::oam_read(unsigned addr) {
  if(addr & 0x0200) addr &= 0x021f;
  if(regs.display_disable) return oam[addr];
  if(cpu.vcounter() >= display.height) return oam[addr];
  return oam[0x0218];
}

void PPU::oam_write(unsigned addr, uint8 data) {
  if(addr & 0x0200) addr &= 0x021f;
  if(!regs.display_disable && cpu.vcounter() < display.height) addr = 0x0218;
  oam[addr] = data;
  sprite.update_list(addr, data);
}

uint8 PPU::cgram_read(unsigned addr) {
  return cgram[addr];
}

void PPU::cgram_write(unsigned addr, uint8 data) {
  cgram[addr] = data;
}

void PPU::mmio_update_video_mode() {
  switch(regs.bgmode) {
    case 0: {
      bg1.regs.mode = Background::Mode::BPP2; bg1.regs.priority0 = 8; bg1.regs.priority1 = 11;
      bg2.regs.mode = Background::Mode::BPP2; bg2.regs.priority0 = 7; bg2.regs.priority1 = 10;
      bg3.regs.mode = Background::Mode::BPP2; bg3.regs.priority0 = 2; bg3.regs.priority1 =  5;
      bg4.regs.mode = Background::Mode::BPP2; bg4.regs.priority0 = 1; bg4.regs.priority1 =  4;
      sprite.regs.priority0 = 3; sprite.regs.priority1 = 6; sprite.regs.priority2 = 9; sprite.regs.priority3 = 12;
    } break;

    case 1: {
      bg1.regs.mode = Background::Mode::BPP4;
      bg2.regs.mode = Background::Mode::BPP4;
      bg3.regs.mode = Background::Mode::BPP2;
      bg4.regs.mode = Background::Mode::Inactive;
      if(regs.bg3_priority) {
        bg1.regs.priority0 = 5; bg1.regs.priority1 =  8;
        bg2.regs.priority0 = 4; bg2.regs.priority1 =  7;
        bg3.regs.priority0 = 1; bg3.regs.priority1 = 10;
        sprite.regs.priority0 = 2; sprite.regs.priority1 = 3; sprite.regs.priority2 = 6; sprite.regs.priority3 = 9;
      } else {
        bg1.regs.priority0 = 6; bg1.regs.priority1 =  9;
        bg2.regs.priority0 = 5; bg2.regs.priority1 =  8;
        bg3.regs.priority0 = 1; bg3.regs.priority1 =  3;
        sprite.regs.priority0 = 2; sprite.regs.priority1 = 4; sprite.regs.priority2 = 7; sprite.regs.priority3 = 10;
      }
    } break;

    case 2: {
      bg1.regs.mode = Background::Mode::BPP4;
      bg2.regs.mode = Background::Mode::BPP4;
      bg3.regs.mode = Background::Mode::Inactive;
      bg4.regs.mode = Background::Mode::Inactive;
      bg1.regs.priority0 = 3; bg1.regs.priority1 = 7;
      bg2.regs.priority0 = 1; bg2.regs.priority1 = 5;
      sprite.regs.priority0 = 2; sprite.regs.priority1 = 4; sprite.regs.priority2 = 6; sprite.regs.priority3 = 8;
    } break;

    case 3: {
      bg1.regs.mode = Background::Mode::BPP8;
      bg2.regs.mode = Background::Mode::BPP4;
      bg3.regs.mode = Background::Mode::Inactive;
      bg4.regs.mode = Background::Mode::Inactive;
      bg1.regs.priority0 = 3; bg1.regs.priority1 = 7;
      bg2.regs.priority0 = 1; bg2.regs.priority1 = 5;
      sprite.regs.priority0 = 2; sprite.regs.priority1 = 4; sprite.regs.priority2 = 6; sprite.regs.priority3 = 8;
    } break;

    case 4: {
      bg1.regs.mode = Background::Mode::BPP8;
      bg2.regs.mode = Background::Mode::BPP2;
      bg3.regs.mode = Background::Mode::Inactive;
      bg4.regs.mode = Background::Mode::Inactive;
      bg1.regs.priority0 = 3; bg1.regs.priority1 = 7;
      bg2.regs.priority0 = 1; bg2.regs.priority1 = 5;
      sprite.regs.priority0 = 2; sprite.regs.priority1 = 4; sprite.regs.priority2 = 6; sprite.regs.priority3 = 8;
    } break;

    case 5: {
      bg1.regs.mode = Background::Mode::BPP4;
      bg2.regs.mode = Background::Mode::BPP2;
      bg3.regs.mode = Background::Mode::Inactive;
      bg4.regs.mode = Background::Mode::Inactive;
      bg1.regs.priority0 = 3; bg1.regs.priority1 = 7;
      bg2.regs.priority0 = 1; bg2.regs.priority1 = 5;
      sprite.regs.priority0 = 2; sprite.regs.priority1 = 4; sprite.regs.priority2 = 6; sprite.regs.priority3 = 8;
    } break;

    case 6: {
      bg1.regs.mode = Background::Mode::BPP4;
      bg2.regs.mode = Background::Mode::Inactive;
      bg3.regs.mode = Background::Mode::Inactive;
      bg4.regs.mode = Background::Mode::Inactive;
      bg1.regs.priority0 = 2; bg1.regs.priority1 = 5;
      sprite.regs.priority0 = 1; sprite.regs.priority1 = 3; sprite.regs.priority2 = 4; sprite.regs.priority3 = 6;
    } break;

    case 7: {
      if(regs.mode7_extbg == false) {
        bg1.regs.mode = Background::Mode::Mode7;
        bg2.regs.mode = Background::Mode::Inactive;
        bg3.regs.mode = Background::Mode::Inactive;
        bg4.regs.mode = Background::Mode::Inactive;
        bg1.regs.priority0 = 2; bg1.regs.priority1 = 2;
        sprite.regs.priority0 = 1; sprite.regs.priority1 = 3; sprite.regs.priority2 = 4; sprite.regs.priority3 = 5;
      } else {
        bg1.regs.mode = Background::Mode::Mode7;
        bg2.regs.mode = Background::Mode::Mode7;
        bg3.regs.mode = Background::Mode::Inactive;
        bg4.regs.mode = Background::Mode::Inactive;
        bg1.regs.priority0 = 3; bg1.regs.priority1 = 3;
        bg2.regs.priority0 = 1; bg2.regs.priority1 = 5;
        sprite.regs.priority0 = 2; sprite.regs.priority1 = 4; sprite.regs.priority2 = 6; sprite.regs.priority3 = 7;
      }
    } break;
  }
}

uint8 PPU::mmio_read(unsigned addr) {
  cpu.synchronize_ppu();

  switch(addr & 0xffff) {
    case 0x2104: case 0x2105: case 0x2106: case 0x2108: case 0x2109: case 0x210a:
    case 0x2114: case 0x2115: case 0x2116: case 0x2118: case 0x2119: case 0x211a:
    case 0x2124: case 0x2125: case 0x2126: case 0x2128: case 0x2129: case 0x212a: {
      return regs.ppu1_mdr;
    }

    case 0x2134: {  //MPYL
      unsigned result = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
      regs.ppu1_mdr = result >>  0;
      return regs.ppu1_mdr;
    }

    case 0x2135: {  //MPYM
      unsigned result = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
      regs.ppu1_mdr = result >>  8;
      return regs.ppu1_mdr;
    }

    case 0x2136: {  //MPYH
      unsigned result = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
      regs.ppu1_mdr = result >> 16;
      return regs.ppu1_mdr;
    }

    case 0x2137: {  //SLHV
      if(cpu.pio() & 0x80) latch_counters();
      return cpu.regs.mdr;
    }

    case 0x2138: {  //OAMDATAREAD
      regs.ppu1_mdr = oam_read(regs.oam_addr);
      regs.oam_addr = (regs.oam_addr + 1) & 0x03ff;
      sprite.set_first();
      return regs.ppu1_mdr;
    }

    case 0x2139: {  //VMDATALREAD
      regs.ppu1_mdr = regs.vram_readbuffer >> 0;
      if(regs.vram_incmode == 0) {
        uint16 addr = get_vram_addr();
        regs.vram_readbuffer  = vram_read(addr + 0) << 0;
        regs.vram_readbuffer |= vram_read(addr + 1) << 8;
        regs.vram_addr += regs.vram_incsize;
      }
      return regs.ppu1_mdr;
    }

    case 0x213a: {  //VMDATAHREAD
      regs.ppu1_mdr = regs.vram_readbuffer >> 8;
      if(regs.vram_incmode == 1) {
        uint16 addr = get_vram_addr();
        regs.vram_readbuffer  = vram_read(addr + 0) << 0;
        regs.vram_readbuffer |= vram_read(addr + 1) << 8;
        regs.vram_addr += regs.vram_incsize;
      }
      return regs.ppu1_mdr;
    }

    case 0x213b: {  //CGDATAREAD
      if((regs.cgram_addr & 1) == 0) {
        regs.ppu2_mdr = cgram_read(regs.cgram_addr);
      } else {
        regs.ppu2_mdr = (regs.ppu2_mdr & 0x80) | (cgram_read(regs.cgram_addr) & 0x7f);
      }
      regs.cgram_addr = (regs.cgram_addr + 1) & 0x01ff;
      return regs.ppu2_mdr;
    }

    case 0x213c: {  //OPHCT
      if(regs.latch_hcounter == 0) {
        regs.ppu2_mdr = regs.hcounter & 0xff;
      } else {
        regs.ppu2_mdr = (regs.ppu2_mdr & 0xfe) | (regs.hcounter >> 8);
      }
      regs.latch_hcounter ^= 1;
      return regs.ppu2_mdr;
    }

    case 0x213d: {  //OPVCT
      if(regs.latch_vcounter == 0) {
        regs.ppu2_mdr = regs.vcounter & 0xff;
      } else {
        regs.ppu2_mdr = (regs.ppu2_mdr & 0xfe) | (regs.vcounter >> 8);
      }
      regs.latch_vcounter ^= 1;
      return regs.ppu2_mdr;
    }

    case 0x213e: {  //STAT77
      regs.ppu1_mdr &= 0x10;
      regs.ppu1_mdr |= sprite.regs.time_over << 7;
      regs.ppu1_mdr |= sprite.regs.range_over << 6;
      regs.ppu1_mdr |= 0x01;  //version
      return regs.ppu1_mdr;
    }

    case 0x213f: {  //STAT78
      regs.latch_hcounter = 0;
      regs.latch_vcounter = 0;

      regs.ppu2_mdr &= 0x20;
      regs.ppu2_mdr |= cpu.field() << 7;
      if((cpu.pio() & 0x80) == 0) {
        regs.ppu2_mdr |= 0x40;
      } else if(regs.counters_latched) {
        regs.ppu2_mdr |= 0x40;
        regs.counters_latched = false;
      }
      regs.ppu2_mdr |= (system.region() == System::Region::NTSC ? 0 : 1) << 4;
      regs.ppu2_mdr |= 0x03;  //version
      return regs.ppu2_mdr;
    }
  }

  return cpu.regs.mdr;
}

void PPU::mmio_write(unsigned addr, uint8 data) {
  cpu.synchronize_ppu();

  switch(addr & 0xffff) {
    case 0x2100: {  //INIDISP
      if(regs.display_disable && cpu.vcounter() == display.height) sprite.address_reset();
      regs.display_disable = data & 0x80;
      regs.display_brightness = data & 0x0f;
      return;
    }

    case 0x2101: {  //OBSEL
      sprite.regs.base_size = (data >> 5) & 7;
      sprite.regs.nameselect = (data >> 3) & 3;
      sprite.regs.tiledata_addr = (data & 3) << 14;
      sprite.list_valid = false;
      return;
    }

    case 0x2102: {  //OAMADDL
      regs.oam_baseaddr = (regs.oam_baseaddr & 0x0100) | (data << 0);
      sprite.address_reset();
      return;
    }

    case 0x2103: {  //OAMADDH
      regs.oam_priority = data & 0x80;
      regs.oam_baseaddr = ((data & 1) << 8) | (regs.oam_baseaddr & 0x00ff);
      sprite.address_reset();
      return;
    }

    case 0x2104: {  //OAMDATA
      if((regs.oam_addr & 1) == 0) regs.oam_latchdata = data;
      if(regs.oam_addr & 0x0200) {
        oam_write(regs.oam_addr, data);
      } else if((regs.oam_addr & 1) == 1) {
        oam_write((regs.oam_addr & ~1) + 0, regs.oam_latchdata);
        oam_write((regs.oam_addr & ~1) + 1, data);
      }
      regs.oam_addr = (regs.oam_addr + 1) & 0x03ff;
      sprite.set_first();
      return;
    }

    case 0x2105: {  //BGMODE
      bg4.regs.tile_size = data & 0x80;
      bg3.regs.tile_size = data & 0x40;
      bg2.regs.tile_size = data & 0x20;
      bg1.regs.tile_size = data & 0x10;
      regs.bg3_priority = data & 0x08;
      regs.bgmode = data & 0x07;
      mmio_update_video_mode();
      return;
    }

    case 0x2106: {  //MOSAIC
      unsigned mosaic_size = (data >> 4) & 15;
      bg4.regs.mosaic = (data & 0x08 ? mosaic_size : 0);
      bg3.regs.mosaic = (data & 0x04 ? mosaic_size : 0);
      bg2.regs.mosaic = (data & 0x02 ? mosaic_size : 0);
      bg1.regs.mosaic = (data & 0x01 ? mosaic_size : 0);
      return;
    }

    case 0x2107: {  //BG1SC
      bg1.regs.screen_addr = (data & 0x7c) << 9;
      bg1.regs.screen_size = data & 3;
      return;
    }

    case 0x2108: {  //BG2SC
      bg2.regs.screen_addr = (data & 0x7c) << 9;
      bg2.regs.screen_size = data & 3;
      return;
    }

    case 0x2109: {  //BG3SC
      bg3.regs.screen_addr = (data & 0x7c) << 9;
      bg3.regs.screen_size = data & 3;
      return;
    }

    case 0x210a: {  //BG4SC
      bg4.regs.screen_addr = (data & 0x7c) << 9;
      bg4.regs.screen_size = data & 3;
      return;
    }

    case 0x210b: {  //BG12NBA
      bg1.regs.tiledata_addr = (data & 0x07) << 13;
      bg2.regs.tiledata_addr = (data & 0x70) <<  9;
      return;
    }

    case 0x210c: {  //BG34NBA
      bg3.regs.tiledata_addr = (data & 0x07) << 13;
      bg4.regs.tiledata_addr = (data & 0x70) <<  9;
      return;
    }

    case 0x210d: {  //BG1HOFS
      regs.mode7_hoffset = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;

      bg1.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg1.regs.hoffset >> 8) & 7);
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x210e: {  //BG1VOFS
      regs.mode7_voffset = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;

      bg1.regs.voffset = (data << 8) | regs.bgofs_latchdata;
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x210f: {  //BG2HOFS
      bg2.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg2.regs.hoffset >> 8) & 7);
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x2110: {  //BG2VOFS
      bg2.regs.voffset = (data << 8) | regs.bgofs_latchdata;
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x2111: {  //BG3HOFS
      bg3.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg3.regs.hoffset >> 8) & 7);
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x2112: {  //BG3VOFS
      bg3.regs.voffset = (data << 8) | regs.bgofs_latchdata;
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x2113: {  //BG4HOFS
      bg4.regs.hoffset = (data << 8) | (regs.bgofs_latchdata & ~7) | ((bg4.regs.hoffset >> 8) & 7);
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x2114: {  //BG4VOFS
      bg4.regs.voffset = (data << 8) | regs.bgofs_latchdata;
      regs.bgofs_latchdata = data;
      return;
    }

    case 0x2115: {  //VMAIN
      regs.vram_incmode = data & 0x80;
      regs.vram_mapping = (data >> 2) & 3;
      switch(data & 3) {
        case 0: regs.vram_incsize =   1; break;
        case 1: regs.vram_incsize =  32; break;
        case 2: regs.vram_incsize = 128; break;
        case 3: regs.vram_incsize = 128; break;
      }
      return;
    }

    case 0x2116: {  //VMADDL
      regs.vram_addr = (regs.vram_addr & 0xff00) | (data << 0);
      uint16 addr = get_vram_addr();
      regs.vram_readbuffer  = vram_read(addr + 0) << 0;
      regs.vram_readbuffer |= vram_read(addr + 1) << 8;
      return;
    }

    case 0x2117: {  //VMADDH
      regs.vram_addr = (data << 8) | (regs.vram_addr & 0x00ff);
      uint16 addr = get_vram_addr();
      regs.vram_readbuffer  = vram_read(addr + 0) << 0;
      regs.vram_readbuffer |= vram_read(addr + 1) << 8;
      return;
    }

    case 0x2118: {  //VMDATAL
      vram_write(get_vram_addr() + 0, data);
      if(regs.vram_incmode == 0) regs.vram_addr += regs.vram_incsize;
      return;
    }

    case 0x2119: {  //VMDATAH
      vram_write(get_vram_addr() + 1, data);
      if(regs.vram_incmode == 1) regs.vram_addr += regs.vram_incsize;
      return;
    }

    case 0x211a: {  //M7SEL
      regs.mode7_repeat = (data >> 6) & 3;
      regs.mode7_vflip = data & 0x02;
      regs.mode7_hflip = data & 0x01;
      return;
    }

    case 0x211b: {  //M7A
      regs.m7a = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;
      return;
    }

    case 0x211c: {  //M7B
      regs.m7b = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;
      return;
    }

    case 0x211d: {  //M7C
      regs.m7c = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;
      return;
    }

    case 0x211e: {  //M7D
      regs.m7d = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;
      return;
    }

    case 0x211f: {  //M7X
      regs.m7x = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;
      return;
    }

    case 0x2120: {  //M7Y
      regs.m7y = (data << 8) | regs.mode7_latchdata;
      regs.mode7_latchdata = data;
      return;
    }

    case 0x2121: {  //CGADD
      regs.cgram_addr = data << 1;
      return;
    }

    case 0x2122: {  //CGDATA
      if((regs.cgram_addr & 1) == 0) {
        regs.cgram_latchdata = data;
      } else {
        cgram_write((regs.cgram_addr & ~1) + 0, regs.cgram_latchdata);
        cgram_write((regs.cgram_addr & ~1) + 1, data & 0x7f);
      }
      regs.cgram_addr = (regs.cgram_addr + 1) & 0x01ff;
      return;
    }

    case 0x2123: {  //W12SEL
      bg2.window.two_enable = data & 0x80;
      bg2.window.two_invert = data & 0x40;
      bg2.window.one_enable = data & 0x20;
      bg2.window.one_invert = data & 0x10;
      bg1.window.two_enable = data & 0x08;
      bg1.window.two_invert = data & 0x04;
      bg1.window.one_enable = data & 0x02;
      bg1.window.one_invert = data & 0x01;
      return;
    }

    case 0x2124: {  //W34SEL
      bg4.window.two_enable = data & 0x80;
      bg4.window.two_invert = data & 0x40;
      bg4.window.one_enable = data & 0x20;
      bg4.window.one_invert = data & 0x10;
      bg3.window.two_enable = data & 0x08;
      bg3.window.two_invert = data & 0x04;
      bg3.window.one_enable = data & 0x02;
      bg3.window.one_invert = data & 0x01;
      return;
    }

    case 0x2125: {  //WOBJSEL
      screen.window.two_enable = data & 0x80;
      screen.window.two_invert = data & 0x40;
      screen.window.one_enable = data & 0x20;
      screen.window.one_invert = data & 0x10;
      sprite.window.two_enable = data & 0x08;
      sprite.window.two_invert = data & 0x04;
      sprite.window.one_enable = data & 0x02;
      sprite.window.one_invert = data & 0x01;
      return;
    }

    case 0x2126: {  //WH0
      regs.window_one_left = data;
      return;
    }

    case 0x2127: {  //WH1
      regs.window_one_right = data;
      return;
    }

    case 0x2128: {  //WH2
      regs.window_two_left = data;
      return;
    }

    case 0x2129: {  //WH3
      regs.window_two_right = data;
      return;
    }

    case 0x212a: {  //WBGLOG
      bg4.window.mask = (data >> 6) & 3;
      bg3.window.mask = (data >> 4) & 3;
      bg2.window.mask = (data >> 2) & 3;
      bg1.window.mask = (data >> 0) & 3;
      return;
    }

    case 0x212b: {  //WOBJLOG
      screen.window.mask = (data >> 2) & 3;
      sprite.window.mask = (data >> 0) & 3;
      return;
    }

    case 0x212c: {  //TM
      sprite.regs.main_enable = data & 0x10;
      bg4.regs.main_enable = data & 0x08;
      bg3.regs.main_enable = data & 0x04;
      bg2.regs.main_enable = data & 0x02;
      bg1.regs.main_enable = data & 0x01;
      return;
    }

    case 0x212d: {  //TS
      sprite.regs.sub_enable = data & 0x10;
      bg4.regs.sub_enable = data & 0x08;
      bg3.regs.sub_enable = data & 0x04;
      bg2.regs.sub_enable = data & 0x02;
      bg1.regs.sub_enable = data & 0x01;
      return;
    }

    case 0x212e: {  //TMW
      sprite.window.main_enable = data & 0x10;
      bg4.window.main_enable = data & 0x08;
      bg3.window.main_enable = data & 0x04;
      bg2.window.main_enable = data & 0x02;
      bg1.window.main_enable = data & 0x01;
      return;
    }

    case 0x212f: {  //TSW
      sprite.window.sub_enable = data & 0x10;
      bg4.window.sub_enable = data & 0x08;
      bg3.window.sub_enable = data & 0x04;
      bg2.window.sub_enable = data & 0x02;
      bg1.window.sub_enable = data & 0x01;
      return;
    }

    case 0x2130: {  //CGWSEL
      screen.window.main_mask = (data >> 6) & 3;
      screen.window.sub_mask = (data >> 4) & 3;
      screen.regs.addsub_mode = data & 0x02;
      screen.regs.direct_color = data & 0x01;
      return;
    }

    case 0x2131: {  //CGADDSUB
      screen.regs.color_mode = data & 0x80;
      screen.regs.color_halve = data & 0x40;
      screen.regs.color_enable[6] = data & 0x20;
      screen.regs.color_enable[5] = data & 0x10;
      screen.regs.color_enable[4] = data & 0x10;
      screen.regs.color_enable[3] = data & 0x08;
      screen.regs.color_enable[2] = data & 0x04;
      screen.regs.color_enable[1] = data & 0x02;
      screen.regs.color_enable[0] = data & 0x01;
      return;
    }

    case 0x2132: {  //COLDATA
      if(data & 0x80) screen.regs.color_b = data & 0x1f;
      if(data & 0x40) screen.regs.color_g = data & 0x1f;
      if(data & 0x20) screen.regs.color_r = data & 0x1f;
      screen.regs.color = (screen.regs.color_b << 10) | (screen.regs.color_g << 5) | (screen.regs.color_r << 0);
      return;
    }

    case 0x2133: {  //SETINI
      regs.mode7_extbg = data & 0x40;
      regs.pseudo_hires = data & 0x08;
      regs.overscan = data & 0x04;
      sprite.regs.interlace = data & 0x02;
      regs.interlace = data & 0x01;
      mmio_update_video_mode();
      sprite.list_valid = false;
      return;
    }
  }
}

void PPU::mmio_reset() {
  //internal
  regs.ppu1_mdr = 0;
  regs.ppu2_mdr = 0;

  regs.vram_readbuffer = 0;
  regs.oam_latchdata = 0;
  regs.cgram_latchdata = 0;
  regs.bgofs_latchdata = 0;
  regs.mode7_latchdata = 0;

  regs.counters_latched = 0;
  regs.latch_hcounter = 0;
  regs.latch_vcounter = 0;

  sprite.regs.first_sprite = 0;
  sprite.list_valid = false;

  //$2100
  regs.display_disable = true;
  regs.display_brightness = 0;

  //$2101
  sprite.regs.base_size = 0;
  sprite.regs.nameselect = 0;
  sprite.regs.tiledata_addr = 0;

  //$2102-$2103
  regs.oam_baseaddr = 0;
  regs.oam_addr = 0;
  regs.oam_priority = 0;

  //$2105
  bg4.regs.tile_size = 0;
  bg3.regs.tile_size = 0;
  bg2.regs.tile_size = 0;
  bg1.regs.tile_size = 0;
  regs.bg3_priority = 0;
  regs.bgmode = 0;

  //$2106
  bg4.regs.mosaic = 0;
  bg3.regs.mosaic = 0;
  bg2.regs.mosaic = 0;
  bg1.regs.mosaic = 0;

  //$2107-$210a
  bg1.regs.screen_addr = 0;
  bg1.regs.screen_size = 0;
  bg2.regs.screen_addr = 0;
  bg2.regs.screen_size = 0;
  bg3.regs.screen_addr = 0;
  bg3.regs.screen_size = 0;
  bg4.regs.screen_addr = 0;
  bg4.regs.screen_size = 0;

  //$210b-$210c
  bg1.regs.tiledata_addr = 0;
  bg2.regs.tiledata_addr = 0;
  bg3.regs.tiledata_addr = 0;
  bg4.regs.tiledata_addr = 0;

  //$210d-$2114
  regs.mode7_hoffset = 0;
  regs.mode7_voffset = 0;
  bg1.regs.hoffset = 0;
  bg1.regs.voffset = 0;
  bg2.regs.hoffset = 0;
  bg2.regs.voffset = 0;
  bg3.regs.hoffset = 0;
  bg3.regs.voffset = 0;
  bg4.regs.hoffset = 0;
  bg4.regs.voffset = 0;

  //$2115
  regs.vram_incmode = 0;
  regs.vram_mapping = 0;
  regs.vram_incsize = 1;

  //$2116-$2117
  regs.vram_addr = 0;

  //$211a
  regs.mode7_repeat = 0;
  regs.mode7_vflip = 0;
  regs.mode7_hflip = 0;

  //$211b-$2120
  regs.m7a = 0;
  regs.m7b = 0;
  regs.m7c = 0;
  regs.m7d = 0;
  regs.m7x = 0;
  regs.m7y = 0;

  //$2121
  regs.cgram_addr = 0;

  //$2123-$2125
  bg1.window.one_enable = 0;
  bg1.window.one_invert = 0;
  bg1.window.two_enable = 0;
  bg1.window.two_invert = 0;

  bg2.window.one_enable = 0;
  bg2.window.one_invert = 0;
  bg2.window.two_enable = 0;
  bg2.window.two_invert = 0;

  bg3.window.one_enable = 0;
  bg3.window.one_invert = 0;
  bg3.window.two_enable = 0;
  bg3.window.two_invert = 0;

  bg4.window.one_enable = 0;
  bg4.window.one_invert = 0;
  bg4.window.two_enable = 0;
  bg4.window.two_invert = 0;

  sprite.window.one_enable = 0;
  sprite.window.one_invert = 0;
  sprite.window.two_enable = 0;
  sprite.window.two_invert = 0;

  screen.window.one_enable = 0;
  screen.window.one_invert = 0;
  screen.window.two_enable = 0;
  screen.window.two_invert = 0;

  //$2126-$2129
  regs.window_one_left = 0;
  regs.window_one_right = 0;
  regs.window_two_left = 0;
  regs.window_two_right = 0;

  //$212a-$212b
  bg1.window.mask = 0;
  bg2.window.mask = 0;
  bg3.window.mask = 0;
  bg4.window.mask = 0;
  sprite.window.mask = 0;
  screen.window.mask = 0;

  //$212c
  bg1.regs.main_enable = 0;
  bg2.regs.main_enable = 0;
  bg3.regs.main_enable = 0;
  bg4.regs.main_enable = 0;
  sprite.regs.main_enable = 0;

  //$212d
  bg1.regs.sub_enable = 0;
  bg2.regs.sub_enable = 0;
  bg3.regs.sub_enable = 0;
  bg4.regs.sub_enable = 0;
  sprite.regs.sub_enable = 0;

  //$212e
  bg1.window.main_enable = 0;
  bg2.window.main_enable = 0;
  bg3.window.main_enable = 0;
  bg4.window.main_enable = 0;
  sprite.window.main_enable = 0;

  //$212f
  bg1.window.sub_enable = 0;
  bg2.window.sub_enable = 0;
  bg3.window.sub_enable = 0;
  bg4.window.sub_enable = 0;
  sprite.window.sub_enable = 0;

  //$2130
  screen.window.main_mask = 0;
  screen.window.sub_mask = 0;
  screen.regs.addsub_mode = 0;
  screen.regs.direct_color = 0;

  //$2131
  screen.regs.color_mode = 0;
  screen.regs.color_halve = 0;
  screen.regs.color_enable[6] = 0;
  screen.regs.color_enable[5] = 0;
  screen.regs.color_enable[4] = 0;
  screen.regs.color_enable[3] = 0;
  screen.regs.color_enable[2] = 0;
  screen.regs.color_enable[1] = 0;
  screen.regs.color_enable[0] = 0;

  //$2132
  screen.regs.color_b = 0;
  screen.regs.color_g = 0;
  screen.regs.color_r = 0;
  screen.regs.color = 0;

  //$2133
  regs.mode7_extbg = 0;
  regs.pseudo_hires = 0;
  regs.overscan = 0;
  sprite.regs.interlace = 0;
  regs.interlace = 0;

  //$213e
  sprite.regs.time_over = 0;
  sprite.regs.range_over = 0;

  mmio_update_video_mode();
}

#endif
