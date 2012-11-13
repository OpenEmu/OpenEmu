#include <gb/gb.hpp>

//LY     =   0-153
//Raster =   0-143
//Vblank = 144-153

//LX     =   0-455

#define PPU_CPP
namespace GameBoy {

#include "mmio.cpp"
#include "dmg.cpp"
#include "cgb.cpp"
#include "serialization.cpp"
PPU ppu;

void PPU::Main() {
  ppu.main();
}

void PPU::main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    add_clocks(4);
    status.lx += 4;
    if(status.lx >= 456) scanline();

    if(status.display_enable && status.lx == 0) {
      if(status.interrupt_oam) cpu.interrupt_raise(CPU::Interrupt::Stat);
    }

    if(status.display_enable && status.lx == 252) {
      if(status.interrupt_hblank) cpu.interrupt_raise(CPU::Interrupt::Stat);
      cpu.hblank();
    }
  }
}

void PPU::add_clocks(unsigned clocks) {
  clock += clocks * cpu.frequency;
  if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) {
    co_switch(scheduler.active_thread = cpu.thread);
  }
}

void PPU::scanline() {
  status.lx -= 456;
  if(++status.ly == 154) frame();

  if(status.display_enable && status.interrupt_lyc == true) {
    if(status.ly == status.lyc) cpu.interrupt_raise(CPU::Interrupt::Stat);
  }

  if(status.ly < 144) {
    system.cgb() == false ? dmg_render() : cgb_render();
  }

  if(status.display_enable && status.ly == 144) {
    cpu.interrupt_raise(CPU::Interrupt::Vblank);
    if(status.interrupt_vblank) cpu.interrupt_raise(CPU::Interrupt::Stat);
  }
}

void PPU::frame() {
  cpu.mmio_joyp_poll();

  status.ly = 0;
  status.wyc = 0;
  scheduler.exit(Scheduler::ExitReason::FrameEvent);
}

unsigned PPU::hflip(unsigned data) const {
  return ((data & 0x8080) >> 7) | ((data & 0x4040) >> 5)
       | ((data & 0x2020) >> 3) | ((data & 0x1010) >> 1)
       | ((data & 0x0808) << 1) | ((data & 0x0404) << 3)
       | ((data & 0x0202) << 5) | ((data & 0x0101) << 7);
}

void PPU::power() {
  create(Main, 4 * 1024 * 1024);

  for(unsigned n = 0x8000; n <= 0x9fff; n++) bus.mmio[n] = this;  //VRAM
  for(unsigned n = 0xfe00; n <= 0xfe9f; n++) bus.mmio[n] = this;  //OAM

  bus.mmio[0xff40] = this;  //LCDC
  bus.mmio[0xff41] = this;  //STAT
  bus.mmio[0xff42] = this;  //SCY
  bus.mmio[0xff43] = this;  //SCX
  bus.mmio[0xff44] = this;  //LY
  bus.mmio[0xff45] = this;  //LYC
  bus.mmio[0xff47] = this;  //BGP
  bus.mmio[0xff48] = this;  //OBP0
  bus.mmio[0xff49] = this;  //OBP1
  bus.mmio[0xff4a] = this;  //WY
  bus.mmio[0xff4b] = this;  //WX

  if(system.cgb()) {
  bus.mmio[0xff4f] = this;  //VBK
  bus.mmio[0xff68] = this;  //BGPI
  bus.mmio[0xff69] = this;  //BGPD
  bus.mmio[0xff6a] = this;  //OBPI
  bus.mmio[0xff6b] = this;  //OBPD
  }

  for(auto &n : screen) n = 0x0000;
  for(auto &n : line) n = 0x0000;
  for(auto &n : origin) n = Origin::None;

  for(auto &n : vram) n = 0x00;
  for(auto &n : oam) n = 0x00;
  for(auto &n : bgp) n = 0x00;
  for(auto &n : obp[0]) n = 0x00;
  for(auto &n : obp[1]) n = 0x00;
  for(auto &n : bgpd) n = 0x0000;
  for(auto &n : obpd) n = 0x0000;

  status.lx = 0;
  status.wyc = 0;

  status.display_enable = 0;
  status.window_tilemap_select = 0;
  status.window_display_enable = 0;
  status.bg_tiledata_select = 0;
  status.bg_tilemap_select = 0;
  status.ob_size = 0;
  status.ob_enable = 0;
  status.bg_enable = 0;

  status.interrupt_lyc = 0;
  status.interrupt_oam = 0;
  status.interrupt_vblank = 0;
  status.interrupt_hblank = 0;

  status.scy = 0;
  status.scx = 0;
  status.ly = 0;
  status.lyc = 0;
  status.wy = 0;
  status.wx = 0;

  status.vram_bank = 0;

  status.bgpi_increment = 0;
  status.bgpi = 0;

  status.obpi_increment = 0;
  status.obpi = 0;
}

PPU::PPU() {
}

}
