//Dev.ID   Size    Blocks  Manufacturer
//======  =====  ========  ============
//0xd4bf   64KB   16x4096  SST
//0x1cc2   64KB   16x4096  Macronix
//0x1b32   64KB   16x4096  Panasonic
//0x3d1f   64KB  512x 128  Atmel
//0x1362  128KB   32x4096  Sanyo
//0x09c2  128KB   32x4096  Macronix

uint8 Cartridge::FlashROM::read(uint16 addr) {
  if(idmode) {
    if(addr == 0x0000) return id >> 0;
    if(addr == 0x0001) return id >> 8;
    return 0u;
  }

  return data[bank << 16 | addr];
}

void Cartridge::FlashROM::write(uint16 addr, uint8 byte) {
  if(bankselect) {
    bankselect = false;
    //bank select is only applicable on 128KB chips
    if(addr == 0x0000) bank = byte & (size > 64 * 1024);
    return;
  }

  if(writeselect) {
    //Atmel writes 128 bytes per command; all others write 1 byte per command
    if(id != 0x3d1f || (addr & 0x007f) == 0x007f) writeselect = false;
    data[bank << 16 | addr] = byte;
    return;
  }

  if(byte == 0xaa && addr == 0x5555) { unlockhi = true; return; }
  if(byte == 0x55 && addr == 0x2aaa) { unlocklo = true; return; }

  if(unlockhi && unlocklo) {
    unlockhi = false;
    unlocklo = false;

    if(byte == 0x10 && addr == 0x5555) {
      if(erasemode) {
        erasemode = false;
        for(unsigned n = 0; n < size; n++) data[n] = 0xff;
      }
    }

    if(byte == 0x30 && (addr & 0x0fff) == 0x0000) {
      //command only valid for non-Atmel chips
      if(erasemode && id != 0x3d1f) {
        erasemode = false;
        unsigned offset = bank << 16 | (addr & ~4095);
        for(unsigned n = 0; n < 4096; n++) data[offset++] = 0xff;
      }
    }

    if(byte == 0x80 && addr == 0x5555) {
      erasemode = true;
    }

    if(byte == 0x90 && addr == 0x5555) {
      idmode = true;
    }

    if(byte == 0xa0 && addr == 0x5555) {
      writeselect = true;
    }

    if(byte == 0xb0 && addr == 0x5555) {
      bankselect = true;
    }

    if(byte == 0xf0 && addr == 0x5555) {
      idmode = false;
    }
  }
}

void Cartridge::FlashROM::power() {
  unlockhi = false;
  unlocklo = false;
  idmode = false;
  bankselect = false;
  writeselect = false;
  bank = 0;
}

void Cartridge::FlashROM::serialize(serializer &s) {
  s.array(data, size);
  s.integer(size);
  s.integer(id);
  s.integer(unlockhi);
  s.integer(unlocklo);
  s.integer(idmode);
  s.integer(erasemode);
  s.integer(bankselect);
  s.integer(writeselect);
  s.integer(bank);
}
