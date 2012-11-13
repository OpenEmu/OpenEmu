struct VRC1 : Chip {

uint4 prg_bank[3];
uint4 chr_banklo[2];
bool chr_bankhi[2];
bool mirror;

unsigned prg_addr(unsigned addr) const {
  unsigned bank = 0x0f;
  if((addr & 0xe000) == 0x8000) bank = prg_bank[0];
  if((addr & 0xe000) == 0xa000) bank = prg_bank[1];
  if((addr & 0xe000) == 0xc000) bank = prg_bank[2];
  return (bank * 0x2000) + (addr & 0x1fff);
}

unsigned chr_addr(unsigned addr) const {
  unsigned bank = chr_banklo[(bool)(addr & 0x1000)];
  bank |= chr_bankhi[(bool)(addr & 0x1000)] << 4;
  return (bank * 0x1000) + (addr & 0x0fff);  
}

unsigned ciram_addr(unsigned addr) const {
  switch(mirror) {
  case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);  //vertical mirroring
  case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);  //horizontal mirroring
  }
  throw;
}

void reg_write(unsigned addr, uint8 data) {
  switch(addr & 0xf000) {
  case 0x8000:
    prg_bank[0] = data & 0x0f;
    break;

  case 0x9000:
    chr_bankhi[1] = data & 0x04;
    chr_bankhi[0] = data & 0x02;
    mirror = data & 0x01;
    break;

  case 0xa000:
    prg_bank[1] = data & 0x0f;
    break;

  case 0xc000:
    prg_bank[2] = data & 0x0f;
    break;

  case 0xe000:
    chr_banklo[0] = data & 0x0f;
    break;

  case 0xf000:
    chr_banklo[1] = data & 0x0f;
    break;
  }
}

void power() {
}

void reset() {
  for(auto &n : prg_bank) n = 0;
  for(auto &n : chr_banklo) n = 0;
  for(auto &n : chr_bankhi) n = 0;
  mirror = 0;
}

void serialize(serializer &s) {
  for(auto &n : prg_bank) s.integer(n);
  for(auto &n : chr_banklo) s.integer(n);
  for(auto &n : chr_bankhi) s.integer(n);
  s.integer(mirror);
}

VRC1(Board &board) : Chip(board) {
}

};
