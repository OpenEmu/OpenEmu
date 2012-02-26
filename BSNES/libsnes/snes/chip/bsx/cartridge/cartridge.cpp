#ifdef BSX_CPP

BSXCartridge bsxcartridge;

void BSXCartridge::init() {
}

void BSXCartridge::load() {
  sram.map(allocate<uint8>(32 * 1024, 0xff), 32 * 1024);
  sram.write_protect(false);
  cartridge.nvram.append(Cartridge::NonVolatileRAM( ".bss", sram.data(), sram.size() ));

  psram.map(allocate<uint8>(512 * 1024, 0xff), 512 * 1024);
  psram.write_protect(false);
  cartridge.nvram.append(Cartridge::NonVolatileRAM( ".bsp", psram.data(), psram.size() ));
}

void BSXCartridge::unload() {
}

void BSXCartridge::power() {
}

void BSXCartridge::reset() {
  for(unsigned i = 0; i < 16; i++) r[i] = 0x00;
  r[0x07] = 0x80;
  r[0x08] = 0x80;
  mmio_commit();
}

uint8 BSXCartridge::memory_access(bool write, Memory &memory, unsigned addr, uint8 data) {
  if(write == 0) return memory_read(memory, addr);
  memory_write(memory, addr, data);

  return 0; // FIXME: Can it reach here?
}

uint8 BSXCartridge::memory_read(Memory &memory, unsigned addr) {
  addr = bus.mirror(addr, memory.size());
  return memory.read(addr);
}

void BSXCartridge::memory_write(Memory &memory, unsigned addr, uint8 data) {
  addr = bus.mirror(addr, memory.size());
  return memory.write(addr, data);
}

//mcu_access() allows mcu_read() and mcu_write() to share decoding logic
uint8 BSXCartridge::mcu_access(bool write, unsigned addr, uint8 data) {
  if(within<0x00, 0x1f, 0x8000, 0xffff>(addr)) {
    if(r07 == 1) {
      addr = ((addr & 0x1f0000) >> 1) | (addr & 0x7fff);
      return memory_access(write, cartridge.rom, addr, data);
    }
  }

  if(within<0x80, 0x9f, 0x8000, 0xffff>(addr)) {
    if(r08 == 1) {
      addr = ((addr & 0x1f0000) >> 1) | (addr & 0x7fff);
      return memory_access(write, cartridge.rom, addr, data);
    }
  }

  if(within<0x20, 0x3f, 0x6000, 0x7fff>(addr)) {
    return memory_access(write, psram, addr, data);
  }

  if(within<0x40, 0x4f, 0x0000, 0xffff>(addr)) {
    if(r05 == 0) return memory_access(write, psram, addr & 0x0fffff, data);
  }

  if(within<0x50, 0x5f, 0x0000, 0xffff>(addr)) {
    if(r06 == 0) return memory_access(write, psram, addr & 0x0fffff, data);
  }

  if(within<0x60, 0x6f, 0x0000, 0xffff>(addr)) {
    if(r03 == 1) return memory_access(write, psram, addr & 0x0fffff, data);
  }

  if(within<0x70, 0x77, 0x0000, 0xffff>(addr)) {
    return memory_access(write, psram, addr & 0x07ffff, data);
  }

  if(within<0x00, 0x3f, 0x8000, 0xffff>(addr)
  || within<0x40, 0x7f, 0x0000, 0xffff>(addr)
  || within<0x80, 0xbf, 0x8000, 0xffff>(addr)
  || within<0xc0, 0xff, 0x0000, 0xffff>(addr)
  ) {
    if(r02 == 0) addr = ((addr & 0x7f0000) >> 1) | (addr & 0x7fff);
    Memory &memory = (r01 == 0 ? (Memory&)bsxflash : (Memory&)psram);
    return memory_access(write, memory, addr & 0x7fffff, data);
  }

  return cpu.regs.mdr;
}

uint8 BSXCartridge::mcu_read(unsigned addr) {
  return mcu_access(0, addr);
}

void BSXCartridge::mcu_write(unsigned addr, uint8 data) {
  mcu_access(1, addr, data);
}

uint8 BSXCartridge::mmio_read(unsigned addr) {
  if(within<0x00, 0x0f, 0x5000, 0x5000>(addr)) {
    uint8 n = (addr >> 16) & 15;
    return r[n];
  }

  if(within<0x10, 0x17, 0x5000, 0x5fff>(addr)) {
    return memory_read(sram, ((addr >> 16) & 7) * 0x1000 + (addr & 0xfff));
  }

  return 0x00;
}

void BSXCartridge::mmio_write(unsigned addr, uint8 data) {
  if(within<0x00, 0x0f, 0x5000, 0x5000>(addr)) {
    uint8 n = (addr >> 16) & 15;
    r[n] = data;
    if(n == 0x0e && data & 0x80) mmio_commit();
    return;
  }

  if(within<0x10, 0x17, 0x5000, 0x5fff>(addr)) {
    return memory_write(sram, ((addr >> 16) & 7) * 0x1000 + (addr & 0xfff), data);
  }
}

void BSXCartridge::mmio_commit() {
  r00 = r[0x00] & 0x80;
  r01 = r[0x01] & 0x80;
  r02 = r[0x02] & 0x80;
  r03 = r[0x03] & 0x80;
  r04 = r[0x04] & 0x80;
  r05 = r[0x05] & 0x80;
  r06 = r[0x06] & 0x80;
  r07 = r[0x07] & 0x80;
  r08 = r[0x08] & 0x80;
  r09 = r[0x09] & 0x80;
  r0a = r[0x0a] & 0x80;
  r0b = r[0x0b] & 0x80;
  r0c = r[0x0c] & 0x80;
  r0d = r[0x0d] & 0x80;
  r0e = r[0x0e] & 0x80;
  r0f = r[0x0f] & 0x80;
}

#endif
