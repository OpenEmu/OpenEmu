#include <../base.hpp>

#define DSP4_CPP
namespace SNES {

DSP4 dsp4;

void DSP4::init() {
}

void DSP4::enable() {
  bus.map(Bus::MapDirect, 0x30, 0x3f, 0x8000, 0xffff, *this);
  bus.map(Bus::MapDirect, 0xb0, 0xbf, 0x8000, 0xffff, *this);
}

namespace DSP4i {
  inline uint16 READ_WORD(uint8 *addr) {
    return (addr[0]) + (addr[1] << 8);
  }

  inline uint32 READ_DWORD(uint8 *addr) {
    return (addr[0]) + (addr[1] << 8) + (addr[2] << 16) + (addr[3] << 24);
  }

  inline void WRITE_WORD(uint8 *addr, uint16 data) {
    addr[0] = data;
    addr[1] = data >> 8;
  }

  #define bool8 uint8
  #include "dsp4emu.c"
  #undef bool8
};

void DSP4::power() {
  reset();
}

void DSP4::reset() {
  DSP4i::InitDSP4();
}

uint8 DSP4::read(unsigned addr) {
  addr &= 0xffff;
  if(addr < 0xc000) {
    DSP4i::dsp4_address = addr;
    DSP4i::DSP4GetByte();
    return DSP4i::dsp4_byte;
  }
  return 0x80;
}

void DSP4::write(unsigned addr, uint8 data) {
  addr &= 0xffff;
  if(addr < 0xc000) {
    DSP4i::dsp4_address = addr;
    DSP4i::dsp4_byte = data;
    DSP4i::DSP4SetByte();
  }
}

};
