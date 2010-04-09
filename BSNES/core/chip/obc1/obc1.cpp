#include <../base.hpp>

#define OBC1_CPP
namespace SNES {

OBC1 obc1;

#include "serialization.cpp"

void OBC1::init() {
}

void OBC1::enable() {
  bus.map(Bus::MapDirect, 0x00, 0x3f, 0x6000, 0x7fff, *this);
  bus.map(Bus::MapDirect, 0x80, 0xbf, 0x6000, 0x7fff, *this);
}

void OBC1::power() {
  reset();
}

void OBC1::reset() {
  for(unsigned i = 0x0000; i <= 0x1fff; i++) ram_write(i, 0xff);

  status.baseptr = (ram_read(0x1ff5) & 1) ? 0x1800 : 0x1c00;
  status.address = (ram_read(0x1ff6) & 0x7f);
  status.shift   = (ram_read(0x1ff6) & 3) << 1;
}

uint8 OBC1::read(unsigned addr) {
  addr &= 0x1fff;
  if((addr & 0x1ff8) != 0x1ff0) return ram_read(addr);

  switch(addr) { default:  //never used, avoids compiler warning
    case 0x1ff0: return ram_read(status.baseptr + (status.address << 2) + 0);
    case 0x1ff1: return ram_read(status.baseptr + (status.address << 2) + 1);
    case 0x1ff2: return ram_read(status.baseptr + (status.address << 2) + 2);
    case 0x1ff3: return ram_read(status.baseptr + (status.address << 2) + 3);
    case 0x1ff4: return ram_read(status.baseptr + (status.address >> 2) + 0x200);
    case 0x1ff5: case 0x1ff6: case 0x1ff7: return ram_read(addr);
  }
}

void OBC1::write(unsigned addr, uint8 data) {
  addr &= 0x1fff;
  if((addr & 0x1ff8) != 0x1ff0) return ram_write(addr, data);

  switch(addr) {
    case 0x1ff0: ram_write(status.baseptr + (status.address << 2) + 0, data); break;
    case 0x1ff1: ram_write(status.baseptr + (status.address << 2) + 1, data); break;
    case 0x1ff2: ram_write(status.baseptr + (status.address << 2) + 2, data); break;
    case 0x1ff3: ram_write(status.baseptr + (status.address << 2) + 3, data); break;
    case 0x1ff4: {
      uint8 temp = ram_read(status.baseptr + (status.address >> 2) + 0x200);
      temp = (temp & ~(3 << status.shift)) | ((data & 3) << status.shift);
      ram_write(status.baseptr + (status.address >> 2) + 0x200, temp);
    } break;
    case 0x1ff5: {
      status.baseptr = (data & 1) ? 0x1800 : 0x1c00;
      ram_write(addr, data);
    } break;
    case 0x1ff6: {
      status.address = (data & 0x7f);
      status.shift   = (data & 3) << 1;
      ram_write(addr, data);
    } break;
    case 0x1ff7: {
      ram_write(addr, data);
    } break;
  }
}

uint8 OBC1::ram_read(unsigned addr) {
  return memory::cartram.read(addr & 0x1fff);
}

void OBC1::ram_write(unsigned addr, uint8 data) {
  memory::cartram.write(addr & 0x1fff, data);
}

OBC1::OBC1() {}
OBC1::~OBC1() {}

};
