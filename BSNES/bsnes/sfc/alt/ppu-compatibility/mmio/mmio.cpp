#ifdef PPU_CPP

//INIDISP
void PPU::mmio_w2100(uint8 value) {
  if(regs.display_disabled == true && cpu.vcounter() == (!overscan() ? 225 : 240)) {
    regs.oam_addr = regs.oam_baseaddr << 1;
    regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;
  }

  regs.display_disabled   = !!(value & 0x80);
  regs.display_brightness = value & 15;
}

//OBSEL
void PPU::mmio_w2101(uint8 value) {
  regs.oam_basesize   = (value >> 5) & 7;
  regs.oam_nameselect = (value >> 3) & 3;
  regs.oam_tdaddr     = (value & 3) << 14;
}

//OAMADDL
void PPU::mmio_w2102(uint8 data) {
  regs.oam_baseaddr    = (regs.oam_baseaddr & ~0xff) | (data << 0);
  regs.oam_baseaddr   &= 0x01ff;
  regs.oam_addr        = regs.oam_baseaddr << 1;
  regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;
}

//OAMADDH
void PPU::mmio_w2103(uint8 data) {
  regs.oam_priority    = !!(data & 0x80);
  regs.oam_baseaddr    = (regs.oam_baseaddr &  0xff) | (data << 8);
  regs.oam_baseaddr   &= 0x01ff;
  regs.oam_addr        = regs.oam_baseaddr << 1;
  regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;
}

//OAMDATA
void PPU::mmio_w2104(uint8 data) {
  if((regs.oam_addr & 1) == 0) regs.oam_latchdata = data;

  if(regs.oam_addr & 0x0200) {
    oam_mmio_write(regs.oam_addr, data);
  } else if((regs.oam_addr & 1) == 1) {
    oam_mmio_write((regs.oam_addr & ~1) + 0, regs.oam_latchdata);
    oam_mmio_write((regs.oam_addr & ~1) + 1, data);
  }

  regs.oam_addr++;
  regs.oam_addr &= 0x03ff;
  regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;
}

//BGMODE
void PPU::mmio_w2105(uint8 value) {
  regs.bg_tilesize[BG4] = !!(value & 0x80);
  regs.bg_tilesize[BG3] = !!(value & 0x40);
  regs.bg_tilesize[BG2] = !!(value & 0x20);
  regs.bg_tilesize[BG1] = !!(value & 0x10);
  regs.bg3_priority     = !!(value & 0x08);
  regs.bg_mode          = (value & 7);
}

//MOSAIC
void PPU::mmio_w2106(uint8 value) {
  regs.mosaic_size         = (value >> 4) & 15;
  regs.mosaic_enabled[BG4] = !!(value & 0x08);
  regs.mosaic_enabled[BG3] = !!(value & 0x04);
  regs.mosaic_enabled[BG2] = !!(value & 0x02);
  regs.mosaic_enabled[BG1] = !!(value & 0x01);
}

//BG1SC
void PPU::mmio_w2107(uint8 value) {
  regs.bg_scaddr[BG1] = (value & 0x7c) << 9;
  regs.bg_scsize[BG1] = value & 3;
}

//BG2SC
void PPU::mmio_w2108(uint8 value) {
  regs.bg_scaddr[BG2] = (value & 0x7c) << 9;
  regs.bg_scsize[BG2] = value & 3;
}

//BG3SC
void PPU::mmio_w2109(uint8 value) {
  regs.bg_scaddr[BG3] = (value & 0x7c) << 9;
  regs.bg_scsize[BG3] = value & 3;
}

//BG4SC
void PPU::mmio_w210a(uint8 value) {
  regs.bg_scaddr[BG4] = (value & 0x7c) << 9;
  regs.bg_scsize[BG4] = value & 3;
}

//BG12NBA
void PPU::mmio_w210b(uint8 value) {
  regs.bg_tdaddr[BG1] = (value & 0x07) << 13;
  regs.bg_tdaddr[BG2] = (value & 0x70) <<  9;
}

//BG34NBA
void PPU::mmio_w210c(uint8 value) {
  regs.bg_tdaddr[BG3] = (value & 0x07) << 13;
  regs.bg_tdaddr[BG4] = (value & 0x70) <<  9;
}

