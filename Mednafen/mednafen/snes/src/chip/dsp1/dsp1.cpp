#include <../base.hpp>

#define DSP1_CPP
namespace SNES {

DSP1 dsp1;

#include "serialization.cpp"
#include "dsp1emu.cpp"

void DSP1::init() {
}

void DSP1::enable() {
  switch(cartridge.dsp1_mapper()) {
    case Cartridge::DSP1LoROM1MB: {
      bus.map(Bus::MapDirect, 0x20, 0x3f, 0x8000, 0xffff, *this);
      bus.map(Bus::MapDirect, 0xa0, 0xbf, 0x8000, 0xffff, *this);
    } break;

    case Cartridge::DSP1LoROM2MB: {
      bus.map(Bus::MapDirect, 0x60, 0x6f, 0x0000, 0x7fff, *this);
      bus.map(Bus::MapDirect, 0xe0, 0xef, 0x0000, 0x7fff, *this);
    } break;

    case Cartridge::DSP1HiROM: {
      bus.map(Bus::MapDirect, 0x00, 0x1f, 0x6000, 0x7fff, *this);
      bus.map(Bus::MapDirect, 0x80, 0x9f, 0x6000, 0x7fff, *this);
    } break;
  }
}

void DSP1::power() {
  reset();
}

void DSP1::reset() {
  dsp1.reset();
}

/*****
 * addr_decode()
 * determine whether address is accessing
 * data register (DR) or status register (SR)
 * -- 0 (false) = DR
 * -- 1 (true ) = SR
 *
 * note: there is no need to bounds check addresses,
 * as memory mapper will not allow DSP1 accesses outside
 * of expected ranges
 *****/
bool DSP1::addr_decode(uint16 addr) {
  switch(cartridge.dsp1_mapper()) {
    case Cartridge::DSP1LoROM1MB: {
    //$[20-3f]:[8000-bfff] = DR, $[20-3f]:[c000-ffff] = SR
      return (addr >= 0xc000);
    }

    case Cartridge::DSP1LoROM2MB: {
    //$[60-6f]:[0000-3fff] = DR, $[60-6f]:[4000-7fff] = SR
      return (addr >= 0x4000);
    }

    case Cartridge::DSP1HiROM: {
    //$[00-1f]:[6000-6fff] = DR, $[00-1f]:[7000-7fff] = SR
      return (addr >= 0x7000);
    }
  }

  return 0;
}

uint8 DSP1::read(unsigned addr) {
  return (addr_decode(addr) == 0) ? dsp1.getDr() : dsp1.getSr();
}

void DSP1::write(unsigned addr, uint8 data) {
  if(addr_decode(addr) == 0) {
    dsp1.setDr(data);
  }
}

};
