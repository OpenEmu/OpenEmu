#include <../base.hpp>

#define ST010_CPP
namespace SNES {

ST010 st010;

#include "st010_data.hpp"
#include "serialization.cpp"
#include "st010_op.cpp"

void ST010::init() {
}

void ST010::enable() {
  bus.map(Bus::MapDirect, 0x68, 0x6f, 0x0000, 0x0fff, *this);
  bus.map(Bus::MapDirect, 0xe8, 0xef, 0x0000, 0x0fff, *this);
}

int16 ST010::sin(int16 theta) {
  return sin_table[(theta >> 8) & 0xff];
}

int16 ST010::cos(int16 theta) {
  return sin_table[((theta + 0x4000) >> 8) & 0xff];
}

uint8 ST010::readb(uint16 addr) {
  return ram[addr & 0xfff];
}

uint16 ST010::readw(uint16 addr) {
  return (readb(addr + 0) <<  0) |
         (readb(addr + 1) <<  8);
}

uint32 ST010::readd(uint16 addr) {
  return (readb(addr + 0) <<  0) |
         (readb(addr + 1) <<  8) |
         (readb(addr + 2) << 16) |
         (readb(addr + 3) << 24);
}

void ST010::writeb(uint16 addr, uint8 data) {
  ram[addr & 0xfff] = data;
}

void ST010::writew(uint16 addr, uint16 data) {
  writeb(addr + 0, data >> 0);
  writeb(addr + 1, data >> 8);
}

void ST010::writed(uint16 addr, uint32 data) {
  writeb(addr + 0, data >>  0);
  writeb(addr + 1, data >>  8);
  writeb(addr + 2, data >> 16);
  writeb(addr + 3, data >> 24);
}

//

void ST010::power() {
  reset();
}

void ST010::reset() {
  memset(ram, 0x00, sizeof ram);
}

//

uint8 ST010::read(unsigned addr) {
  return readb(addr);
}

void ST010::write(unsigned addr, uint8 data) {
  writeb(addr, data);

  if((addr & 0xfff) == 0x0021 && (data & 0x80)) {
    switch(ram[0x0020]) {
      case 0x01: op_01(); break;
      case 0x02: op_02(); break;
      case 0x03: op_03(); break;
      case 0x04: op_04(); break;
      case 0x05: op_05(); break;
      case 0x06: op_06(); break;
      case 0x07: op_07(); break;
      case 0x08: op_08(); break;
    }

    ram[0x0021] &= ~0x80;
  }
}

};
