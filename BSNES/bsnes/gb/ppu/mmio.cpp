#ifdef PPU_CPP

unsigned PPU::vram_addr(uint16 addr) const {
  return (status.vram_bank * 0x2000) + (addr & 0x1fff);
}

uint8 PPU::mmio_read(uint16 addr) {
  if(addr >= 0x8000 && addr <= 0x9fff) return vram[vram_addr(addr)];
  if(addr >= 0xfe00 && addr <= 0xfe9f) return oam[addr & 0xff];

  if(addr == 0xff40) {  //LCDC
    return (status.display_enable << 7)
         | (status.window_tilemap_select << 6)
         | (status.window_display_enable << 5)
         | (status.bg_tiledata_select << 4)
         | (status.bg_tilemap_select << 3)
         | (status.ob_size << 2)
         | (status.ob_enable << 1)
         | (status.bg_enable << 0);
  }

  if(addr == 0xff41) {  //STAT
    unsigned mode;
    if(status.ly >= 144) mode = 1;  //Vblank
    else if(status.lx < 80) mode = 2;  //OAM
    else if(status.lx < 252) mode = 3;  //LCD
    else mode = 0;  //Hblank

    return (status.interrupt_lyc << 6)
         | (status.interrupt_oam << 5)
         | (status.interrupt_vblank << 4)
         | (status.interrupt_hblank << 3)
         | ((status.ly == status.lyc) << 2)
         | (mode << 0);
  }

  if(addr == 0xff42) {  //SCY
    return status.scy;
  }

  if(addr == 0xff43) {  //SCX
    return status.scx;
  }

  if(addr == 0xff44) {  //LY
    return status.ly;
  }

  if(addr == 0xff45) {  //LYC
    return status.lyc;
  }

  if(addr == 0xff47) {  //BGP
    return (bgp[3] << 6)
         | (bgp[2] << 4)
         | (bgp[1] << 2)
         | (bgp[0] << 0);
  }

  if(addr == 0xff48) {  //OBP0
    return (obp[0][3] << 6)
         | (obp[0][2] << 4)
         | (obp[0][1] << 2)
         | (obp[0][0] << 0);
  }

  if(addr == 0xff49) {  //OBP1
    return (obp[1][3] << 6)
         | (obp[1][2] << 4)
         | (obp[1][1] << 2)
         | (obp[1][0] << 0);
  }

  if(addr == 0xff4a) {  //WY
    return status.wy;
  }

  if(addr == 0xff4b) {  //WX
    return status.wx;
  }

  if(addr == 0xff69) { //BGPD
    return bgpd[status.bgpi];
  }

  if(addr == 0xff6b) {  //OBPD
    return obpd[status.obpi];
  }

  return 0x00;
}

void PPU::mmio_write(uint16 addr, uint8 data) {
  if(addr >= 0x8000 && addr <= 0x9fff) { vram[vram_addr(addr)] = data; return; }
  if(addr >= 0xfe00 && addr <= 0xfe9f) { oam[addr & 0xff] = data; return; }

  if(addr == 0xff40) {  //LCDC
    if(status.display_enable == false && (data & 0x80)) {
      status.lx = 0;  //unverified behavior; fixes Super Mario Land 2 - Tree Zone
    }

    status.display_enable = data & 0x80;
    status.window_tilemap_select = data & 0x40;
    status.window_display_enable = data & 0x20;
    status.bg_tiledata_select = data & 0x10;
    status.bg_tilemap_select = data & 0x08;
    status.ob_size = data & 0x04;
    status.ob_enable = data & 0x02;
    status.bg_enable = data & 0x01;
    return;
  }

  if(addr == 0xff41) {  //STAT
    status.interrupt_lyc = data & 0x40;
    status.interrupt_oam = data & 0x20;
    status.interrupt_vblank = data & 0x10;
    status.interrupt_hblank = data & 0x08;
    return;
  }

  if(addr == 0xff42) {  //SCY
    status.scy = data;
    return;
  }

  if(addr == 0xff43) {  //SCX
    status.scx = data;
    return;
  }

  if(addr == 0xff44) {  //LY
    status.ly = 0;
    return;
  }

  if(addr == 0xff45) {  //LYC
    status.lyc = data;
    return;
  }

  if(addr == 0xff47) {  //BGP
    bgp[3] = (data >> 6) & 3;
    bgp[2] = (data >> 4) & 3;
    bgp[1] = (data >> 2) & 3;
    bgp[0] = (data >> 0) & 3;
    return;
  }

  if(addr == 0xff48) {  //OBP0
    obp[0][3] = (data >> 6) & 3;
    obp[0][2] = (data >> 4) & 3;
    obp[0][1] = (data >> 2) & 3;
    obp[0][0] = (data >> 0) & 3;
    return;
  }

  if(addr == 0xff49) {  //OBP1
    obp[1][3] = (data >> 6) & 3;
    obp[1][2] = (data >> 4) & 3;
    obp[1][1] = (data >> 2) & 3;
    obp[1][0] = (data >> 0) & 3;
    return;
  }

  if(addr == 0xff4a) {  //WY
    status.wy = data;
    return;
  }

  if(addr == 0xff4b) {  //WX
    status.wx = data;
    return;
  }

  if(addr == 0xff4f) {  //VBK
    status.vram_bank = data & 1;
    return;
  }

  if(addr == 0xff68) {  //BGPI
    status.bgpi_increment = data & 0x80;
    status.bgpi = data & 0x3f;
    return;
  }

  if(addr == 0xff69) {  //BGPD
    bgpd[status.bgpi] = data;
    if(status.bgpi_increment) status.bgpi++;
    return;
  }

  if(addr == 0xff6a) {  //OBPI
    status.obpi_increment = data & 0x80;
    status.obpi = data & 0x3f;
  }

  if(addr == 0xff6b) {  //OBPD
    obpd[status.obpi] = data;
    if(status.obpi_increment) status.obpi++;
  }
}

#endif
