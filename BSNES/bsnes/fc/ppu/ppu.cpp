#include <fc/fc.hpp>

namespace Famicom {

#include "serialization.cpp"
PPU ppu;

void PPU::Main() {
  ppu.main();
}

void PPU::main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::PPU) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    raster_scanline();
  }
}

void PPU::tick() {
  if(status.ly == 240 && status.lx == 340) status.nmi_hold = 1;
  if(status.ly == 241 && status.lx ==   0) status.nmi_flag = status.nmi_hold;
  if(status.ly == 241 && status.lx ==   2) cpu.set_nmi_line(status.nmi_enable && status.nmi_flag);

  if(status.ly == 260 && status.lx == 340) status.sprite_zero_hit = 0, status.sprite_overflow = 0;

  if(status.ly == 260 && status.lx == 340) status.nmi_hold = 0;
  if(status.ly == 261 && status.lx ==   0) status.nmi_flag = status.nmi_hold;
  if(status.ly == 261 && status.lx ==   2) cpu.set_nmi_line(status.nmi_enable && status.nmi_flag);

  clock += 4;
  if(clock >= 0) co_switch(cpu.thread);

  status.lx++;
}

void PPU::scanline() {
  status.lx = 0;
  if(++status.ly == 262) {
    status.ly = 0;
    frame();
  }
  cartridge.scanline(status.ly);
}

void PPU::frame() {
  status.field ^= 1;
  scheduler.exit(Scheduler::ExitReason::FrameEvent);
}

void PPU::power() {
}

void PPU::reset() {
  create(PPU::Main, 21477272);

  status.mdr = 0x00;
  status.field = 0;
  status.ly = 0;
  status.bus_data = 0x00;
  status.address_latch = 0;

  status.vaddr = 0x0000;
  status.taddr = 0x0000;
  status.xaddr = 0x00;

  status.nmi_hold = 0;
  status.nmi_flag = 0;

  //$2000
  status.nmi_enable = false;
  status.master_select = 0;
  status.sprite_size = 0;
  status.bg_addr = 0x0000;
  status.sprite_addr = 0x0000;
  status.vram_increment = 1;

  //$2001
  status.emphasis = 0;
  status.sprite_enable = false;
  status.bg_enable = false;
  status.sprite_edge_enable = false;
  status.bg_edge_enable = false;
  status.grayscale = false;

  //$2002
  status.sprite_zero_hit = false;
  status.sprite_overflow = false;

  //$2003
  status.oam_addr = 0x00;

  for(auto &n : buffer) n = 0;
  for(auto &n : ciram ) n = 0;
  for(auto &n : cgram ) n = 0;
  for(auto &n : oam   ) n = 0;
}

uint8 PPU::read(uint16 addr) {
  uint8 result = 0x00;

  switch(addr & 7) {
  case 2:  //PPUSTATUS
    result |= status.nmi_flag << 7;
    result |= status.sprite_zero_hit << 6;
    result |= status.sprite_overflow << 5;
    result |= status.mdr & 0x1f;
    status.address_latch = 0;
    status.nmi_hold = 0;
    cpu.set_nmi_line(status.nmi_flag = 0);
    break;
  case 4:  //OAMDATA
    result = oam[status.oam_addr];
    if((status.oam_addr & 3) == 3) result &= 0xe3;
    break;
  case 7:  //PPUDATA
    if(raster_enable() && (status.ly <= 240 || status.ly == 261)) return 0x00;

    addr = status.vaddr & 0x3fff;
    if(addr <= 0x1fff) {
      result = status.bus_data;
      status.bus_data = cartridge.chr_read(addr);
    } else if(addr <= 0x3eff) {
      result = status.bus_data;
      status.bus_data = cartridge.chr_read(addr);
    } else if(addr <= 0x3fff) {
      result = cgram_read(addr);
      status.bus_data = cartridge.chr_read(addr);
    }
    status.vaddr += status.vram_increment;
    break;
  }

  return result;
}

