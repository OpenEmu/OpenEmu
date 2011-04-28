//=============
//Cx4 emulation
//=============
//Used in Rockman X2/X3 (Megaman X2/X3)
//Portions (c) anomie, Overload, zsKnight, Nach, byuu

#include <../base.hpp>

#define CX4_CPP
namespace SNES {

Cx4 cx4;

#include "serialization.cpp"
#include "data.cpp"
#include "functions.cpp"
#include "oam.cpp"
#include "opcodes.cpp"

void Cx4::init() {
}

void Cx4::enable() {
  bus.map(Bus::MapDirect, 0x00, 0x3f, 0x6000, 0x7fff, *this);
  bus.map(Bus::MapDirect, 0x80, 0xbf, 0x6000, 0x7fff, *this);
}

uint32 Cx4::ldr(uint8 r) {
  uint16 addr = 0x0080 + (r * 3);
  return (reg[addr + 0] <<  0)
       | (reg[addr + 1] <<  8)
       | (reg[addr + 2] << 16);
}

void Cx4::str(uint8 r, uint32 data) {
  uint16 addr = 0x0080 + (r * 3);
  reg[addr + 0] = (data >>  0);
  reg[addr + 1] = (data >>  8);
  reg[addr + 2] = (data >> 16);
}

void Cx4::mul(uint32 x, uint32 y, uint32 &rl, uint32 &rh) {
  int64 rx = x & 0xffffff;
  int64 ry = y & 0xffffff;
  if(rx & 0x800000)rx |= ~0x7fffff;
  if(ry & 0x800000)ry |= ~0x7fffff;

  rx *= ry;

  rl = (rx)       & 0xffffff;
  rh = (rx >> 24) & 0xffffff;
}

uint32 Cx4::sin(uint32 rx) {
  r0 = rx & 0x1ff;
  if(r0 & 0x100)r0 ^= 0x1ff;
  if(r0 & 0x080)r0 ^= 0x0ff;
  if(rx & 0x100) {
    return sin_table[r0 + 0x80];
  } else {
    return sin_table[r0];
  }
}

uint32 Cx4::cos(uint32 rx) {
  return sin(rx + 0x080);
}

void Cx4::immediate_reg(uint32 start) {
  r0 = ldr(0);
  for(uint32 i = start; i < 48; i++) {
    if((r0 & 0x0fff) < 0x0c00) {
      ram[r0 & 0x0fff] = immediate_data[i];
    }
    r0++;
  }
  str(0, r0);
}

void Cx4::transfer_data() {
  uint32 src;
  uint16 dest, count;

  src   = (reg[0x40]) | (reg[0x41] << 8) | (reg[0x42] << 16);
  count = (reg[0x43]) | (reg[0x44] << 8);
  dest  = (reg[0x45]) | (reg[0x46] << 8);

  for(uint32 i=0;i<count;i++) {
    write(dest++, bus.read(src++));
  }
}

void Cx4::write(unsigned addr, uint8 data) {
  addr &= 0x1fff;

  if(addr < 0x0c00) {
    //ram
    ram[addr] = data;
    return;
  }

  if(addr < 0x1f00) {
    //unmapped
    return;
  }

  //command register
  reg[addr & 0xff] = data;

  if(addr == 0x1f47) {
    //memory transfer
    transfer_data();
    return;
  }

  if(addr == 0x1f4f) {
    //c4 command
    if(reg[0x4d] == 0x0e && !(data & 0xc3)) {
      //c4 test command
      reg[0x80] = data >> 2;
      return;
    }

    switch(data) {
      case 0x00: op00(); break;
      case 0x01: op01(); break;
      case 0x05: op05(); break;
      case 0x0d: op0d(); break;
      case 0x10: op10(); break;
      case 0x13: op13(); break;
      case 0x15: op15(); break;
      case 0x1f: op1f(); break;
      case 0x22: op22(); break;
      case 0x25: op25(); break;
      case 0x2d: op2d(); break;
      case 0x40: op40(); break;
      case 0x54: op54(); break;
      case 0x5c: op5c(); break;
      case 0x5e: op5e(); break;
      case 0x60: op60(); break;
      case 0x62: op62(); break;
      case 0x64: op64(); break;
      case 0x66: op66(); break;
      case 0x68: op68(); break;
      case 0x6a: op6a(); break;
      case 0x6c: op6c(); break;
      case 0x6e: op6e(); break;
      case 0x70: op70(); break;
      case 0x72: op72(); break;
      case 0x74: op74(); break;
      case 0x76: op76(); break;
      case 0x78: op78(); break;
      case 0x7a: op7a(); break;
      case 0x7c: op7c(); break;
      case 0x89: op89(); break;
    }
  }
}

void Cx4::writeb(uint16 addr, uint8 data) {
  write(addr,     data);
}

void Cx4::writew(uint16 addr, uint16 data) {
  write(addr + 0, data >> 0);
  write(addr + 1, data >> 8);
}

void Cx4::writel(uint16 addr, uint32 data) {
  write(addr + 0, data >>  0);
  write(addr + 1, data >>  8);
  write(addr + 2, data >> 16);
}

uint8 Cx4::read(unsigned addr) {
  addr &= 0x1fff;

  if(addr < 0x0c00) {
    return ram[addr];
  }

  if(addr >= 0x1f00) {
    return reg[addr & 0xff];
  }

  return cpu.regs.mdr;
}

uint8 Cx4::readb(uint16 addr) {
  return read(addr);
}

uint16 Cx4::readw(uint16 addr) {
  return read(addr) | (read(addr + 1) << 8);
}

uint32 Cx4::readl(uint16 addr) {
  return read(addr) | (read(addr + 1) << 8) + (read(addr + 2) << 16);
}

void Cx4::power() {
  reset();
}

void Cx4::reset() {
  memset(ram, 0, 0x0c00);
  memset(reg, 0, 0x0100);
}

};