//BG1HOFS
void PPU::mmio_w210d(uint8 value) {
  regs.m7_hofs  = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;

  regs.bg_hofs[BG1] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG1] >> 8) & 7);
  regs.bg_ofslatch  = value;
}

//BG1VOFS
void PPU::mmio_w210e(uint8 value) {
  regs.m7_vofs  = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;

  regs.bg_vofs[BG1] = (value << 8) | (regs.bg_ofslatch);
  regs.bg_ofslatch  = value;
}

//BG2HOFS
void PPU::mmio_w210f(uint8 value) {
  regs.bg_hofs[BG2] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG2] >> 8) & 7);
  regs.bg_ofslatch  = value;
}

//BG2VOFS
void PPU::mmio_w2110(uint8 value) {
  regs.bg_vofs[BG2] = (value << 8) | (regs.bg_ofslatch);
  regs.bg_ofslatch  = value;
}

//BG3HOFS
void PPU::mmio_w2111(uint8 value) {
  regs.bg_hofs[BG3] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG3] >> 8) & 7);
  regs.bg_ofslatch  = value;
}

//BG3VOFS
void PPU::mmio_w2112(uint8 value) {
  regs.bg_vofs[BG3] = (value << 8) | (regs.bg_ofslatch);
  regs.bg_ofslatch  = value;
}

//BG4HOFS
void PPU::mmio_w2113(uint8 value) {
  regs.bg_hofs[BG4] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG4] >> 8) & 7);
  regs.bg_ofslatch  = value;
}

//BG4VOFS
void PPU::mmio_w2114(uint8 value) {
  regs.bg_vofs[BG4] = (value << 8) | (regs.bg_ofslatch);
  regs.bg_ofslatch  = value;
}

//VMAIN
void PPU::mmio_w2115(uint8 value) {
  regs.vram_incmode = !!(value & 0x80);
  regs.vram_mapping = (value >> 2) & 3;
  switch(value & 3) {
    case 0: regs.vram_incsize =   1; break;
    case 1: regs.vram_incsize =  32; break;
    case 2: regs.vram_incsize = 128; break;
    case 3: regs.vram_incsize = 128; break;
  }
}

//VMADDL
void PPU::mmio_w2116(uint8 value) {
  regs.vram_addr = (regs.vram_addr & 0xff00) | value;
  uint16 addr = get_vram_address();
  regs.vram_readbuffer  = vram_mmio_read(addr + 0);
  regs.vram_readbuffer |= vram_mmio_read(addr + 1) << 8;
}

//VMADDH
void PPU::mmio_w2117(uint8 value) {
  regs.vram_addr = (value << 8) | (regs.vram_addr & 0x00ff);
  uint16 addr = get_vram_address();
  regs.vram_readbuffer  = vram_mmio_read(addr + 0);
  regs.vram_readbuffer |= vram_mmio_read(addr + 1) << 8;
}

//VMDATAL
void PPU::mmio_w2118(uint8 value) {
uint16 addr = get_vram_address();
  vram_mmio_write(addr, value);
  bg_tiledata_state[TILE_2BIT][(addr >> 4)] = 1;
  bg_tiledata_state[TILE_4BIT][(addr >> 5)] = 1;
  bg_tiledata_state[TILE_8BIT][(addr >> 6)] = 1;

  if(regs.vram_incmode == 0) {
    regs.vram_addr += regs.vram_incsize;
  }
}

//VMDATAH
void PPU::mmio_w2119(uint8 value) {
uint16 addr = get_vram_address() + 1;
  vram_mmio_write(addr, value);
  bg_tiledata_state[TILE_2BIT][(addr >> 4)] = 1;
  bg_tiledata_state[TILE_4BIT][(addr >> 5)] = 1;
  bg_tiledata_state[TILE_8BIT][(addr >> 6)] = 1;

  if(regs.vram_incmode == 1) {
    regs.vram_addr += regs.vram_incsize;
  }
}

//M7SEL
void PPU::mmio_w211a(uint8 value) {
  regs.mode7_repeat = (value >> 6) & 3;
  regs.mode7_vflip  = !!(value & 0x02);
  regs.mode7_hflip  = !!(value & 0x01);
}

//M7A
void PPU::mmio_w211b(uint8 value) {
  regs.m7a      = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;
}

//M7B
void PPU::mmio_w211c(uint8 value) {
  regs.m7b      = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;
}

