#ifdef CARTRIDGE_CPP

uint8 Cartridge::MBC1::mmio_read(uint16 addr) {
  if((addr & 0xc000) == 0x0000) {  //$0000-3fff
    return cartridge.rom_read(addr);
  }

  if((addr & 0xc000) == 0x4000) {  //$4000-7fff
    if(mode_select == 0) {
      return cartridge.rom_read((ram_select << 19) | (rom_select << 14) | (addr & 0x3fff));
    } else {
      return cartridge.rom_read((rom_select << 14) | (addr & 0x3fff));
    }
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    if(ram_enable) {
      if(mode_select == 0) {
        return cartridge.ram_read(addr & 0x1fff);
      } else {
        return cartridge.ram_read((ram_select << 13) | (addr & 0x1fff));
      }
    }
    return 0x00;
  }

  return 0x00;
}

void Cartridge::MBC1::mmio_write(uint16 addr, uint8 data) {
  if((addr & 0xe000) == 0x0000) {  //$0000-1fff
    ram_enable = (data & 0x0f) == 0x0a;
    return;
  }

  if((addr & 0xe000) == 0x2000) {  //$2000-3fff
    rom_select = (data & 0x1f) + ((data & 0x1f) == 0);
    return;
  }

  if((addr & 0xe000) == 0x4000) {  //$4000-5fff
    ram_select = data & 0x03;
    return;
  }

  if((addr & 0xe000) == 0x6000) {  //$6000-7fff
    mode_select = data & 0x01;
    return;
  }

  if((addr & 0xe000) == 0xa000) {  //$a000-bfff
    if(ram_enable) {
      if(mode_select == 0) {
        cartridge.ram_write(addr & 0x1fff, data);
      } else {
        cartridge.ram_write((ram_select << 13) | (addr & 0x1fff), data);
      }
    }
    return;
  }
}

void Cartridge::MBC1::power() {
  ram_enable = false;
  rom_select = 0x01;
  ram_select = 0x00;
  mode_select = 0;
}

#endif
