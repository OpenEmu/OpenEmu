#ifdef HITACHIDSP_CPP

uint8 HitachiDSP::bus_read(unsigned addr) {
  if((addr & 0x408000) == 0x008000) return bus.read(addr);
  return 0x00;
}

void HitachiDSP::bus_write(unsigned addr, uint8 data) {
  if((addr & 0x40e000) == 0x006000) return bus.write(addr, data);
}

uint8 HitachiDSP::rom_read(unsigned addr) {
  if(co_active() == cpu.thread) {
    if(state.i == State::Idle) return cartridge.rom.read(addr);
    if((addr & 0x40ffe0) == 0x00ffe0) return regs.vector[addr & 0x1f];
    return cpu.regs.mdr;
  }
  if(co_active() == hitachidsp.thread) {
    return cartridge.rom.read(addr);
  }
  return cpu.regs.mdr;
}

void HitachiDSP::rom_write(unsigned addr, uint8 data) {
}

uint8 HitachiDSP::dsp_read(unsigned addr) {
  addr &= 0x1fff;

  //Data RAM
  if((addr <= 0x0bff) || (addr >= 0x1000 && addr <= 0x1bff)) {
    return dataRAM[addr & 0x0fff];
  }

  //MMIO
  switch(addr) {
  case 0x1f40: return regs.dma_source >>  0;
  case 0x1f41: return regs.dma_source >>  8;
  case 0x1f42: return regs.dma_source >> 16;
  case 0x1f43: return regs.dma_length >>  0;
  case 0x1f44: return regs.dma_length >>  8;
  case 0x1f45: return regs.dma_target >>  0;
  case 0x1f46: return regs.dma_target >>  8;
  case 0x1f47: return regs.dma_target >> 16;
  case 0x1f48: return regs.r1f48;
  case 0x1f49: return regs.program_offset >>  0;
  case 0x1f4a: return regs.program_offset >>  8;
  case 0x1f4b: return regs.program_offset >> 16;
  case 0x1f4c: return regs.r1f4c;
  case 0x1f4d: return regs.page_number >> 0;
  case 0x1f4e: return regs.page_number >> 8;
  case 0x1f4f: return regs.program_counter;
  case 0x1f50: return regs.r1f50;
  case 0x1f51: return regs.r1f51;
  case 0x1f52: return regs.r1f52;
  case 0x1f53: case 0x1f54: case 0x1f55: case 0x1f56:
  case 0x1f57: case 0x1f58: case 0x1f59: case 0x1f5a:
  case 0x1f5b: case 0x1f5c: case 0x1f5d: case 0x1f5e:
  case 0x1f5f: return ((state.i != State::Idle) << 6) | ((state.i == State::Idle) << 1);
  }

  //Vector
  if(addr >= 0x1f60 && addr <= 0x1f7f) {
    return regs.vector[addr & 0x1f];
  }

  //GPRs
  if((addr >= 0x1f80 && addr <= 0x1faf) || (addr >= 0x1fc0 && addr <= 0x1fef)) {
    unsigned index = (addr & 0x3f) / 3;        //0..15
    unsigned shift = ((addr & 0x3f) % 3) * 8;  //0, 8, 16
    return regs.gpr[index] >> shift;
  }

  return 0x00;
}

void HitachiDSP::dsp_write(unsigned addr, uint8 data) {
  addr &= 0x1fff;

  //Data RAM
  if((addr <= 0x0bff) || (addr >= 0x1000 && addr <= 0x1bff)) {
    dataRAM[addr & 0x0fff] = data;
    return;
  }

  //MMIO
  switch(addr) {
  case 0x1f40: regs.dma_source = (regs.dma_source & 0xffff00) | (data <<  0); return;
  case 0x1f41: regs.dma_source = (regs.dma_source & 0xff00ff) | (data <<  8); return;
  case 0x1f42: regs.dma_source = (regs.dma_source & 0x00ffff) | (data << 16); return;
  case 0x1f43: regs.dma_length = (regs.dma_length &   0xff00) | (data <<  0); return;
  case 0x1f44: regs.dma_length = (regs.dma_length &   0x00ff) | (data <<  8); return;
  case 0x1f45: regs.dma_target = (regs.dma_target & 0xffff00) | (data <<  0); return;
  case 0x1f46: regs.dma_target = (regs.dma_target & 0xff00ff) | (data <<  8); return;
  case 0x1f47: regs.dma_target = (regs.dma_target & 0x00ffff) | (data << 16);
    if(state.i == State::Idle) state.i = State::DMA;
    return;
  case 0x1f48: regs.r1f48 = data & 0x01; return;
  case 0x1f49: regs.program_offset = (regs.program_offset & 0xffff00) | (data <<  0); return;
  case 0x1f4a: regs.program_offset = (regs.program_offset & 0xff00ff) | (data <<  8); return;
  case 0x1f4b: regs.program_offset = (regs.program_offset & 0x00ffff) | (data << 16); return;
  case 0x1f4c: regs.r1f4c = data & 0x03; return;
  case 0x1f4d: regs.page_number = (regs.page_number & 0x7f00) | ((data & 0xff) << 0); return;
  case 0x1f4e: regs.page_number = (regs.page_number & 0x00ff) | ((data & 0x7f) << 8); return;
  case 0x1f4f: regs.program_counter = data;
    if(state.i == State::Idle) {
      regs.pc = regs.page_number * 256 + regs.program_counter;
      state.i = State::Execute;
    }
    return;
  case 0x1f50: regs.r1f50 = data & 0x77; return;
  case 0x1f51: regs.r1f51 = data & 0x01; return;
  case 0x1f52: regs.r1f52 = data & 0x01; return;
  }

  //Vector
  if(addr >= 0x1f60 && addr <= 0x1f7f) {
    regs.vector[addr & 0x1f] = data;
    return;
  }

  //GPRs
  if((addr >= 0x1f80 && addr <= 0x1faf) || (addr >= 0x1fc0 && addr <= 0x1fef)) {
    unsigned index = (addr & 0x3f) / 3;
    switch((addr & 0x3f) % 3) {
    case 0: regs.gpr[index] = (regs.gpr[index] & 0xffff00) | (data <<  0); return;
    case 1: regs.gpr[index] = (regs.gpr[index] & 0xff00ff) | (data <<  8); return;
    case 2: regs.gpr[index] = (regs.gpr[index] & 0x00ffff) | (data << 16); return;
    }
  }
}

#endif
