struct NES_HKROM : Board {

MMC6 mmc6;

void main() {
  mmc6.main();
}

uint8 prg_read(unsigned addr) {
  if((addr & 0xf000) == 0x7000) return mmc6.ram_read(addr);
  if(addr & 0x8000) return prgrom.read(mmc6.prg_addr(addr));
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if((addr & 0xf000) == 0x7000) return mmc6.ram_write(addr, data);
  if(addr & 0x8000) return mmc6.reg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  mmc6.irq_test(addr);
  if(addr & 0x2000) return ppu.ciram_read(mmc6.ciram_addr(addr));
  return Board::chr_read(mmc6.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  mmc6.irq_test(addr);
  if(addr & 0x2000) return ppu.ciram_write(mmc6.ciram_addr(addr), data);
  return Board::chr_write(mmc6.chr_addr(addr), data);
}

void power() {
  mmc6.power();
}

void reset() {
  mmc6.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  mmc6.serialize(s);
}

NES_HKROM(XML::Document &document, const stream &memory) : Board(document, memory), mmc6(*this) {
}

};
