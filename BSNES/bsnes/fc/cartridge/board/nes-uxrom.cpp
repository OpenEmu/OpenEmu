//NES-UNROM
//NES-UOROM

struct NES_UxROM : Board {

struct Settings {
  bool mirror;  //0 = horizontal, 1 = vertical
} settings;

uint4 prg_bank;

uint8 prg_read(unsigned addr) {
  if((addr & 0xc000) == 0x8000) return prgrom.read((prg_bank << 14) | (addr & 0x3fff));
  if((addr & 0xc000) == 0xc000) return prgrom.read((    0x0f << 14) | (addr & 0x3fff));
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if(addr & 0x8000) prg_bank = data & 0x0f;
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) {
    if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    return ppu.ciram_read(addr);
  }
  return Board::chr_read(addr);
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) {
    if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    return ppu.ciram_write(addr, data);
  }
  return Board::chr_write(addr, data);
}

void power() {
}

void reset() {
  prg_bank = 0;
}

void serialize(serializer &s) {
  Board::serialize(s);

  s.integer(prg_bank);
}

NES_UxROM(XML::Document &document, const stream &memory) : Board(document, memory) {
  settings.mirror = document["cartridge"]["mirror"]["mode"].data == "vertical" ? 1 : 0;
}

};
