#include <snes/snes.hpp>

#define PPU_CPP
namespace SNES {

#include "mmio.cpp"
PPU ppu;

void PPU::latch_counters() {
}

bool PPU::interlace() const {
  return false;
}

bool PPU::overscan() const {
  return false;
}

bool PPU::hires() const {
  return false;
}

void PPU::enter() {
  scanline();
  clock += lineclocks();
  tick(lineclocks());
}

void PPU::scanline() {
  if(vcounter() == 0) return frame();
  if(vcounter() > 224) return;
}

void PPU::frame() {
}

void PPU::enable() {
  bus.map(Bus::MapMode::Direct, 0x00, 0x3f, 0x2100, 0x213f, { &PPU::mmio_read, this }, { &PPU::mmio_write, this });
  bus.map(Bus::MapMode::Direct, 0x80, 0xbf, 0x2100, 0x213f, { &PPU::mmio_read, this }, { &PPU::mmio_write, this });
}

void PPU::power() {
  for(unsigned n = 0; n < 512 * 480; n++) output[n] = rand() & ((1 << 19) - 1);

  for(auto &n : vram) n = 0;
  for(auto &n : oam) n = 0;
  for(auto &n : cgram) n = 0;
  for(auto &n : r) n = 0;
  reset();
}

void PPU::reset() {
  PPUcounter::reset();
}

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
}

PPU::PPU() {
  surface = new uint32[512 * 512];
  output = surface + 16 * 512;
}

PPU::~PPU() {
  delete[] surface;
}

}
