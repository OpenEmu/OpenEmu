#include <sfc/sfc.hpp>

#define PPU_CPP
namespace SuperFamicom {

PPU ppu;

#include "mmio/mmio.cpp"
#include "window/window.cpp"
#include "cache/cache.cpp"
#include "background/background.cpp"
#include "sprite/sprite.cpp"
#include "screen/screen.cpp"
#include "serialization.cpp"

void PPU::step(unsigned clocks) {
  clock += clocks;
}

void PPU::synchronize_cpu() {
  if(CPU::Threaded == true) {
    if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
  } else {
    while(clock >= 0) cpu.enter();
  }
}

void PPU::Enter() { ppu.enter(); }

void PPU::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    scanline();
    if(vcounter() < display.height && vcounter()) {
      add_clocks(512);
      render_scanline();
      add_clocks(lineclocks() - 512);
    } else {
      add_clocks(lineclocks());
    }
  }
}

void PPU::add_clocks(unsigned clocks) {
  tick(clocks);
  step(clocks);
  synchronize_cpu();
}

void PPU::render_scanline() {
  if(display.framecounter) return;  //skip this frame?
  bg1.scanline();
  bg2.scanline();
  bg3.scanline();
  bg4.scanline();
  if(regs.display_disable) return screen.render_black();
  screen.scanline();
  bg1.render();
  bg2.render();
  bg3.render();
  bg4.render();
  sprite.render();
  screen.render();
}

void PPU::scanline() {
  display.width = !hires() ? 256 : 512;
  display.height = !overscan() ? 225 : 240;
  if(vcounter() == 0) frame();
  if(vcounter() == display.height && regs.display_disable == false) sprite.address_reset();
}

void PPU::frame() {
  sprite.frame();
  system.frame();
  display.interlace = regs.interlace;
  display.overscan = regs.overscan;
  display.framecounter = display.frameskip == 0 ? 0 : (display.framecounter + 1) % display.frameskip;
}

void PPU::enable() {
  function<uint8 (unsigned)> read = { &PPU::mmio_read, (PPU*)&ppu };
  function<void (unsigned, uint8)> write = { &PPU::mmio_write, (PPU*)&ppu };

  bus.map(Bus::MapMode::Direct, 0x00, 0x3f, 0x2100, 0x213f, read, write);
  bus.map(Bus::MapMode::Direct, 0x80, 0xbf, 0x2100, 0x213f, read, write);
}

void PPU::power() {
  for(auto &n : vram) n = 0;
  for(auto &n : oam) n = 0;
  for(auto &n : cgram) n = 0;
  reset();
}

void PPU::reset() {
  create(Enter, system.cpu_frequency());
  PPUcounter::reset();
  memset(surface, 0, 512 * 512 * sizeof(uint32));
  mmio_reset();
  display.interlace = false;
  display.overscan = false;
}

void PPU::layer_enable(unsigned layer, unsigned priority, bool enable) {
  switch(layer * 4 + priority) {
    case  0: bg1.priority0_enable = enable; break;
    case  1: bg1.priority1_enable = enable; break;
    case  4: bg2.priority0_enable = enable; break;
    case  5: bg2.priority1_enable = enable; break;
    case  8: bg3.priority0_enable = enable; break;
    case  9: bg3.priority1_enable = enable; break;
    case 12: bg4.priority0_enable = enable; break;
    case 13: bg4.priority1_enable = enable; break;
    case 16: sprite.priority0_enable = enable; break;
    case 17: sprite.priority1_enable = enable; break;
    case 18: sprite.priority2_enable = enable; break;
    case 19: sprite.priority3_enable = enable; break;
  }
}

void PPU::set_frameskip(unsigned frameskip) {
  display.frameskip = frameskip;
  display.framecounter = 0;
}

PPU::PPU() :
cache(*this),
bg1(*this, Background::ID::BG1),
bg2(*this, Background::ID::BG2),
bg3(*this, Background::ID::BG3),
bg4(*this, Background::ID::BG4),
sprite(*this),
screen(*this) {
  surface = new uint32[512 * 512];
  output = surface + 16 * 512;
  display.width = 256;
  display.height = 224;
  display.frameskip = 0;
  display.framecounter = 0;
}

PPU::~PPU() {
  delete[] surface;
}

}
