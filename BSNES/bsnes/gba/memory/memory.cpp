#include <gba/gba.hpp>

namespace GameBoyAdvance {

#include "mmio.cpp"
#include "serialization.cpp"
Bus bus;

struct UnmappedMemory : Memory {
  uint32 read(uint32 addr, uint32 size) { return 0u; }
  void write(uint32 addr, uint32 size, uint32 word) {}
};

static UnmappedMemory unmappedMemory;

uint32 Bus::mirror(uint32 addr, uint32 size) {
  uint32 base = 0;
  if(size) {
    uint32 mask = 1 << 27;  //28-bit bus
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

uint32 Bus::speed(uint32 addr, uint32 size) {
  if(addr & 0x08000000) {
    static unsigned timing[] = { 5, 4, 3, 9 };
    unsigned n = cpu.regs.wait.control.nwait[addr >> 25 & 3];
    unsigned s = cpu.regs.wait.control.swait[addr >> 25 & 3];
    n = timing[n];

    switch(addr >> 25 & 3) {
    case 0: s = s ? 3 : 2; break;
    case 1: s = s ? 5 : 2; break;
    case 2: s = s ? 9 : 2; break;
    case 3: s = n; break;
    }

    bool sequential = cpu.sequential();
    if((addr & 0xffff << 1) == 0) sequential = false;  //N cycle on 16-bit ROM crossing page boundary (RAM S==N)
    if(idleflag) sequential = false;  //LDR/LDM interrupts instruction fetches

    if(sequential) return s << (size == Word);  //16-bit bus requires two transfers for words
    if(size == Word) n += s;
    return n;
  }

  switch(addr >> 24 & 7) {
  case 0: return 1;
  case 1: return 1;
  case 2: return (1 + 15 - cpu.regs.memory.control.ewramwait) << (size == Word);
  case 3: return 1;
  case 4: return 1;
  case 5: return 1 << (size == Word);
  case 6: return 1 << (size == Word);
  case 7: return 1;
  }
}

void Bus::idle(uint32 addr) {
  if(addr & 0x08000000) idleflag = true;
}

uint32 Bus::read(uint32 addr, uint32 size) {
  idleflag = false;
  if(addr & 0x08000000) return cartridge.read(addr, size);

  switch(addr >> 24 & 7) {
  case 0: return bios.read(addr, size);
  case 1: return bios.read(addr, size);
  case 2: return cpu.ewram_read(addr, size);
  case 3: return cpu.iwram_read(addr, size);
  case 4:
    if((addr & 0xfffffc00) == 0x04000000) return mmio[addr & 0x3ff]->read(addr, size);
    if((addr & 0xff00ffff) == 0x04000800) return ((MMIO&)cpu).read(0x04000800 | (addr & 3), size);
    return cpu.pipeline.fetch.instruction;
  case 5: return ppu.pram_read(addr, size);
  case 6: return ppu.vram_read(addr, size);
  case 7: return ppu.oam_read(addr, size);
  }
}

void Bus::write(uint32 addr, uint32 size, uint32 word) {
  idleflag = false;
  if(addr & 0x08000000) return cartridge.write(addr, size, word);

  switch(addr >> 24 & 7) {
  case 0: return;
  case 1: return;
  case 2: return cpu.ewram_write(addr, size, word);
  case 3: return cpu.iwram_write(addr, size, word);
  case 4:
    if((addr & 0xfffffc00) == 0x04000000) return mmio[addr & 0x3ff]->write(addr, size, word);
    if((addr & 0xff00ffff) == 0x04000800) return ((MMIO&)cpu).write(0x04000800 | (addr & 3), size, word);
    return;
  case 5: return ppu.pram_write(addr, size, word);
  case 6: return ppu.vram_write(addr, size, word);
  case 7: return ppu.oam_write(addr, size, word);
  }
}

void Bus::power() {
  for(unsigned n = 0; n < 0x400; n++) mmio[n] = &unmappedMemory;
  idleflag = false;
}

}
