#include <../base.hpp>

#define PPU_CPP
namespace SNES {

#include "serialization.cpp"

void PPU::enable_renderer(bool r) { status.render_output = r; }
bool PPU::renderer_enabled() { return status.render_output; }

void PPU::frame() {
  status.frame_executed = true;

  static int32 fr = 0, fe = 0;
  static time_t prev, curr;
  fe++;
  if(status.render_output)fr++;

  time(&curr);
  if(curr != prev) {
    status.frames_updated  = true;
    status.frames_rendered = fr;
    status.frames_executed = fe;
    fr = fe = 0;
  }
  prev = curr;
}

void PPU::power() {
  ppu1_version = config.ppu1.version;
  ppu2_version = config.ppu2.version;
}

void PPU::reset() {
  PPUcounter::reset();
  memset(output, 0, 512 * 480 * sizeof(uint16));
}

PPU::PPU() {
  output = new uint16[512 * 480];

  status.render_output   = true;
  status.frames_updated  = false;
  status.frames_rendered = 0;
  status.frames_executed = 0;
}

PPU::~PPU() {
  delete[] output;
}

};