//M7C
void PPU::mmio_w211d(uint8 value) {
  regs.m7c      = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;
}

//M7D
void PPU::mmio_w211e(uint8 value) {
  regs.m7d      = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;
}

//M7X
void PPU::mmio_w211f(uint8 value) {
  regs.m7x      = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;
}

//M7Y
void PPU::mmio_w2120(uint8 value) {
  regs.m7y      = (value << 8) | regs.m7_latch;
  regs.m7_latch = value;
}

//CGADD
void PPU::mmio_w2121(uint8 value) {
  regs.cgram_addr = value << 1;
}

//CGDATA
//note: CGRAM palette data format is 15-bits
//(0,bbbbb,ggggg,rrrrr). Highest bit is ignored,
//as evidenced by $213b CGRAM data reads.
//
//anomie indicates writes to CGDATA work the same
//as writes to OAMDATA's low table. need to verify
//this on hardware.
void PPU::mmio_w2122(uint8 value) {
  if(!(regs.cgram_addr & 1)) {
    regs.cgram_latchdata = value;
  } else {
    cgram_mmio_write((regs.cgram_addr & 0x01fe),     regs.cgram_latchdata);
    cgram_mmio_write((regs.cgram_addr & 0x01fe) + 1, value & 0x7f);
  }
  regs.cgram_addr++;
  regs.cgram_addr &= 0x01ff;
}

//W12SEL
void PPU::mmio_w2123(uint8 value) {
  regs.window2_enabled[BG2] = !!(value & 0x80);
  regs.window2_invert [BG2] = !!(value & 0x40);
  regs.window1_enabled[BG2] = !!(value & 0x20);
  regs.window1_invert [BG2] = !!(value & 0x10);
  regs.window2_enabled[BG1] = !!(value & 0x08);
  regs.window2_invert [BG1] = !!(value & 0x04);
  regs.window1_enabled[BG1] = !!(value & 0x02);
  regs.window1_invert [BG1] = !!(value & 0x01);
}

//W34SEL
void PPU::mmio_w2124(uint8 value) {
  regs.window2_enabled[BG4] = !!(value & 0x80);
  regs.window2_invert [BG4] = !!(value & 0x40);
  regs.window1_enabled[BG4] = !!(value & 0x20);
  regs.window1_invert [BG4] = !!(value & 0x10);
  regs.window2_enabled[BG3] = !!(value & 0x08);
  regs.window2_invert [BG3] = !!(value & 0x04);
  regs.window1_enabled[BG3] = !!(value & 0x02);
  regs.window1_invert [BG3] = !!(value & 0x01);
}

//WOBJSEL
void PPU::mmio_w2125(uint8 value) {
  regs.window2_enabled[COL] = !!(value & 0x80);
  regs.window2_invert [COL] = !!(value & 0x40);
  regs.window1_enabled[COL] = !!(value & 0x20);
  regs.window1_invert [COL] = !!(value & 0x10);
  regs.window2_enabled[OAM] = !!(value & 0x08);
  regs.window2_invert [OAM] = !!(value & 0x04);
  regs.window1_enabled[OAM] = !!(value & 0x02);
  regs.window1_invert [OAM] = !!(value & 0x01);
}

//WH0
void PPU::mmio_w2126(uint8 value) {
  regs.window1_left = value;
}

//WH1
void PPU::mmio_w2127(uint8 value) {
  regs.window1_right = value;
}

//WH2
void PPU::mmio_w2128(uint8 value) {
  regs.window2_left = value;
}

//WH3
void PPU::mmio_w2129(uint8 value) {
  regs.window2_right = value;
}

//WBGLOG
void PPU::mmio_w212a(uint8 value) {
  regs.window_mask[BG4] = (value >> 6) & 3;
  regs.window_mask[BG3] = (value >> 4) & 3;
  regs.window_mask[BG2] = (value >> 2) & 3;
  regs.window_mask[BG1] = (value     ) & 3;
}

//WOBJLOG
void PPU::mmio_w212b(uint8 value) {
  regs.window_mask[COL] = (value >> 2) & 3;
  regs.window_mask[OAM] = (value     ) & 3;
}

//TM
void PPU::mmio_w212c(uint8 value) {
  regs.bg_enabled[OAM] = !!(value & 0x10);
  regs.bg_enabled[BG4] = !!(value & 0x08);
  regs.bg_enabled[BG3] = !!(value & 0x04);
  regs.bg_enabled[BG2] = !!(value & 0x02);
  regs.bg_enabled[BG1] = !!(value & 0x01);
}

