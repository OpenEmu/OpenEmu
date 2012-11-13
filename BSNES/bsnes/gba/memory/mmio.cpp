uint32 MMIO::read(uint32 addr, uint32 size) {
  uint32 word = 0;

  switch(size) {
  case Word:
    addr &= ~3;
    word |= read(addr + 0) <<  0;
    word |= read(addr + 1) <<  8;
    word |= read(addr + 2) << 16;
    word |= read(addr + 3) << 24;
    break;
  case Half:
    addr &= ~1;
    word |= read(addr + 0) <<  0;
    word |= read(addr + 1) <<  8;
    break;
  case Byte:
    word |= read(addr + 0) <<  0;
    break;
  }

  return word;
}

void MMIO::write(uint32 addr, uint32 size, uint32 word) {
  switch(size) {
  case Word:
    addr &= ~3;
    write(addr + 0, word >>  0);
    write(addr + 1, word >>  8);
    write(addr + 2, word >> 16);
    write(addr + 3, word >> 24);
    break;
  case Half:
    addr &= ~1;
    write(addr + 0, word >>  0);
    write(addr + 1, word >>  8);
    break;
  case Byte:
    write(addr + 0, word >>  0);
    break;
  }
}