void PPU::write(uint16 addr, uint8 data) {
  status.mdr = data;

  switch(addr & 7) {
  case 0:  //PPUCTRL
    status.nmi_enable = data & 0x80;
    status.master_select = data & 0x40;
    status.sprite_size = data & 0x20;
    status.bg_addr = (data & 0x10) ? 0x1000 : 0x0000;
    status.sprite_addr = (data & 0x08) ? 0x1000 : 0x0000;
    status.vram_increment = (data & 0x04) ? 32 : 1;
    status.taddr = (status.taddr & 0x73ff) | ((data & 0x03) << 10);
    cpu.set_nmi_line(status.nmi_enable && status.nmi_hold && status.nmi_flag);
    return;
  case 1:  //PPUMASK
    status.emphasis = data >> 5;
    status.sprite_enable = data & 0x10;
    status.bg_enable = data & 0x08;
    status.sprite_edge_enable = data & 0x04;
    status.bg_edge_enable = data & 0x02;
    status.grayscale = data & 0x01;
    return;
  case 2:  //PPUSTATUS
    return;
  case 3:  //OAMADDR
    status.oam_addr = data;
    return;
  case 4:  //OAMDATA
    oam[status.oam_addr++] = data;
    return;
  case 5:  //PPUSCROLL
    if(status.address_latch == 0) {
      status.xaddr = data & 0x07;
      status.taddr = (status.taddr & 0x7fe0) | (data >> 3);
    } else {
      status.taddr = (status.taddr & 0x0c1f) | ((data & 0x07) << 12) | ((data >> 3) << 5);
    }
    status.address_latch ^= 1;
    return;
  case 6:  //PPUADDR
    if(status.address_latch == 0) {
      status.taddr = (status.taddr & 0x00ff) | ((data & 0x3f) << 8);
    } else {
      status.taddr = (status.taddr & 0x7f00) | data;
      status.vaddr = status.taddr;
    }
    status.address_latch ^= 1;
    return;
  case 7:  //PPUDATA
    if(raster_enable() && (status.ly <= 240 || status.ly == 261)) return;

    addr = status.vaddr & 0x3fff;
    if(addr <= 0x1fff) {
      cartridge.chr_write(addr, data);
    } else if(addr <= 0x3eff) {
      cartridge.chr_write(addr, data);
    } else if(addr <= 0x3fff) {
      cgram_write(addr, data);
    }
    status.vaddr += status.vram_increment;
    return;
  }
}

uint8 PPU::ciram_read(uint16 addr) {
  return ciram[addr & 0x07ff];
}

void PPU::ciram_write(uint16 addr, uint8 data) {
  ciram[addr & 0x07ff] = data;
}

uint8 PPU::cgram_read(uint16 addr) {
  if((addr & 0x13) == 0x10) addr &= ~0x10;
  uint8 data = cgram[addr & 0x1f];
  if(status.grayscale) data &= 0x30;
  return data;
}

void PPU::cgram_write(uint16 addr, uint8 data) {
  if((addr & 0x13) == 0x10) addr &= ~0x10;
  cgram[addr & 0x1f] = data;
}

//

//vaddr = 0yyy VHYY  YYYX XXXX
//yyy = fine Yscroll (y:d0-d2)
//V = V nametable (y:d8)
//H = H nametable (x:d8)
//YYYYY = Y nametable (y:d3-d7)
//XXXXX = X nametable (x:d3-d7)

bool PPU::raster_enable() const {
  return (status.bg_enable || status.sprite_enable);
}

unsigned PPU::nametable_addr() const {
  return 0x2000 + (status.vaddr & 0x0c00);
}

unsigned PPU::scrollx() const {
  return ((status.vaddr & 0x1f) << 3) | status.xaddr;
}

unsigned PPU::scrolly() const {
  return (((status.vaddr >> 5) & 0x1f) << 3) | ((status.vaddr >> 12) & 7);
}

unsigned PPU::sprite_height() const {
  return status.sprite_size == 0 ? 8 : 16;
}

//

uint8 PPU::chr_load(uint16 addr) {
  if(raster_enable() == false) return 0x00;
  return cartridge.chr_read(addr);
}

//

void PPU::scrollx_increment() {
  if(raster_enable() == false) return;
  status.vaddr = (status.vaddr & 0x7fe0) | ((status.vaddr + 0x0001) & 0x001f);
  if((status.vaddr & 0x001f) == 0x0000) {
    status.vaddr ^= 0x0400;
  }
}

