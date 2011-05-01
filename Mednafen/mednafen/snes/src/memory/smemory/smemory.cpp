#include <../base.hpp>

#define SMEMORY_CPP
namespace SNES {

sBus bus;

#include "system.cpp"
#include "generic.cpp"
#include "serialization.cpp"

void sBus::power() {
  for(unsigned i = 0x2000; i <= 0x5fff; i++) memory::mmio.map(i, memory::mmio_unmapped);
  for(unsigned i = 0; i < memory::wram.size(); i++) memory::wram[i] = config.cpu.wram_init_value;
}

void sBus::reset() {
}

bool sBus::load_cart() {
  if(cartridge.loaded() == true) return false;

  map_reset();
  map_generic();
  map_system();
  return true;
}

void sBus::unload_cart() {
}

sBus::sBus() {
}

sBus::~sBus() {
}

};
