uint32 CPU::iwram_read(uint32 addr, uint32 size) {
  if(regs.memory.control.disable) return cpu.pipeline.fetch.instruction;

  if(size == Word) return iwram_read(addr &~ 2, Half) << 0 | iwram_read(addr | 2, Half) << 16;
  if(size == Half) return iwram_read(addr &~ 1, Byte) << 0 | iwram_read(addr | 1, Byte) <<  8;

  return iwram[addr & 0x7fff];
}

void CPU::iwram_write(uint32 addr, uint32 size, uint32 word) {
  if(regs.memory.control.disable) return;

  if(size == Word) {
    iwram_write(addr &~2, Half, word >>  0);
    iwram_write(addr | 2, Half, word >> 16);
    return;
  }

  if(size == Half) {
    iwram_write(addr &~1, Byte, word >>  0);
    iwram_write(addr | 1, Byte, word >>  8);
    return;
  }

  iwram[addr & 0x7fff] = word;
}

uint32 CPU::ewram_read(uint32 addr, uint32 size) {
  if(regs.memory.control.disable) return cpu.pipeline.fetch.instruction;
  if(regs.memory.control.ewram == false) return iwram_read(addr, size);

  if(size == Word) return ewram_read(addr &~ 2, Half) << 0 | ewram_read(addr | 2, Half) << 16;
  if(size == Half) return ewram_read(addr &~ 1, Byte) << 0 | ewram_read(addr | 1, Byte) <<  8;

  return ewram[addr & 0x3ffff];
}

void CPU::ewram_write(uint32 addr, uint32 size, uint32 word) {
  if(regs.memory.control.disable) return;
  if(regs.memory.control.ewram == false) return iwram_write(addr, size, word);

  if(size == Word) {
    ewram_write(addr &~2, Half, word >>  0);
    ewram_write(addr | 2, Half, word >> 16);
    return;
  }

  if(size == Half) {
    ewram_write(addr &~1, Byte, word >>  0);
    ewram_write(addr | 1, Byte, word >>  8);
    return;
  }

  ewram[addr & 0x3ffff] = word;
}
