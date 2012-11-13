#ifdef ARMDSP_CPP

void ArmDSP::bus_idle(uint32 addr) {
  step(1);
}

uint32 ArmDSP::bus_read(uint32 addr, uint32 size) {
  step(1);

  static auto memory = [&](const uint8 *memory, uint32 addr, uint32 size) -> uint32 {
    switch(size) {
    case Word:
      memory += addr & ~3;
      return memory[0] << 0 | memory[1] << 8 | memory[2] << 16 | memory[3] << 24;
    case Byte:
      return memory[addr];
    }
  };

  switch(addr & 0xe0000000) {
  case 0x00000000: return memory(programROM, addr & 0x1ffff, size);
  case 0x20000000: return pipeline.fetch.instruction;
  case 0x40000000: break;
  case 0x60000000: return 0x40404001;
  case 0x80000000: return pipeline.fetch.instruction;
  case 0xa0000000: return memory(dataROM, addr & 0x7fff, size);
  case 0xc0000000: return pipeline.fetch.instruction;
  case 0xe0000000: return memory(programRAM, addr & 0x3fff, size);
  }

  addr &= 0xe000003f;

  if(addr == 0x40000010) {
    if(bridge.cputoarm.ready) {
      bridge.cputoarm.ready = false;
      return bridge.cputoarm.data;
    }
  }

  if(addr == 0x40000020) {
    return bridge.status();
  }

  return 0u;
}

void ArmDSP::bus_write(uint32 addr, uint32 size, uint32 word) {
  step(1);

  static auto memory = [](uint8 *memory, uint32 addr, uint32 size, uint32 word) {
    switch(size) {
    case Word:
      memory += addr & ~3;
      *memory++ = word >>  0;
      *memory++ = word >>  8;
      *memory++ = word >> 16;
      *memory++ = word >> 24;
      break;
    case Byte:
      memory += addr;
      *memory++ = word >>  0;
      break;
    }
  };

  switch(addr & 0xe0000000) {
  case 0x00000000: return;
  case 0x20000000: return;
  case 0x40000000: break;
  case 0x60000000: return;
  case 0x80000000: return;
  case 0xa0000000: return;
  case 0xc0000000: return;
  case 0xe0000000: return memory(programRAM, addr & 0x3fff, size, word);
  }

  addr &= 0xe000003f;
  word &= 0x000000ff;

  if(addr == 0x40000000) {
    bridge.armtocpu.ready = true;
    bridge.armtocpu.data = word;
    return;
  }

  if(addr == 0x40000010) {
    bridge.signal = true;
    return;
  }

  if(addr == 0x40000020) { bridge.timerlatch = (bridge.timerlatch & 0xffff00) | (word <<  0); return; }
  if(addr == 0x40000024) { bridge.timerlatch = (bridge.timerlatch & 0xff00ff) | (word <<  8); return; }
  if(addr == 0x40000028) { bridge.timerlatch = (bridge.timerlatch & 0x00ffff) | (word << 16); return; }

  if(addr == 0x4000002c) {
    bridge.timer = bridge.timerlatch;
    return;
  }
}

#endif
