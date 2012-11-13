struct MMC1 : Chip {

enum class Revision : unsigned {
  MMC1,
  MMC1A,
  MMC1B1,
  MMC1B2,
  MMC1B3,
  MMC1C,
} revision;

unsigned writedelay;
unsigned shiftaddr;
unsigned shiftdata;

bool chr_mode;
bool prg_size;  //0 = 32K, 1 = 16K
bool prg_mode;
uint2 mirror;  //0 = first, 1 = second, 2 = vertical, 3 = horizontal
uint5 chr_bank[2];
bool ram_disable;
uint4 prg_bank;

void main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(writedelay) writedelay--;
    tick();
  }
}

unsigned prg_addr(unsigned addr) {
  bool region = addr & 0x4000;
  unsigned bank = (prg_bank & ~1) + region;

  if(prg_size) {
    bank = (region == 0 ? 0x0 : 0xf);
    if(region != prg_mode) bank = prg_bank;
  }

  return (bank << 14) | (addr & 0x3fff);
}

unsigned chr_addr(unsigned addr) {
  bool region = addr & 0x1000;
  unsigned bank = chr_bank[region];
  if(chr_mode == 0) bank = (chr_bank[0] & ~1) | region;
  return (bank << 12) | (addr & 0x0fff);
}

unsigned ciram_addr(unsigned addr) {
  switch(mirror) {
  case 0: return 0x0000 | (addr & 0x03ff);
  case 1: return 0x0400 | (addr & 0x03ff);
  case 2: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
  case 3: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
  }
}

void mmio_write(unsigned addr, uint8 data) {
  if(writedelay) return;
  writedelay = 2;

  if(data & 0x80) {
    shiftaddr = 0;
    prg_size = 1;
    prg_mode = 1;
  } else {
    shiftdata = ((data & 1) << 4) | (shiftdata >> 1);
    if(++shiftaddr == 5) {
      shiftaddr = 0;
      switch((addr >> 13) & 3) {
      case 0:
        chr_mode = (shiftdata & 0x10);
        prg_size = (shiftdata & 0x08);
        prg_mode = (shiftdata & 0x04);
        mirror = (shiftdata & 0x03);
        break;

      case 1:
        chr_bank[0] = (shiftdata & 0x1f);
        break;

      case 2:
        chr_bank[1] = (shiftdata & 0x1f);
        break;

      case 3:
        ram_disable = (shiftdata & 0x10);
        prg_bank = (shiftdata & 0x0f);
        break;
      }
    }
  }
}

void power() {
}

void reset() {
  writedelay = 0;
  shiftaddr = 0;
  shiftdata = 0;

  chr_mode = 0;
  prg_size = 1;
  prg_mode = 1;
  mirror = 0;
  chr_bank[0] = 0;
  chr_bank[1] = 1;
  ram_disable = 0;
  prg_bank = 0;
}

void serialize(serializer &s) {
  s.integer(writedelay);
  s.integer(shiftaddr);
  s.integer(shiftdata);

  s.integer(chr_mode);
  s.integer(prg_size);
  s.integer(prg_mode);
  s.integer(mirror);
  s.array(chr_bank);
  s.integer(ram_disable);
  s.integer(prg_bank);
}

MMC1(Board &board) : Chip(board) {
  revision = Revision::MMC1B2;
}

};
