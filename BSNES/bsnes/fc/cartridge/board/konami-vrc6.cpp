struct KonamiVRC6 : Board {

VRC6 vrc6;

uint8 prg_read(unsigned addr) {
  if((addr & 0xe000) == 0x6000) return vrc6.ram_read(addr);
  if(addr & 0x8000) return prgrom.read(vrc6.prg_addr(addr));
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if((addr & 0xe000) == 0x6000) return vrc6.ram_write(addr, data);
  if(addr & 0x8000) {
    addr = (addr & 0xf003);
    if(prgram.size) addr = (addr & ~3) | ((addr & 2) >> 1) | ((addr & 1) << 1);
    return vrc6.reg_write(addr, data);
  }
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(vrc6.ciram_addr(addr));
  return Board::chr_read(vrc6.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(vrc6.ciram_addr(addr), data);
  return Board::chr_write(vrc6.chr_addr(addr), data);
}

void serialize(serializer &s) {
  Board::serialize(s);
  vrc6.serialize(s);
}

void main() { vrc6.main(); }
void power() { vrc6.power(); }
void reset() { vrc6.reset(); }

KonamiVRC6(XML::Document &document, const stream &memory) : Board(document, memory), vrc6(*this) {
}

};
