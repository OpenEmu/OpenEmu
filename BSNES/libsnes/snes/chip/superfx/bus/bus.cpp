#ifdef SUPERFX_CPP

//ROM / RAM access from the S-CPU

unsigned SuperFX::ROM::size() const {
  return cartridge.rom.size();
}

uint8 SuperFX::ROM::read(unsigned addr) {
  if(superfx.regs.sfr.g && superfx.regs.scmr.ron) {
    static const uint8_t data[16] = {
      0x00, 0x01, 0x00, 0x01, 0x04, 0x01, 0x00, 0x01,
      0x00, 0x01, 0x08, 0x01, 0x00, 0x01, 0x0c, 0x01,
    };
    return data[addr & 15];
  }
  return cartridge.rom.read(addr);
}

void SuperFX::ROM::write(unsigned addr, uint8 data) {
  cartridge.rom.write(addr, data);
}

unsigned SuperFX::RAM::size() const {
  return cartridge.ram.size();
}

uint8 SuperFX::RAM::read(unsigned addr) {
  if(superfx.regs.sfr.g && superfx.regs.scmr.ran) return cpu.regs.mdr;
  return cartridge.ram.read(addr);
}

void SuperFX::RAM::write(unsigned addr, uint8 data) {
  cartridge.ram.write(addr, data);
}

#endif
