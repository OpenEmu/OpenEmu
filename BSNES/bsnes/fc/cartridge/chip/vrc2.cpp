struct VRC2 : Chip {

uint5 prg_bank[2];
uint8 chr_bank[8];
uint2 mirror;
bool latch;

unsigned prg_addr(unsigned addr) const {
  unsigned bank;
  switch(addr & 0xe000) {
  case 0x8000: bank = prg_bank[0]; break;
  case 0xa000: bank = prg_bank[1]; break;
  case 0xc000: bank = 0x1e; break;
  case 0xe000: bank = 0x1f; break;
  }
  return (bank * 0x2000) + (addr & 0x1fff);
}

unsigned chr_addr(unsigned addr) const {
  unsigned bank = chr_bank[addr / 0x0400];
  return (bank * 0x0400) + (addr & 0x03ff);
}

unsigned ciram_addr(unsigned addr) const {
  switch(mirror) {
  case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);  //vertical mirroring
  case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);  //horizontal mirroring
  case 2: return 0x0000 | (addr & 0x03ff);                  //one-screen mirroring (first)
  case 3: return 0x0400 | (addr & 0x03ff);                  //one-screen mirroring (second)
  }
  throw;
}

uint8 ram_read(unsigned addr) {
  if(board.prgram.size == 0) {
    if((addr & 0xf000) == 0x6000) return cpu.mdr() | latch;
    return cpu.mdr();
  }
  return board.prgram.read(addr & 0x1fff);
}

void ram_write(unsigned addr, uint8 data) {
  if(board.prgram.size == 0) {
    if((addr & 0xf000) == 0x6000) latch = data & 0x01;
    return;
  }
  return board.prgram.write(addr & 0x1fff, data);
}

void reg_write(unsigned addr, uint8 data) {
  switch(addr) {
  case 0x8000: case 0x8001: case 0x8002: case 0x8003:
    prg_bank[0] = data & 0x1f;
    break;

  case 0x9000: case 0x9001: case 0x9002: case 0x9003:
    mirror = data & 0x03;
    break;

  case 0xa000: case 0xa001: case 0xa002: case 0xa003:
    prg_bank[1] = data & 0x1f;
    break;

  case 0xb000: chr_bank[0] = (chr_bank[0] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xb001: chr_bank[0] = (chr_bank[0] & 0x0f) | ((data & 0x0f) << 4); break;

  case 0xb002: chr_bank[1] = (chr_bank[1] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xb003: chr_bank[1] = (chr_bank[1] & 0x0f) | ((data & 0x0f) << 4); break;

  case 0xc000: chr_bank[2] = (chr_bank[2] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xc001: chr_bank[2] = (chr_bank[2] & 0x0f) | ((data & 0x0f) << 4); break;

  case 0xc002: chr_bank[3] = (chr_bank[3] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xc003: chr_bank[3] = (chr_bank[3] & 0x0f) | ((data & 0x0f) << 4); break;

  case 0xd000: chr_bank[4] = (chr_bank[4] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xd001: chr_bank[4] = (chr_bank[4] & 0x0f) | ((data & 0x0f) << 4); break;

  case 0xd002: chr_bank[5] = (chr_bank[5] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xd003: chr_bank[5] = (chr_bank[5] & 0x0f) | ((data & 0x0f) << 4); break;

  case 0xe000: chr_bank[6] = (chr_bank[6] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xe001: chr_bank[6] = (chr_bank[6] & 0x0f) | ((data & 0x0f) << 4); break;

  case 0xe002: chr_bank[7] = (chr_bank[7] & 0xf0) | ((data & 0x0f) << 0); break;
  case 0xe003: chr_bank[7] = (chr_bank[7] & 0x0f) | ((data & 0x0f) << 4); break;
  }
}

void power() {
}

void reset() {
  for(auto &n : prg_bank) n = 0;
  for(auto &n : chr_bank) n = 0;
  mirror = 0;
  latch = 0;
}

void serialize(serializer &s) {
  for(auto &n : prg_bank) s.integer(n);
  for(auto &n : chr_bank) s.integer(n);
  s.integer(mirror);
  s.integer(latch);
}

VRC2(Board &board) : Chip(board) {
}

};
