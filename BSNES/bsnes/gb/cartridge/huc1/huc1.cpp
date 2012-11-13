#ifdef CARTRIDGE_CPP

uint8 Cartridge::HuC1::mmio_read(uint16 addr) {
  if((addr & 0xc000) == 0x0000) {  //$0000-3fff
    return cartridge.rom_read(addr);
  }

  if((addr & 0xc000) == 0x4000) {  //$4000-7fff
    return cartridge.rom_read((rom_select << 14) | (addr & 0x3fff));
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    return cartridge.ram_read((ram_select << 13) | (addr & 0x1fff));
  }

  return 0x00;
}

void Cartridge::HuC1::mmio_write(uint16 addr, uint8 data) {
  if((addr & 0xe000) == 0x0000) {  //$0000-1fff
    ram_writable = (data & 0x0f) == 0x0a;
    return;
  }

  if((addr & 0xe000) == 0x2000) {  //$2000-3fff
    rom_select = data;
    if(rom_select == 0) rom_select = 1;
    return;
  }

  if((addr & 0xe000) == 0x4000) {  //$4000-5fff
    ram_select = data;
    return;
  }

  if((addr & 0xe000) == 0x6000) {  //$6000-7fff
    model = data & 0x01;
    return;
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    if(ram_writable == false) return;
    return cartridge.ram_write((ram_select << 13) | (addr & 0x1fff), data);
  }
}

void Cartridge::HuC1::power() {
  ram_writable = false;
  rom_select = 0x01;
  ram_select = 0x00;
  model = 0;
}

#endif
