struct NES_SxROM : Board {

enum class Revision : unsigned {
  SAROM,
  SBROM,
  SCROM,
  SC1ROM,
  SEROM,
  SFROM,
  SGROM,
  SHROM,
  SH1ROM,
  SIROM,
  SJROM,
  SKROM,
  SLROM,
  SL1ROM,
  SL2ROM,
  SL3ROM,
  SLRROM,
  SMROM,
  SNROM,
  SOROM,
  SUROM,
  SXROM,
} revision;

MMC1 mmc1;

void main() {
  return mmc1.main();
}

unsigned ram_addr(unsigned addr) {
  unsigned bank = 0;
  if(revision == Revision::SOROM) bank = (mmc1.chr_bank[0] & 0x08) >> 3;
  if(revision == Revision::SUROM) bank = (mmc1.chr_bank[0] & 0x0c) >> 2;
  if(revision == Revision::SXROM) bank = (mmc1.chr_bank[0] & 0x0c) >> 2;
  return (bank << 13) | (addr & 0x1fff);
}

uint8 prg_read(unsigned addr) {
  if((addr & 0xe000) == 0x6000) {
    if(revision == Revision::SNROM) {
      if(mmc1.chr_bank[0] & 0x10) return cpu.mdr();
    }
    if(mmc1.ram_disable) return 0x00;
    return prgram.read(ram_addr(addr));
  }

  if(addr & 0x8000) {
    addr = mmc1.prg_addr(addr);
    if(revision == Revision::SXROM) {
      addr |= ((mmc1.chr_bank[0] & 0x10) >> 4) << 18;
    }
    return prgrom.read(addr);
  }

  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if((addr & 0xe000) == 0x6000) {
    if(revision == Revision::SNROM) {
      if(mmc1.chr_bank[0] & 0x10) return;
    }
    if(mmc1.ram_disable) return;
    return prgram.write(ram_addr(addr), data);
  }

  if(addr & 0x8000) return mmc1.mmio_write(addr, data);
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(mmc1.ciram_addr(addr));
  return Board::chr_read(mmc1.chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(mmc1.ciram_addr(addr), data);
  return Board::chr_write(mmc1.chr_addr(addr), data);
}

void power() {
  mmc1.power();
}

void reset() {
  mmc1.reset();
}

void serialize(serializer &s) {
  Board::serialize(s);
  mmc1.serialize(s);
}

NES_SxROM(XML::Document &document, const stream &memory) : Board(document, memory), mmc1(*this) {
  revision = Revision::SXROM;
}

};