//TS
void PPU::mmio_w212d(uint8 value) {
  regs.bgsub_enabled[OAM] = !!(value & 0x10);
  regs.bgsub_enabled[BG4] = !!(value & 0x08);
  regs.bgsub_enabled[BG3] = !!(value & 0x04);
  regs.bgsub_enabled[BG2] = !!(value & 0x02);
  regs.bgsub_enabled[BG1] = !!(value & 0x01);
}

//TMW
void PPU::mmio_w212e(uint8 value) {
  regs.window_enabled[OAM] = !!(value & 0x10);
  regs.window_enabled[BG4] = !!(value & 0x08);
  regs.window_enabled[BG3] = !!(value & 0x04);
  regs.window_enabled[BG2] = !!(value & 0x02);
  regs.window_enabled[BG1] = !!(value & 0x01);
}

//TSW
void PPU::mmio_w212f(uint8 value) {
  regs.sub_window_enabled[OAM] = !!(value & 0x10);
  regs.sub_window_enabled[BG4] = !!(value & 0x08);
  regs.sub_window_enabled[BG3] = !!(value & 0x04);
  regs.sub_window_enabled[BG2] = !!(value & 0x02);
  regs.sub_window_enabled[BG1] = !!(value & 0x01);
}

//CGWSEL
void PPU::mmio_w2130(uint8 value) {
  regs.color_mask    = (value >> 6) & 3;
  regs.colorsub_mask = (value >> 4) & 3;
  regs.addsub_mode   = !!(value & 0x02);
  regs.direct_color  = !!(value & 0x01);
}

//CGADDSUB
void PPU::mmio_w2131(uint8 value) {
  regs.color_mode          = !!(value & 0x80);
  regs.color_halve         = !!(value & 0x40);
  regs.color_enabled[BACK] = !!(value & 0x20);
  regs.color_enabled[OAM]  = !!(value & 0x10);
  regs.color_enabled[BG4]  = !!(value & 0x08);
  regs.color_enabled[BG3]  = !!(value & 0x04);
  regs.color_enabled[BG2]  = !!(value & 0x02);
  regs.color_enabled[BG1]  = !!(value & 0x01);
}

//COLDATA
void PPU::mmio_w2132(uint8 value) {
  if(value & 0x80) regs.color_b = value & 0x1f;
  if(value & 0x40) regs.color_g = value & 0x1f;
  if(value & 0x20) regs.color_r = value & 0x1f;

  regs.color_rgb = (regs.color_r)
                 | (regs.color_g << 5)
                 | (regs.color_b << 10);
}

//SETINI
void PPU::mmio_w2133(uint8 value) {
  regs.mode7_extbg   = !!(value & 0x40);
  regs.pseudo_hires  = !!(value & 0x08);
  regs.overscan      = !!(value & 0x04);
  regs.oam_interlace = !!(value & 0x02);
  regs.interlace     = !!(value & 0x01);

  display.overscan = regs.overscan;
  sprite_list_valid = false;
}

//MPYL
uint8 PPU::mmio_r2134() {
uint32 r;
  r = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
  regs.ppu1_mdr = r;
  return regs.ppu1_mdr;
}

//MPYM
uint8 PPU::mmio_r2135() {
uint32 r;
  r = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
  regs.ppu1_mdr = r >> 8;
  return regs.ppu1_mdr;
}

//MPYH
uint8 PPU::mmio_r2136() {
uint32 r;
  r = ((int16)regs.m7a * (int8)(regs.m7b >> 8));
  regs.ppu1_mdr = r >> 16;
  return regs.ppu1_mdr;
}

//SLHV
uint8 PPU::mmio_r2137() {
  if(cpu.pio() & 0x80) {
    latch_counters();
  }
  return cpu.regs.mdr;
}

//OAMDATAREAD
uint8 PPU::mmio_r2138() {
  regs.ppu1_mdr = oam_mmio_read(regs.oam_addr);

  regs.oam_addr++;
  regs.oam_addr &= 0x03ff;
  regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;

  return regs.ppu1_mdr;
}

