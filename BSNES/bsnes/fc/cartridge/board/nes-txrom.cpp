struct NES_TxROM : Board {

enum class Revision : unsigned {
  TBROM,
  TEROM,
  TFROM,
  TGROM,
  TKROM,
  TKSROM,
  TLROM,
  TL1ROM,
  TL2ROM,
  TLSROM,
  TNROM,
  TQROM,
  TR1ROM,
  TSROM,
  TVROM,
} revision;

MMC3 mmc3;

void main() {
  mmc3.main();
}

uint8 prg_read(unsigned addr) {
  if((addr & 0xe000) == 0x6000) return mmc3.ram_read(addr);
  if(addr & 0x8000) return prgrom.read(mmc3.prg_addr(addr));
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if((addr & 0xe000) == 0x6000) return mmc3.ram_write(addr, data);
  if(addr & 0x8000) return mmc3.reg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  mmc3.irq_test(addr);
  if(addr & 0x2000) return ppu.ciram_read(mmc3.ciram_addr(addr));
  return Board::chr_read(mmc3.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  mmc3.irq_test(addr);
  if(addr & 0x2000) return ppu.ciram_write(mmc3.ciram_addr(addr), data);
  return Board::chr_write(mmc3.chr_addr(addr), data);
}

void power() {
  mmc3.power();
}

void reset() {
  mmc3.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  mmc3.serialize(s);
}

NES_TxROM(XML::Document &document, const stream &memory) : Board(document, memory), mmc3(*this) {
  revision = Revision::TLROM;
}

};
