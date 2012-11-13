struct VRC4 : Chip {

bool prg_mode;
uint5 prg_bank[2];
uint2 mirror;
uint8 chr_bank[8];

uint8 irq_latch;
bool irq_mode;
bool irq_enable;
bool irq_acknowledge;

uint8 irq_counter;
signed irq_scalar;
bool irq_line;

void main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(irq_enable) {
      if(irq_mode == 0) {
        irq_scalar -= 3;
        if(irq_scalar <= 0) {
          irq_scalar += 341;
          if(irq_counter == 0xff) {
            irq_counter = irq_latch;
            irq_line = 1;
          } else {
            irq_counter++;
          }
        }
      }

      if(irq_mode == 1) {
        if(irq_counter == 0xff) {
          irq_counter = irq_latch;
          irq_line = 1;
        } else {
          irq_counter++;
        }
      }
    }

    cpu.set_irq_line(irq_line);
    tick();
  }
}

unsigned prg_addr(unsigned addr) const {
  unsigned bank = 0, banks = board.prgrom.size / 0x2000;
  switch(addr & 0xe000) {
  case 0x8000: bank = prg_mode == 0 ? (unsigned)prg_bank[0] : banks - 2; break;
  case 0xa000: bank = prg_bank[1]; break;
  case 0xc000: bank = prg_mode == 0 ? banks - 2 : (unsigned)prg_bank[0]; break;
  case 0xe000: bank = banks - 1; break;
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

void reg_write(unsigned addr, uint8 data) {
  switch(addr) {
  case 0x8000: case 0x8001: case 0x8002: case 0x8003:
    prg_bank[0] = data & 0x1f;
    break;

  case 0x9000: case 0x9001:
    mirror = data & 0x03;
    break;

  case 0x9002: case 0x9003:
    prg_mode = data & 0x02;
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

  case 0xf000:
    irq_latch = (irq_latch & 0xf0) | ((data & 0x0f) << 0);
    break;

  case 0xf001:
    irq_latch = (irq_latch & 0x0f) | ((data & 0x0f) << 4);
    break;

  case 0xf002:
    irq_mode = data & 0x04;
    irq_enable = data & 0x02;
    irq_acknowledge = data & 0x01;
    if(irq_enable) {
      irq_counter = irq_latch;
      irq_scalar = 341;
    }
    irq_line = 0;
    break;

  case 0xf003:
    irq_enable = irq_acknowledge;
    irq_line = 0;
    break;
  }
}

void power() {
}

void reset() {
  prg_mode = 0;
  for(auto &n : prg_bank) n = 0;
  mirror = 0;
  for(auto &n : chr_bank) n = 0;

  irq_latch = 0;
  irq_mode = 0;
  irq_enable = 0;
  irq_acknowledge = 0;

  irq_counter = 0;
  irq_scalar = 0;
  irq_line = 0;
}

void serialize(serializer &s) {
  s.integer(prg_mode);
  for(auto &n : prg_bank) s.integer(n);
  s.integer(mirror);
  for(auto &n : chr_bank) s.integer(n);

  s.integer(irq_latch);
  s.integer(irq_mode);
  s.integer(irq_enable);
  s.integer(irq_acknowledge);

  s.integer(irq_counter);
  s.integer(irq_scalar);
  s.integer(irq_line);
}

VRC4(Board &board) : Chip(board) {
}

};
