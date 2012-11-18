struct KonamiVRC3 : Board {

struct Settings {
  bool mirror;  //0 = horizontal, 1 = vertical
} settings;

VRC3 vrc3;

void main() {
  vrc3.main();
}

uint8 prg_read(unsigned addr) {
  if((addr & 0xe000) == 0x6000) return prgram.read(addr & 0x1fff);
  if(addr & 0x8000) return prgrom.read(vrc3.prg_addr(addr));
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if((addr & 0xe000) == 0x6000) return prgram.write(addr & 0x1fff, data);
  if(addr & 0x8000) return vrc3.reg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) {
    if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    return ppu.ciram_read(addr & 0x07ff);
  }
  return chrram.read(addr);
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) {
    if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    return ppu.ciram_write(addr & 0x07ff, data);
  }
  return chrram.write(addr, data);
}

void power() {
  vrc3.power();
}

void reset() {
  vrc3.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  vrc3.serialize(s);
}

KonamiVRC3(XML::Document &document, const stream &memory) : Board(document, memory), vrc3(*this) {
  settings.mirror = document["cartridge"]["mirror"]["mode"].data == "vertical" ? 1 : 0;
}

};
