struct NES_ExROM : Board {

enum class Revision : unsigned {
  EKROM,
  ELROM,
  ETROM,
  EWROM,
} revision;

MMC5 mmc5;

void main() {
  mmc5.main();
}

uint8 prg_read(unsigned addr) {
  return mmc5.prg_read(addr);
}

void prg_write(unsigned addr, uint8 data) {
  mmc5.prg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  return mmc5.chr_read(addr);
}

void chr_write(unsigned addr, uint8 data) {
  mmc5.chr_write(addr, data);
}

void scanline(unsigned y) {
  mmc5.scanline(y);
}

void power() {
  mmc5.power();
}

void reset() {
  mmc5.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  mmc5.serialize(s);
}

NES_ExROM(XML::Document &document, const stream &memory) : Board(document, memory), mmc5(*this) {
  revision = Revision::ELROM;
}

};
