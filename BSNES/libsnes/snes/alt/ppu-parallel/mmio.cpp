#ifdef PPU_CPP

bool PPU::display_disable() const { return r[0x00] & 0x80; }
unsigned PPU::display_brightness() const { return r[0x00] & 0x0f; }

uint8 PPU::mmio_read(unsigned addr) {
  return cpu.regs.mdr;
}

void PPU::mmio_write(unsigned addr, uint8 data) {
  r[addr & 0x3f] = data;
}

#endif