void PPU::scrolly_increment() {
  if(raster_enable() == false) return;
  status.vaddr = (status.vaddr & 0x0fff) | ((status.vaddr + 0x1000) & 0x7000);
  if((status.vaddr & 0x7000) == 0x0000) {
    status.vaddr = (status.vaddr & 0x7c1f) | ((status.vaddr + 0x0020) & 0x03e0);
    if((status.vaddr & 0x03e0) == 0x03c0) {  //0x03c0 == 30 << 5; 30 * 8 = 240
      status.vaddr &= 0x7c1f;
      status.vaddr ^= 0x0800;
    }
  }
}

//

void PPU::raster_pixel() {
  uint32 *output = buffer + status.ly * 256;

  unsigned mask = 0x8000 >> (status.xaddr + (status.lx & 7));
  unsigned palette = 0, object_palette = 0;
  bool object_priority = 0;
  palette |= (raster.tiledatalo & mask) ? 1 : 0;
  palette |= (raster.tiledatahi & mask) ? 2 : 0;
  if(palette) {
    unsigned attr = raster.attribute;
    if(mask >= 256) attr >>= 2;
    palette |= (attr & 3) << 2;
  }

  if(status.bg_enable == false) palette = 0;
  if(status.bg_edge_enable == false && status.lx < 8) palette = 0;

  if(status.sprite_enable == true)
  for(signed sprite = 7; sprite >= 0; sprite--) {
    if(status.sprite_edge_enable == false && status.lx < 8) continue;
    if(raster.oam[sprite].id == 64) continue;

    unsigned spritex = status.lx - raster.oam[sprite].x;
    if(spritex >= 8) continue;

    if(raster.oam[sprite].attr & 0x40) spritex ^= 7;
    unsigned mask = 0x80 >> spritex;
    unsigned sprite_palette = 0;
    sprite_palette |= (raster.oam[sprite].tiledatalo & mask) ? 1 : 0;
    sprite_palette |= (raster.oam[sprite].tiledatahi & mask) ? 2 : 0;
    if(sprite_palette == 0) continue;

    if(raster.oam[sprite].id == 0 && palette && status.lx != 255) status.sprite_zero_hit = 1;
    sprite_palette |= (raster.oam[sprite].attr & 3) << 2;

    object_priority = raster.oam[sprite].attr & 0x20;
    object_palette = 16 + sprite_palette;
  }

  if(object_palette) {
    if(palette == 0 || object_priority == 0) palette = object_palette;
  }

  if(raster_enable() == false) palette = 0;
  output[status.lx] = video.palette[(status.emphasis << 6) | cgram_read(palette)];
}

void PPU::raster_sprite() {
  if(raster_enable() == false) return;

  unsigned n = raster.oam_iterator++;
  signed ly = (status.ly == 261 ? -1 : status.ly);
  unsigned y = ly - oam[(n * 4) + 0];

  if(y >= sprite_height()) return;
  if(raster.oam_counter == 8) {
    status.sprite_overflow = 1;
    return;
  }

  raster.soam[raster.oam_counter].id   = n;
  raster.soam[raster.oam_counter].y    = oam[(n * 4) + 0];
  raster.soam[raster.oam_counter].tile = oam[(n * 4) + 1];
  raster.soam[raster.oam_counter].attr = oam[(n * 4) + 2];
  raster.soam[raster.oam_counter].x    = oam[(n * 4) + 3];
  raster.oam_counter++;
}

