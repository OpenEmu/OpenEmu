//NES-NROM-128
//NES-NROM-256

struct NES_NROM : Board {

struct Settings {
  bool mirror;  //0 = horizontal, 1 = vertical
} settings;

uint8 prg_read(unsigned addr) {
  if(addr & 0x8000) return prgrom.read(addr);
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) {
    if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    return ppu.ciram_read(addr & 0x07ff);
  }
  if(chrram.size) return chrram.read(addr);
  return chrrom.read(addr);
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) {
    if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    return ppu.ciram_write(addr & 0x07ff, data);
  }
  if(chrram.size) return chrram.write(addr, data);
}

void serialize(serializer &s) {
  Board::serialize(s);
}

NES_NROM(XML::Document &document, const stream &memory) : Board(document, memory) {
  settings.mirror = document["cartridge"]["mirror"]["mode"].data == "vertical" ? 1 : 0;
}

};