//VMDATALREAD
uint8 PPU::mmio_r2139() {
uint16 addr = get_vram_address();
  regs.ppu1_mdr = regs.vram_readbuffer;
  if(regs.vram_incmode == 0) {
    addr &= 0xfffe;
    regs.vram_readbuffer  = vram_mmio_read(addr + 0);
    regs.vram_readbuffer |= vram_mmio_read(addr + 1) << 8;
    regs.vram_addr += regs.vram_incsize;
  }
  return regs.ppu1_mdr;
}

//VMDATAHREAD
uint8 PPU::mmio_r213a() {
uint16 addr = get_vram_address() + 1;
  regs.ppu1_mdr = regs.vram_readbuffer >> 8;
  if(regs.vram_incmode == 1) {
    addr &= 0xfffe;
    regs.vram_readbuffer  = vram_mmio_read(addr + 0);
    regs.vram_readbuffer |= vram_mmio_read(addr + 1) << 8;
    regs.vram_addr += regs.vram_incsize;
  }
  return regs.ppu1_mdr;
}

//CGDATAREAD
//note: CGRAM palette data is 15-bits (0,bbbbb,ggggg,rrrrr)
//therefore, the high byte read from each color does not
//update bit 7 of the PPU2 MDR.
uint8 PPU::mmio_r213b() {
  if(!(regs.cgram_addr & 1)) {
    regs.ppu2_mdr  = cgram_mmio_read(regs.cgram_addr) & 0xff;
  } else {
    regs.ppu2_mdr &= 0x80;
    regs.ppu2_mdr |= cgram_mmio_read(regs.cgram_addr) & 0x7f;
  }
  regs.cgram_addr++;
  regs.cgram_addr &= 0x01ff;
  return regs.ppu2_mdr;
}

//OPHCT
uint8 PPU::mmio_r213c() {
  if(!regs.latch_hcounter) {
    regs.ppu2_mdr  = regs.hcounter & 0xff;
  } else {
    regs.ppu2_mdr &= 0xfe;
    regs.ppu2_mdr |= (regs.hcounter >> 8) & 1;
  }
  regs.latch_hcounter ^= 1;
  return regs.ppu2_mdr;
}

//OPVCT
uint8 PPU::mmio_r213d() {
  if(!regs.latch_vcounter) {
    regs.ppu2_mdr  = regs.vcounter & 0xff;
  } else {
    regs.ppu2_mdr &= 0xfe;
    regs.ppu2_mdr |= (regs.vcounter >> 8) & 1;
  }
  regs.latch_vcounter ^= 1;
  return regs.ppu2_mdr;
}

//STAT77
uint8 PPU::mmio_r213e() {
uint8 r = 0x00;
  r |= (regs.time_over)  ? 0x80 : 0x00;
  r |= (regs.range_over) ? 0x40 : 0x00;
  r |= (regs.ppu1_mdr & 0x10);
  r |= (ppu1_version & 0x0f);
  regs.ppu1_mdr = r;
  return regs.ppu1_mdr;
}

//STAT78
uint8 PPU::mmio_r213f() {
uint8 r = 0x00;
  regs.latch_hcounter = 0;
  regs.latch_vcounter = 0;

  r |= cpu.field() << 7;
  if(!(cpu.pio() & 0x80)) {
    r |= 0x40;
  } else if(regs.counters_latched == true) {
    r |= 0x40;
    regs.counters_latched = false;
  }
  r |= (regs.ppu2_mdr & 0x20);
  r |= (region << 4); //0 = NTSC, 1 = PAL
  r |= (ppu2_version & 0x0f);
  regs.ppu2_mdr = r;
  return regs.ppu2_mdr;
}

uint8 PPU::mmio_read(unsigned addr) {
  cpu.synchronize_ppu();

  switch(addr & 0xffff) {
    case 0x2104:
    case 0x2105:
    case 0x2106:
    case 0x2108:
    case 0x2109:
    case 0x210a:
    case 0x2114:
    case 0x2115:
    case 0x2116:
    case 0x2118:
    case 0x2119:
    case 0x211a:
    case 0x2124:
    case 0x2125:
    case 0x2126:
    case 0x2128:
    case 0x2129:
    case 0x212a: return regs.ppu1_mdr;
    case 0x2134: return mmio_r2134();  //MPYL
    case 0x2135: return mmio_r2135();  //MPYM
    case 0x2136: return mmio_r2136();  //MPYH
    case 0x2137: return mmio_r2137();  //SLHV
    case 0x2138: return mmio_r2138();  //OAMDATAREAD
    case 0x2139: return mmio_r2139();  //VMDATALREAD
    case 0x213a: return mmio_r213a();  //VMDATAHREAD
    case 0x213b: return mmio_r213b();  //CGDATAREAD
    case 0x213c: return mmio_r213c();  //OPHCT
    case 0x213d: return mmio_r213d();  //OPVCT
    case 0x213e: return mmio_r213e();  //STAT77
    case 0x213f: return mmio_r213f();  //STAT78
  }

  return cpu.regs.mdr;
}

