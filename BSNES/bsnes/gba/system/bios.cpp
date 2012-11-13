uint32 BIOS::read(uint32 addr, uint32 size) {
  //GBA BIOS is read-protected; only the BIOS itself can read its own memory
  //when accessed elsewhere; this returns the last value read by the BIOS program
  if(cpu.r(15) >= 0x02000000) return mdr;

  if(size == Word) return mdr = read(addr &~ 2, Half) << 0 | read(addr | 2, Half) << 16;
  if(size == Half) return mdr = read(addr &~ 1, Byte) << 0 | read(addr | 1, Byte) <<  8;
  return mdr = data[addr & 0x3fff];
}

void BIOS::write(uint32 addr, uint32 size, uint32 word) {
}

BIOS::BIOS() {
  data = new uint8[size = 16384]();
}

BIOS::~BIOS() {
  delete[] data;
}