void PPU::raster_scanline() {
  if((status.ly >= 240 && status.ly <= 260)) {
    for(unsigned x = 0; x < 341; x++) tick();
    return scanline();
  }

  raster.oam_iterator = 0;
  raster.oam_counter = 0;

  for(unsigned n = 0; n < 8; n++) {
    raster.soam[n].id   = 64;
    raster.soam[n].y    = 0xff;
    raster.soam[n].tile = 0xff;
    raster.soam[n].attr = 0xff;
    raster.soam[n].x    = 0xff;
    raster.soam[n].tiledatalo = 0;
    raster.soam[n].tiledatahi = 0;
  }

  for(unsigned tile = 0; tile < 32; tile++) {  //  0-255
    unsigned nametable = chr_load(0x2000 | (status.vaddr & 0x0fff));
    unsigned tileaddr = status.bg_addr + (nametable << 4) + (scrolly() & 7);
    raster_pixel();
    tick();

    raster_pixel();
    tick();

    unsigned attribute = chr_load(0x23c0 | (status.vaddr & 0x0fc0) | ((scrolly() >> 5) << 3) | (scrollx() >> 5));
    if(scrolly() & 16) attribute >>= 4;
    if(scrollx() & 16) attribute >>= 2;
    raster_pixel();
    tick();

    scrollx_increment();
    if(tile == 31) scrolly_increment();
    raster_pixel();
    raster_sprite();
    tick();

    unsigned tiledatalo = chr_load(tileaddr + 0);
    raster_pixel();
    tick();

    raster_pixel();
    tick();

    unsigned tiledatahi = chr_load(tileaddr + 8);
    raster_pixel();
    tick();

    raster_pixel();
    raster_sprite();
    tick();

    raster.nametable = (raster.nametable << 8) | nametable;
    raster.attribute = (raster.attribute << 2) | (attribute & 3);
    raster.tiledatalo = (raster.tiledatalo << 8) | tiledatalo;
    raster.tiledatahi = (raster.tiledatahi << 8) | tiledatahi;
  }

  for(unsigned n = 0; n < 8; n++) raster.oam[n] = raster.soam[n];

  for(unsigned sprite = 0; sprite < 8; sprite++) {  //256-319
    unsigned nametable = chr_load(0x2000 | (status.vaddr & 0x0fff));
    tick();

    if(raster_enable() && sprite == 0) status.vaddr = (status.vaddr & 0x7be0) | (status.taddr & 0x041f);  //257
    tick();

    unsigned attribute = chr_load(0x23c0 | (status.vaddr & 0x0fc0) | ((scrolly() >> 5) << 3) | (scrollx() >> 5));
    unsigned tileaddr = (sprite_height() == 8)
    ? status.sprite_addr + raster.oam[sprite].tile * 16
    : ((raster.oam[sprite].tile & ~1) * 16) + ((raster.oam[sprite].tile & 1) * 0x1000);
    tick();
    tick();

    unsigned spritey = (status.ly - raster.oam[sprite].y) & (sprite_height() - 1);
    if(raster.oam[sprite].attr & 0x80) spritey ^= (sprite_height() - 1);
    tileaddr += spritey + (spritey & 8);

    raster.oam[sprite].tiledatalo = chr_load(tileaddr + 0);
    tick();
    tick();

    raster.oam[sprite].tiledatahi = chr_load(tileaddr + 8);
    tick();
    tick();

    if(raster_enable() && sprite == 6 && status.ly == 261) status.vaddr = status.taddr;  //304
  }

  for(unsigned tile = 0; tile < 2; tile++) {  //320-335
    unsigned nametable = chr_load(0x2000 | (status.vaddr & 0x0fff));
    unsigned tileaddr = status.bg_addr + (nametable << 4) + (scrolly() & 7);
    tick();
    tick();

    unsigned attribute = chr_load(0x23c0 | (status.vaddr & 0x0fc0) | ((scrolly() >> 5) << 3) | (scrollx() >> 5));
    if(scrolly() & 16) attribute >>= 4;
    if(scrollx() & 16) attribute >>= 2;
    tick();

    scrollx_increment();
    tick();

    unsigned tiledatalo = chr_load(tileaddr + 0);
    tick();
    tick();

    unsigned tiledatahi = chr_load(tileaddr + 8);
    tick();
    tick();

    raster.nametable = (raster.nametable << 8) | nametable;
    raster.attribute = (raster.attribute << 2) | (attribute & 3);
    raster.tiledatalo = (raster.tiledatalo << 8) | tiledatalo;
    raster.tiledatahi = (raster.tiledatahi << 8) | tiledatahi;
  }

  //336-339
  chr_load(0x2000 | (status.vaddr & 0x0fff));
  tick();
  bool skip = (raster_enable() && status.field == 1 && status.ly == 261);
  tick();

  chr_load(0x2000 | (status.vaddr & 0x0fff));
  tick();
  tick();

  //340
  if(skip == false) tick();

  return scanline();
}

}
