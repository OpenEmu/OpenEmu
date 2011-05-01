#ifdef BSX_CPP

BSXFlash bsxflash;

void BSXFlash::init() {}
void BSXFlash::enable() {}

void BSXFlash::power() {
  reset();
}

void BSXFlash::reset() {
  regs.command   = 0;
  regs.write_old = 0x00;
  regs.write_new = 0x00;

  regs.flash_enable = false;
  regs.read_enable  = false;
  regs.write_enable = false;
  memory::bsxflash.write_protect(!regs.write_enable);
}

unsigned BSXFlash::size() const {
  return memory::bsxflash.size();
}

uint8 BSXFlash::read(unsigned addr) {
  if(addr == 0x0002) {
    if(regs.flash_enable) return 0x80;
  }

  if(addr == 0x5555) {
    if(regs.flash_enable) return 0x80;
  }

  if(regs.read_enable && addr >= 0xff00 && addr <= 0xff13) {
    //read flash cartridge vendor information
    switch(addr - 0xff00) {
      case 0x00: return 0x4d;
      case 0x01: return 0x00;
      case 0x02: return 0x50;
      case 0x03: return 0x00;
      case 0x04: return 0x00;
      case 0x05: return 0x00;
      case 0x06: return 0x2a;  //0x2a = 8mbit, 0x2b = 16mbit (not known to exist, though BIOS recognizes ID)
      case 0x07: return 0x00;
      default:   return 0x00;
    }
  }

  return memory::bsxflash.read(addr);
}

void BSXFlash::write(unsigned addr, uint8 data) {
  //there exist both read-only and read-write BS-X flash cartridges ...
  //unfortunately, the vendor info is not stored inside memory dumps
  //of BS-X flashcarts, so it is impossible to determine whether a
  //given flashcart is writeable.
  //however, it has been observed that LoROM-mapped BS-X carts always
  //use read-write flashcarts, and HiROM-mapped BS-X carts always use
  //read-only flashcarts.
  //below is an unfortunately necessary workaround to this problem.
  if(cartridge.mapper() == Cartridge::BSCHiROM) return;

  if((addr & 0xff0000) == 0) {
    regs.write_old = regs.write_new;
    regs.write_new = data;

    if(regs.write_enable && regs.write_old == regs.write_new) {
      return memory::bsxflash.write(addr, data);
    }
  } else {
    if(regs.write_enable) {
      return memory::bsxflash.write(addr, data);
    }
  }

  if(addr == 0x0000) {
    regs.command <<= 8;
    regs.command  |= data;

    if((regs.command & 0xffff) == 0x38d0) {
      regs.flash_enable = true;
      regs.read_enable  = true;
    }
  }

  if(addr == 0x2aaa) {
    regs.command <<= 8;
    regs.command  |= data;
  }

  if(addr == 0x5555) {
    regs.command <<= 8;
    regs.command  |= data;

    if((regs.command & 0xffffff) == 0xaa5570) {
      regs.write_enable = false;
    }

    if((regs.command & 0xffffff) == 0xaa55a0) {
      regs.write_old = 0x00;
      regs.write_new = 0x00;
      regs.flash_enable = true;
      regs.write_enable = true;
    }

    if((regs.command & 0xffffff) == 0xaa55f0) {
      regs.flash_enable = false;
      regs.read_enable  = false;
      regs.write_enable = false;
    }

    memory::bsxflash.write_protect(!regs.write_enable);
  }
}

#endif