void PPU::mmio_write(unsigned addr, uint8 data) {
  cpu.synchronize_ppu();

  switch(addr & 0xffff) {
    case 0x2100: return mmio_w2100(data);  //INIDISP
    case 0x2101: return mmio_w2101(data);  //OBSEL
    case 0x2102: return mmio_w2102(data);  //OAMADDL
    case 0x2103: return mmio_w2103(data);  //OAMADDH
    case 0x2104: return mmio_w2104(data);  //OAMDATA
    case 0x2105: return mmio_w2105(data);  //BGMODE
    case 0x2106: return mmio_w2106(data);  //MOSAIC
    case 0x2107: return mmio_w2107(data);  //BG1SC
    case 0x2108: return mmio_w2108(data);  //BG2SC
    case 0x2109: return mmio_w2109(data);  //BG3SC
    case 0x210a: return mmio_w210a(data);  //BG4SC
    case 0x210b: return mmio_w210b(data);  //BG12NBA
    case 0x210c: return mmio_w210c(data);  //BG34NBA
    case 0x210d: return mmio_w210d(data);  //BG1HOFS
    case 0x210e: return mmio_w210e(data);  //BG1VOFS
    case 0x210f: return mmio_w210f(data);  //BG2HOFS
    case 0x2110: return mmio_w2110(data);  //BG2VOFS
    case 0x2111: return mmio_w2111(data);  //BG3HOFS
    case 0x2112: return mmio_w2112(data);  //BG3VOFS
    case 0x2113: return mmio_w2113(data);  //BG4HOFS
    case 0x2114: return mmio_w2114(data);  //BG4VOFS
    case 0x2115: return mmio_w2115(data);  //VMAIN
    case 0x2116: return mmio_w2116(data);  //VMADDL
    case 0x2117: return mmio_w2117(data);  //VMADDH
    case 0x2118: return mmio_w2118(data);  //VMDATAL
    case 0x2119: return mmio_w2119(data);  //VMDATAH
    case 0x211a: return mmio_w211a(data);  //M7SEL
    case 0x211b: return mmio_w211b(data);  //M7A
    case 0x211c: return mmio_w211c(data);  //M7B
    case 0x211d: return mmio_w211d(data);  //M7C
    case 0x211e: return mmio_w211e(data);  //M7D
    case 0x211f: return mmio_w211f(data);  //M7X
    case 0x2120: return mmio_w2120(data);  //M7Y
    case 0x2121: return mmio_w2121(data);  //CGADD
    case 0x2122: return mmio_w2122(data);  //CGDATA
    case 0x2123: return mmio_w2123(data);  //W12SEL
    case 0x2124: return mmio_w2124(data);  //W34SEL
    case 0x2125: return mmio_w2125(data);  //WOBJSEL
    case 0x2126: return mmio_w2126(data);  //WH0
    case 0x2127: return mmio_w2127(data);  //WH1
    case 0x2128: return mmio_w2128(data);  //WH2
    case 0x2129: return mmio_w2129(data);  //WH3
    case 0x212a: return mmio_w212a(data);  //WBGLOG
    case 0x212b: return mmio_w212b(data);  //WOBJLOG
    case 0x212c: return mmio_w212c(data);  //TM
    case 0x212d: return mmio_w212d(data);  //TS
    case 0x212e: return mmio_w212e(data);  //TMW
    case 0x212f: return mmio_w212f(data);  //TSW
    case 0x2130: return mmio_w2130(data);  //CGWSEL
    case 0x2131: return mmio_w2131(data);  //CGADDSUB
    case 0x2132: return mmio_w2132(data);  //COLDATA
    case 0x2133: return mmio_w2133(data);  //SETINI
  }
}

#endif
