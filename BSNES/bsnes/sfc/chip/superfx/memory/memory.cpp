#ifdef SUPERFX_CPP

uint8 SuperFX::bus_read(unsigned addr) {
  if((addr & 0xc00000) == 0x000000) {  //$00-3f:0000-7fff, $00-3f:8000-ffff
    while(!regs.scmr.ron && scheduler.sync != Scheduler::SynchronizeMode::All) {
      step(6);
      synchronize_cpu();
    }
    return cartridge.rom.read((((addr & 0x3f0000) >> 1) | (addr & 0x7fff)) & rom_mask);
  }

  if((addr & 0xe00000) == 0x400000) {  //$40-5f:0000-ffff
    while(!regs.scmr.ron && scheduler.sync != Scheduler::SynchronizeMode::All) {
      step(6);
      synchronize_cpu();
    }
    return cartridge.rom.read(addr & rom_mask);
  }

  if((addr & 0xe00000) == 0x600000) {  //$60-7f:0000-ffff
    while(!regs.scmr.ran && scheduler.sync != Scheduler::SynchronizeMode::All) {
      step(6);
      synchronize_cpu();
    }
    return cartridge.ram.read(addr & ram_mask);
  }
}

void SuperFX::bus_write(unsigned addr, uint8 data) {
  if((addr & 0xe00000) == 0x600000) {  //$60-7f:0000-ffff
    while(!regs.scmr.ran && scheduler.sync != Scheduler::SynchronizeMode::All) {
      step(6);
      synchronize_cpu();
    }
    return cartridge.ram.write(addr & ram_mask, data);
  }
}

uint8 SuperFX::op_read(uint16 addr) {
  uint16 offset = addr - regs.cbr;
  if(offset < 512) {
    if(cache.valid[offset >> 4] == false) {
      unsigned dp = offset & 0xfff0;
      unsigned sp = (regs.pbr << 16) + ((regs.cbr + dp) & 0xfff0);
      for(unsigned n = 0; n < 16; n++) {
        step(memory_access_speed);
        cache.buffer[dp++] = bus_read(sp++);
      }
      cache.valid[offset >> 4] = true;
    } else {
      step(cache_access_speed);
    }
    return cache.buffer[offset];
  }

  if(regs.pbr <= 0x5f) {
    //$[00-5f]:[0000-ffff] ROM
    rombuffer_sync();
    step(memory_access_speed);
    return bus_read((regs.pbr << 16) + addr);
  } else {
    //$[60-7f]:[0000-ffff] RAM
    rambuffer_sync();
    step(memory_access_speed);
    return bus_read((regs.pbr << 16) + addr);
  }
}

uint8 SuperFX::peekpipe() {
  uint8 result = regs.pipeline;
  regs.pipeline = op_read(regs.r[15]);
  r15_modified = false;
  return result;
}

uint8 SuperFX::pipe() {
  uint8 result = regs.pipeline;
  regs.pipeline = op_read(++regs.r[15]);
  r15_modified = false;
  return result;
}

void SuperFX::cache_flush() {
  for(unsigned n = 0; n < 32; n++) cache.valid[n] = false;
}

uint8 SuperFX::cache_mmio_read(uint16 addr) {
  addr = (addr + regs.cbr) & 511;
  return cache.buffer[addr];
}

void SuperFX::cache_mmio_write(uint16 addr, uint8 data) {
  addr = (addr + regs.cbr) & 511;
  cache.buffer[addr] = data;
  if((addr & 15) == 15) cache.valid[addr >> 4] = true;
}

void SuperFX::memory_reset() {
  rom_mask = cartridge.rom.size() - 1;
  ram_mask = cartridge.ram.size() - 1;

  for(unsigned n = 0; n < 512; n++) cache.buffer[n] = 0x00;
  for(unsigned n = 0; n < 32; n++) cache.valid[n] = false;
  for(unsigned n = 0; n < 2; n++) {
    pixelcache[n].offset = ~0;
    pixelcache[n].bitpend = 0x00;
  }
}

#endif
