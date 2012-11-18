#ifdef HITACHIDSP_CPP

uint8 HitachiDSP::bus_read(uint24 addr) {
  if((addr & 0x408000) == 0x008000) return bus.read(addr);
  return 0x00;
}

void HitachiDSP::bus_write(uint24 addr, uint8 data) {
  if((addr & 0x40e000) == 0x006000) return bus.write(addr, data);
}

uint8 HitachiDSP::rom_read(unsigned addr) {
  if(co_active() == cpu.thread) {
    if(regs.halt) return cartridge.rom.read(addr);
    if((addr & 0x40ffe0) == 0x00ffe0) return mmio.vector[addr & 0x1f];
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
  if((addr >= 0x0000 && addr <= 0x0bff) || (addr >= 0x1000 && addr <= 0x1bff)) {
    return dataRAM[addr & 0x0fff];
  }

  //MMIO
  switch(addr) {
  case 0x1f40: return mmio.dma_source >>  0;
  case 0x1f41: return mmio.dma_source >>  8;
  case 0x1f42: return mmio.dma_source >> 16;
  case 0x1f43: return mmio.dma_length >>  0;
  case 0x1f44: return mmio.dma_length >>  8;
  case 0x1f45: return mmio.dma_target >>  0;
  case 0x1f46: return mmio.dma_target >>  8;
  case 0x1f47: return mmio.dma_target >> 16;
  case 0x1f48: return mmio.r1f48;
  case 0x1f49: return mmio.program_offset >>  0;
  case 0x1f4a: return mmio.program_offset >>  8;
  case 0x1f4b: return mmio.program_offset >> 16;
  case 0x1f4c: return mmio.r1f4c;
  case 0x1f4d: return mmio.page_number >> 0;
  case 0x1f4e: return mmio.page_number >> 8;
  case 0x1f4f: return mmio.program_counter;
  case 0x1f50: return mmio.r1f50;
  case 0x1f51: return mmio.r1f51;
  case 0x1f52: return mmio.r1f52;
  case 0x1f53: case 0x1f54: case 0x1f55: case 0x1f56:
  case 0x1f57: case 0x1f58: case 0x1f59: case 0x1f5a:
  case 0x1f5b: case 0x1f5c: case 0x1f5d: case 0x1f5e:
  case 0x1f5f: return ((regs.halt == false) << 6) | ((regs.halt == true) << 1);
  }

  //Vector
  if(addr >= 0x1f60 && addr <= 0x1f7f) {
    return mmio.vector[addr & 0x1f];
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
  if((addr >= 0x0000 && addr <= 0x0bff) || (addr >= 0x1000 && addr <= 0x1bff)) {
    dataRAM[addr & 0x0fff] = data;
    return;
  }

  //MMIO
  switch(addr) {
  case 0x1f40: mmio.dma_source = (mmio.dma_source & 0xffff00) | (data <<  0); return;
  case 0x1f41: mmio.dma_source = (mmio.dma_source & 0xff00ff) | (data <<  8); return;
  case 0x1f42: mmio.dma_source = (mmio.dma_source & 0x00ffff) | (data << 16); return;
  case 0x1f43: mmio.dma_length = (mmio.dma_length &   0xff00) | (data <<  0); return;
  case 0x1f44: mmio.dma_length = (mmio.dma_length &   0x00ff) | (data <<  8); return;
  case 0x1f45: mmio.dma_target = (mmio.dma_target & 0xffff00) | (data <<  0); return;
  case 0x1f46: mmio.dma_target = (mmio.dma_target & 0xff00ff) | (data <<  8); return;
  case 0x1f47: mmio.dma_target = (mmio.dma_target & 0x00ffff) | (data << 16);
    if(regs.halt) mmio.dma = true;
    return;
  case 0x1f48: mmio.r1f48 = data & 0x01; return;
  case 0x1f49: mmio.program_offset = (mmio.program_offset & 0xffff00) | (data <<  0); return;
  case 0x1f4a: mmio.program_offset = (mmio.program_offset & 0xff00ff) | (data <<  8); return;
  case 0x1f4b: mmio.program_offset = (mmio.program_offset & 0x00ffff) | (data << 16); return;
  case 0x1f4c: mmio.r1f4c = data & 0x03; return;
  case 0x1f4d: mmio.page_number = (mmio.page_number & 0x7f00) | ((data & 0xff) << 0); return;
  case 0x1f4e: mmio.page_number = (mmio.page_number & 0x00ff) | ((data & 0x7f) << 8); return;
  case 0x1f4f: mmio.program_counter = data;
    if(regs.halt) {
      regs.pc = mmio.page_number * 256 + mmio.program_counter;
      regs.halt = false;
    }
    return;
  case 0x1f50: mmio.r1f50 = data & 0x77; return;
  case 0x1f51: mmio.r1f51 = data & 0x01; return;
  case 0x1f52: mmio.r1f52 = data & 0x01; return;
  }

  //Vector
  if(addr >= 0x1f60 && addr <= 0x1f7f) {
    mmio.vector[addr & 0x1f] = data;
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
