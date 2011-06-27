#include <../base.hpp>

#define MEMORY_CPP
namespace SNES {

namespace memory {
  MMIOAccess mmio;
  StaticRAM  wram(128 * 1024);
  StaticRAM  apuram(64 * 1024);
  StaticRAM  vram(64 * 1024);
  StaticRAM  oam(544);
  StaticRAM  cgram(512);

  UnmappedMemory memory_unmapped;
  UnmappedMMIO   mmio_unmapped;
};

unsigned UnmappedMemory::size() const { return 16 * 1024 * 1024; }
uint8 UnmappedMemory::read(unsigned) { return cpu.regs.mdr; }
void UnmappedMemory::write(unsigned, uint8) {}

uint8 UnmappedMMIO::mmio_read(unsigned) { return cpu.regs.mdr; }
void UnmappedMMIO::mmio_write(unsigned, uint8) {}

void MMIOAccess::map(unsigned addr, MMIO &access) {
  //MMIO: $[00-3f]:[2000-5fff]
  mmio[(addr - 0x2000) & 0x3fff] = &access;
}

uint8 MMIOAccess::read(unsigned addr) {
  return mmio[(addr - 0x2000) & 0x3fff]->mmio_read(addr);
}

void MMIOAccess::write(unsigned addr, uint8 data) {
  mmio[(addr - 0x2000) & 0x3fff]->mmio_write(addr, data);
}

unsigned Bus::mirror(unsigned addr, unsigned size) {
  unsigned base = 0;
  if(size) {
    unsigned mask = 1 << 23;
    while(addr >= size) {
      while(!(addr & mask)) mask >>= 1;
      addr -= mask;
      if(size > mask) {
        size -= mask;
        base += mask;
      }
      mask >>= 1;
    }
    base += addr;
  }
  return base;
}

void Bus::map(unsigned addr, Memory &access, unsigned offset) {
  page[addr >> 8].access = &access;
  page[addr >> 8].offset = offset - addr;
}

void Bus::map(
  MapMode mode,
  uint8  bank_lo, uint8  bank_hi,
  uint16 addr_lo, uint16 addr_hi,
  Memory &access, unsigned offset, unsigned size
) {
  assert(bank_lo <= bank_hi);
  assert(addr_lo <= addr_hi);
  if(access.size() == -1U) return;

  uint8 page_lo = addr_lo >> 8;
  uint8 page_hi = addr_hi >> 8;
  unsigned index = 0;

  switch(mode) {
    case MapDirect: {
      for(unsigned bank = bank_lo; bank <= bank_hi; bank++) {
        for(unsigned page = page_lo; page <= page_hi; page++) {
          map((bank << 16) + (page << 8), access, (bank << 16) + (page << 8));
        }
      }
    } break;

    case MapLinear: {
      for(unsigned bank = bank_lo; bank <= bank_hi; bank++) {
        for(unsigned page = page_lo; page <= page_hi; page++) {
          map((bank << 16) + (page << 8), access, mirror(offset + index, access.size()));
          index += 256;
          if(size) index %= size;
        }
      }
    } break;

    case MapShadow: {
      for(unsigned bank = bank_lo; bank <= bank_hi; bank++) {
        index += page_lo * 256;
        if(size) index %= size;

        for(unsigned page = page_lo; page <= page_hi; page++) {
          map((bank << 16) + (page << 8), access, mirror(offset + index, access.size()));
          index += 256;
          if(size) index %= size;
        }

        index += (255 - page_hi) * 256;
        if(size) index %= size;
      }
    } break;
  }
}

};
