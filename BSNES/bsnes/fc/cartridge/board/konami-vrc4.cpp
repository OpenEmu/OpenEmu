struct KonamiVRC4 : Board {

struct Settings {
  struct Pinout {
    unsigned a0;
    unsigned a1;
  } pinout;
} settings;

VRC4 vrc4;

void main() {
  return vrc4.main();
}

uint8 prg_read(unsigned addr) {
  if(addr < 0x6000) return cpu.mdr();
  if(addr < 0x8000) return prgram.read(addr);
  return prgrom.read(vrc4.prg_addr(addr));
}

void prg_write(unsigned addr, uint8 data) {
  if(addr < 0x6000) return;
  if(addr < 0x8000) return prgram.write(addr, data);

  bool a0 = (addr & settings.pinout.a0);
  bool a1 = (addr & settings.pinout.a1);
  addr &= 0xfff0;
  addr |= (a1 << 1) | (a0 << 0);
  return vrc4.reg_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(vrc4.ciram_addr(addr));
  return Board::chr_read(vrc4.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(vrc4.ciram_addr(addr), data);
  return Board::chr_write(vrc4.chr_addr(addr), data);
}

void power() {
  vrc4.power();
}

void reset() {
  vrc4.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  vrc4.serialize(s);
}

KonamiVRC4(XML::Document &document, const stream &memory) : Board(document, memory), vrc4(*this) {
  settings.pinout.a0 = 1 << decimal(document["cartridge"]["chip"]["pinout"]["a0"].data);
  settings.pinout.a1 = 1 << decimal(document["cartridge"]["chip"]["pinout"]["a1"].data);
}

};
