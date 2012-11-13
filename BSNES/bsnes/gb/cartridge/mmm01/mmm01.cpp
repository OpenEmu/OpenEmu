#ifdef CARTRIDGE_CPP

uint8 Cartridge::MMM01::mmio_read(uint16 addr) {
  if((addr & 0x8000) == 0x0000) {  //$0000-7fff
    if(rom_mode == 0) return cartridge.rom_read(addr);
  }

  if((addr & 0xc000) == 0x0000) {  //$0000-3fff
    return cartridge.rom_read(0x8000 + (rom_base << 14) + (addr & 0x3fff));
  }

  if((addr & 0xc000) == 0x4000) {  //$4000-7fff
    return cartridge.rom_read(0x8000 + (rom_base << 14) + (rom_select << 14) + (addr & 0x3fff));
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    if(ram_enable) return cartridge.ram_read((ram_select << 13) + (addr & 0x1fff));
    return 0x00;
  }

  return 0x00;
}

void Cartridge::MMM01::mmio_write(uint16 addr, uint8 data) {
  if((addr & 0xe000) == 0x0000) {  //$0000-1fff
    if(rom_mode == 0) {
      rom_mode = 1;
    } else {
      ram_enable = (data & 0x0f) == 0x0a;
    }
  }

  if((addr & 0xe000) == 0x2000) {  //$2000-3fff
    if(rom_mode == 0) {
      rom_base = data & 0x3f;
    } else {
      rom_select = data;
    }
  }

  if((addr & 0xe000) == 0x4000) {  //$4000-5fff
    if(rom_mode == 1) {
      ram_select = data;
    }
  }

  if((addr & 0xe000) == 0x6000) {  //$6000-7fff
    //unknown purpose
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    if(ram_enable) cartridge.ram_write((ram_select << 13) + (addr & 0x1fff), data);
  }
}

void Cartridge::MMM01::power() {
  rom_mode = 0;
  rom_base = 0;

  ram_enable = false;
  rom_select = 0x01;
  ram_select = 0x00;
}

#endif
