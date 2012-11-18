struct MMC6 : Chip {

bool chr_mode;
bool prg_mode;
bool ram_enable;
uint3 bank_select;
uint8 prg_bank[2];
uint8 chr_bank[6];
bool mirror;
bool ram_readable[2];
bool ram_writable[2];
uint8 irq_latch;
uint8 irq_counter;
bool irq_enable;
unsigned irq_delay;
bool irq_line;

uint16 chr_abus;

void main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(irq_delay) irq_delay--;
    cpu.set_irq_line(irq_line);
    tick();
  }
}

void irq_test(unsigned addr) {
  if(!(chr_abus & 0x1000) && (addr & 0x1000)) {
    if(irq_delay == 0) {
      if(irq_counter == 0) {
        irq_counter = irq_latch;
      } else if(--irq_counter == 0) {
        if(irq_enable) irq_line = 1;
      }
    }
    irq_delay = 6;
  }
  chr_abus = addr;
}

unsigned prg_addr(unsigned addr) const {
  switch((addr >> 13) & 3) {
  case 0:
    if(prg_mode == 1) return (0x3e << 13) | (addr & 0x1fff);
    return (prg_bank[0] << 13) | (addr & 0x1fff);
  case 1:
    return (prg_bank[1] << 13) | (addr & 0x1fff);
  case 2:
    if(prg_mode == 0) return (0x3e << 13) | (addr & 0x1fff);
    return (prg_bank[0] << 13) | (addr & 0x1fff);
  case 3:
    return (0x3f << 13) | (addr & 0x1fff);
  }
}

unsigned chr_addr(unsigned addr) const {
  if(chr_mode == 0) {
    if(addr <= 0x07ff) return (chr_bank[0] << 10) | (addr & 0x07ff);
    if(addr <= 0x0fff) return (chr_bank[1] << 10) | (addr & 0x07ff);
    if(addr <= 0x13ff) return (chr_bank[2] << 10) | (addr & 0x03ff);
    if(addr <= 0x17ff) return (chr_bank[3] << 10) | (addr & 0x03ff);
    if(addr <= 0x1bff) return (chr_bank[4] << 10) | (addr & 0x03ff);
    if(addr <= 0x1fff) return (chr_bank[5] << 10) | (addr & 0x03ff);
  } else {
    if(addr <= 0x03ff) return (chr_bank[2] << 10) | (addr & 0x03ff);
    if(addr <= 0x07ff) return (chr_bank[3] << 10) | (addr & 0x03ff);
    if(addr <= 0x0bff) return (chr_bank[4] << 10) | (addr & 0x03ff);
    if(addr <= 0x0fff) return (chr_bank[5] << 10) | (addr & 0x03ff);
    if(addr <= 0x17ff) return (chr_bank[0] << 10) | (addr & 0x07ff);
    if(addr <= 0x1fff) return (chr_bank[1] << 10) | (addr & 0x07ff);
  }
}

unsigned ciram_addr(unsigned addr) const {
  if(mirror == 0) return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
  if(mirror == 1) return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
}

uint8 ram_read(unsigned addr) {
  if(ram_enable == false) return cpu.mdr();
  if(ram_readable[0] == false && ram_readable[1] == false) return cpu.mdr();
  bool region = addr & 0x0200;
  if(ram_readable[region] == false) return 0x00;
  return board.prgram.read((region * 0x0200) + (addr & 0x01ff));
}

void ram_write(unsigned addr, uint8 data) {
  if(ram_enable == false) return;
  bool region = addr & 0x0200;
  if(ram_writable[region] == false) return;
  return board.prgram.write((region * 0x0200) + (addr & 0x01ff), data);
}

void reg_write(unsigned addr, uint8 data) {
  switch(addr & 0xe001) {
  case 0x8000:
    chr_mode = data & 0x80;
    prg_mode = data & 0x40;
    ram_enable = data & 0x20;
    bank_select = data & 0x07;
    if(ram_enable == false) {
      for(auto &n : ram_readable) n = false;
      for(auto &n : ram_writable) n = false;
    }
    break;

  case 0x8001:
    switch(bank_select) {
    case 0: chr_bank[0] = data & ~1; break;
    case 1: chr_bank[1] = data & ~1; break;
    case 2: chr_bank[2] = data; break;
    case 3: chr_bank[3] = data; break;
    case 4: chr_bank[4] = data; break;
    case 5: chr_bank[5] = data; break;
    case 6: prg_bank[0] = data & 0x3f; break;
    case 7: prg_bank[1] = data & 0x3f; break;
    }
    break;

  case 0xa000:
    mirror = data & 0x01;
    break;

  case 0xa001:
    if(ram_enable == false) break;
    ram_readable[1] = data & 0x80;
    ram_writable[1] = data & 0x40;
    ram_readable[0] = data & 0x20;
    ram_writable[0] = data & 0x10;
    break;

  case 0xc000:
    irq_latch = data;
    break;

  case 0xc001:
    irq_counter = 0;
    break;

  case 0xe000:
    irq_enable = false;
    irq_line = 0;
    break;

  case 0xe001:
    irq_enable = true;
    break;
  }
}

void power() {
}

void reset() {
  chr_mode = 0;
  prg_mode = 0;
  ram_enable = 0;
  bank_select = 0;
  for(auto &n : prg_bank) n = 0;
  for(auto &n : chr_bank) n = 0;
  mirror = 0;
  for(auto &n : ram_readable) n = 0;
  for(auto &n : ram_writable) n = 0;
  irq_latch = 0;
  irq_counter = 0;
  irq_enable = 0;
  irq_delay = 0;
  irq_line = 0;

  chr_abus = 0;
}

void serialize(serializer &s) {
  s.integer(chr_mode);
  s.integer(prg_mode);
  s.integer(ram_enable);
  s.integer(bank_select);
  for(auto &n : prg_bank) s.integer(n);
  for(auto &n : chr_bank) s.integer(n);
  s.integer(mirror);
  for(auto &n : ram_readable) s.integer(n);
  for(auto &n : ram_writable) s.integer(n);
  s.integer(irq_latch);
  s.integer(irq_counter);
  s.integer(irq_enable);
  s.integer(irq_delay);
  s.integer(irq_line);

  s.integer(chr_abus);
}

MMC6(Board &board) : Chip(board) {
}

};
