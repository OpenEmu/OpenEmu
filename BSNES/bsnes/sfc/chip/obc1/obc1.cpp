#include <sfc/sfc.hpp>

#define OBC1_CPP
namespace SuperFamicom {

#include "serialization.cpp"
OBC1 obc1;

void OBC1::init() {
}

void OBC1::load() {
}

void OBC1::unload() {
}

void OBC1::power() {
}

void OBC1::reset() {
  status.baseptr = (ram_read(0x1ff5) & 1) ? 0x1800 : 0x1c00;
  status.address = (ram_read(0x1ff6) & 0x7f);
  status.shift   = (ram_read(0x1ff6) & 3) << 1;
}

uint8 OBC1::read(unsigned addr) {
  addr &= 0x1fff;

  switch(addr) {
  case 0x1ff0: return ram_read(status.baseptr + (status.address << 2) + 0);
  case 0x1ff1: return ram_read(status.baseptr + (status.address << 2) + 1);
  case 0x1ff2: return ram_read(status.baseptr + (status.address << 2) + 2);
  case 0x1ff3: return ram_read(status.baseptr + (status.address << 2) + 3);
  case 0x1ff4: return ram_read(status.baseptr + (status.address >> 2) + 0x200);
  }

  return ram_read(addr);
}

void OBC1::write(unsigned addr, uint8 data) {
  addr &= 0x1fff;

  switch(addr) {
  case 0x1ff0: ram_write(status.baseptr + (status.address << 2) + 0, data); return;
  case 0x1ff1: ram_write(status.baseptr + (status.address << 2) + 1, data); return;
  case 0x1ff2: ram_write(status.baseptr + (status.address << 2) + 2, data); return;
  case 0x1ff3: ram_write(status.baseptr + (status.address << 2) + 3, data); return;
  case 0x1ff4: {
    uint8 temp = ram_read(status.baseptr + (status.address >> 2) + 0x200);
    temp = (temp & ~(3 << status.shift)) | ((data & 3) << status.shift);
    ram_write(status.baseptr + (status.address >> 2) + 0x200, temp);
  } return;
  case 0x1ff5:
    status.baseptr = (data & 1) ? 0x1800 : 0x1c00;
    ram_write(addr, data);
    return;
  case 0x1ff6:
    status.address = (data & 0x7f);
    status.shift   = (data & 3) << 1;
    ram_write(addr, data);
    return;
  case 0x1ff7:
    ram_write(addr, data);
    return;
  }

  return ram_write(addr, data);
}

uint8 OBC1::ram_read(unsigned addr) {
  return cartridge.ram.read(addr & 0x1fff);
}

void OBC1::ram_write(unsigned addr, uint8 data) {
  cartridge.ram.write(addr & 0x1fff, data);
}

}
