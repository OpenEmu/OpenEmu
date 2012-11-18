struct KonamiVRC2 : Board {

struct Settings {
  struct Pinout {
    unsigned a0;
    unsigned a1;
  } pinout;
} settings;

VRC2 vrc2;

uint8 prg_read(unsigned addr) {
  if(addr < 0x6000) return cpu.mdr();
  if(addr < 0x8000) return vrc2.ram_read(addr);
  return prgrom.read(vrc2.prg_addr(addr));
}

void prg_write(unsigned addr, uint8 data) {
  if(addr < 0x6000) return;
  if(addr < 0x8000) return vrc2.ram_write(addr, data);

  bool a0 = (addr & settings.pinout.a0);
  bool a1 = (addr & settings.pinout.a1);
  addr &= 0xfff0;
  addr |= (a0 << 0) | (a1 << 1);
  return vrc2.reg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(vrc2.ciram_addr(addr));
  return Board::chr_read(vrc2.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(vrc2.ciram_addr(addr), data);
  return Board::chr_write(vrc2.chr_addr(addr), data);
}

void power() {
  vrc2.power();
}

void reset() {
  vrc2.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  vrc2.serialize(s);
}

KonamiVRC2(XML::Document &document, const stream &memory) : Board(document, memory), vrc2(*this) {
  settings.pinout.a0 = 1 << decimal(document["cartridge"]["chip"]["pinout"]["a0"].data);
  settings.pinout.a1 = 1 << decimal(document["cartridge"]["chip"]["pinout"]["a1"].data);
}

};
