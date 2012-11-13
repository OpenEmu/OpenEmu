//MMC2

struct NES_PxROM : Board {

enum Revision : unsigned {
  PEEOROM,
  PNROM,
} revision;

uint4 prg_bank;
uint5 chr_bank[2][2];
bool mirror;
bool latch[2];

uint8 prg_read(unsigned addr) {
  if(addr < 0x6000) return cpu.mdr();
  if(addr < 0x8000) return prgram.read(addr);
  unsigned bank = 0;
  switch((addr / 0x2000) & 3) {
  case 0: bank = prg_bank; break;
  case 1: bank = 0x0d; break;
  case 2: bank = 0x0e; break;
  case 3: bank = 0x0f; break;
  }
  return prgrom.read((bank * 0x2000) | (addr & 0x1fff));
}

void prg_write(unsigned addr, uint8 data) {
  if(addr < 0x6000) return;
  if(addr < 0x8000) return prgram.write(addr, data);

  switch(addr & 0xf000) {
  case 0xa000: prg_bank = data & 0x0f; break;
  case 0xb000: chr_bank[0][0] = data & 0x1f; break;
  case 0xc000: chr_bank[0][1] = data & 0x1f; break;
  case 0xd000: chr_bank[1][0] = data & 0x1f; break;
  case 0xe000: chr_bank[1][1] = data & 0x1f; break;
  case 0xf000: mirror = data & 0x01; break;
  }
}

unsigned ciram_addr(unsigned addr) const {
  switch(mirror) {
  case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);  //vertical mirroring
  case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);  //horizontal mirroring
  }
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(ciram_addr(addr));
  bool region = addr & 0x1000;
  unsigned bank = chr_bank[region][latch[region]];
  if((addr & 0x0ff8) == 0x0fd8) latch[region] = 0;
  if((addr & 0x0ff8) == 0x0fe8) latch[region] = 1;
  return Board::chr_read((bank * 0x1000) | (addr & 0x0fff));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(ciram_addr(addr), data);
  bool region = addr & 0x1000;
  unsigned bank = chr_bank[region][latch[region]];
  if((addr & 0x0ff8) == 0x0fd8) latch[region] = 0;
  if((addr & 0x0ff8) == 0x0fe8) latch[region] = 1;
  return Board::chr_write((bank * 0x1000) | (addr & 0x0fff), data);
}

void power() {
}

void reset() {
  prg_bank = 0;
  chr_bank[0][0] = 0;
  chr_bank[0][1] = 0;
  chr_bank[1][0] = 0;
  chr_bank[1][1] = 0;
  mirror = 0;
  latch[0] = 0;
  latch[1] = 0;
}

void serialize(serializer &s) {
  Board::serialize(s);

  s.integer(prg_bank);
  s.integer(chr_bank[0][0]);
  s.integer(chr_bank[0][1]);
  s.integer(chr_bank[1][0]);
  s.integer(chr_bank[1][1]);
  s.integer(mirror);
  s.array(latch);
}

NES_PxROM(XML::Document &document, const stream &memory) : Board(document, memory) {
  revision = Revision::PNROM;
}

};
