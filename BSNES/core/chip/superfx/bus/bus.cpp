#ifdef SUPERFX_CPP

SuperFXBus superfxbus;

namespace memory {
  static SuperFXGSUROM gsurom;
  static SuperFXGSURAM gsuram;
  static SuperFXCPUROM fxrom;
  static SuperFXCPURAM fxram;
};

void SuperFXBus::init() {
  map(MapDirect, 0x00, 0xff, 0x0000, 0xffff, memory::memory_unmapped);

  map(MapLinear, 0x00, 0x3f, 0x0000, 0x7fff, memory::gsurom);
  map(MapLinear, 0x00, 0x3f, 0x8000, 0xffff, memory::gsurom);
  map(MapLinear, 0x40, 0x5f, 0x0000, 0xffff, memory::gsurom);
  map(MapLinear, 0x60, 0x7f, 0x0000, 0xffff, memory::gsuram);

  bus.map(MapLinear, 0x00, 0x3f, 0x6000, 0x7fff, memory::fxram, 0x0000, 0x2000);
  bus.map(MapLinear, 0x00, 0x3f, 0x8000, 0xffff, memory::fxrom);
  bus.map(MapLinear, 0x40, 0x5f, 0x0000, 0xffff, memory::fxrom);
  bus.map(MapLinear, 0x60, 0x7d, 0x0000, 0xffff, memory::fxram);
  bus.map(MapLinear, 0x80, 0xbf, 0x6000, 0x7fff, memory::fxram, 0x0000, 0x2000);
  bus.map(MapLinear, 0x80, 0xbf, 0x8000, 0xffff, memory::fxrom);
  bus.map(MapLinear, 0xc0, 0xdf, 0x0000, 0xffff, memory::fxrom);
  bus.map(MapLinear, 0xe0, 0xff, 0x0000, 0xffff, memory::fxram);
}

//ROM / RAM access from the SuperFX CPU

unsigned SuperFXGSUROM::size() const {
  return memory::cartrom.size();
}

uint8 SuperFXGSUROM::read(unsigned addr) {
  while(!superfx.regs.scmr.ron) {
    superfx.add_clocks(6);
    scheduler.sync_copcpu();
  }
  return memory::cartrom.read(addr);
}

void SuperFXGSUROM::write(unsigned addr, uint8 data) {
  while(!superfx.regs.scmr.ron) {
    superfx.add_clocks(6);
    scheduler.sync_copcpu();
  }
  memory::cartrom.write(addr, data);
}

unsigned SuperFXGSURAM::size() const {
  return memory::cartram.size();
}

uint8 SuperFXGSURAM::read(unsigned addr) {
  while(!superfx.regs.scmr.ran) {
    superfx.add_clocks(6);
    scheduler.sync_copcpu();
  }
  return memory::cartram.read(addr);
}

void SuperFXGSURAM::write(unsigned addr, uint8 data) {
  while(!superfx.regs.scmr.ran) {
    superfx.add_clocks(6);
    scheduler.sync_copcpu();
  }
  memory::cartram.write(addr, data);
}

//ROM / RAM access from the S-CPU

unsigned SuperFXCPUROM::size() const {
  return memory::cartrom.size();
}

uint8 SuperFXCPUROM::read(unsigned addr) {
  if(superfx.regs.sfr.g && superfx.regs.scmr.ron) {
    static const uint8_t data[16] = {
      0x00, 0x01, 0x00, 0x01, 0x04, 0x01, 0x00, 0x01,
      0x00, 0x01, 0x08, 0x01, 0x00, 0x01, 0x0c, 0x01,
    };
    return data[addr & 15];
  }
  return memory::cartrom.read(addr);
}

void SuperFXCPUROM::write(unsigned addr, uint8 data) {
  memory::cartrom.write(addr, data);
}

unsigned SuperFXCPURAM::size() const {
  return memory::cartram.size();
}

uint8 SuperFXCPURAM::read(unsigned addr) {
  if(superfx.regs.sfr.g && superfx.regs.scmr.ran) return cpu.regs.mdr;
  return memory::cartram.read(addr);
}

void SuperFXCPURAM::write(unsigned addr, uint8 data) {
  memory::cartram.write(addr, data);
}

#endif
