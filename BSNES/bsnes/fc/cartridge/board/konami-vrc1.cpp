struct KonamiVRC1 : Board {

VRC1 vrc1;

uint8 prg_read(unsigned addr) {
  if(addr & 0x8000) return prgrom.read(vrc1.prg_addr(addr));
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if(addr & 0x8000) return vrc1.reg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(vrc1.ciram_addr(addr));
  return Board::chr_read(vrc1.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(vrc1.ciram_addr(addr), data);
  return Board::chr_write(vrc1.chr_addr(addr), data);
}

void power() {
  vrc1.power();
}

void reset() {
  vrc1.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  vrc1.serialize(s);
}

KonamiVRC1(XML::Document &document, const stream &memory) : Board(document, memory), vrc1(*this) {
}

};
