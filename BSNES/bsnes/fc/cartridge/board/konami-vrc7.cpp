struct KonamiVRC7 : Board {

VRC7 vrc7;

void main() {
  return vrc7.main();
}

uint8 prg_read(unsigned addr) {
  if(addr < 0x6000) return cpu.mdr();
  if(addr < 0x8000) return prgram.read(addr);
  return prgrom.read(vrc7.prg_addr(addr));
}

void prg_write(unsigned addr, uint8 data) {
  if(addr < 0x6000) return;
  if(addr < 0x8000) return prgram.write(addr, data);
  return vrc7.reg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(vrc7.ciram_addr(addr));
  return chrram.read(vrc7.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(vrc7.ciram_addr(addr), data);
  return chrram.write(vrc7.chr_addr(addr), data);
}

void power() {
  vrc7.power();
}

void reset() {
  vrc7.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  vrc7.serialize(s);
}

KonamiVRC7(XML::Document &document, const stream &memory) : Board(document, memory), vrc7(*this) {
}

};
