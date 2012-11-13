//Konami VRC7
//Yamaha YM2413 OPLL audio - not emulated

struct VRC7 : Chip {

uint8 prg_bank[3];
uint8 chr_bank[8];
uint2 mirror;

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

void reg_write(unsigned addr, uint8 data) {
  switch(addr) {
  case 0x8000: prg_bank[0] = data; break;
  case 0x8010: prg_bank[1] = data; break;
  case 0x9000: prg_bank[2] = data; break;
  case 0x9010: break;  //APU addr port
  case 0x9030: break;  //APU data port
  case 0xa000: chr_bank[0] = data; break;
  case 0xa010: chr_bank[1] = data; break;
  case 0xb000: chr_bank[2] = data; break;
  case 0xb010: chr_bank[3] = data; break;
  case 0xc000: chr_bank[4] = data; break;
  case 0xc010: chr_bank[5] = data; break;
  case 0xd000: chr_bank[6] = data; break;
  case 0xd010: chr_bank[7] = data; break;
  case 0xe000: mirror = data & 0x03; break;

  case 0xe010:
    irq_latch = data;
    break;

  case 0xf000:
    irq_mode = data & 0x04;
    irq_enable = data & 0x02;
    irq_acknowledge = data & 0x01;
    if(irq_enable) {
      irq_counter = irq_latch;
      irq_scalar = 341;
    }
    irq_line = 0;
    break;

  case 0xf010:
    irq_enable = irq_acknowledge;
    irq_line = 0;
    break;
  }
}

unsigned prg_addr(unsigned addr) const {
  unsigned bank = 0;
  switch(addr & 0xe000) {
  case 0x8000: bank = prg_bank[0]; break;
  case 0xa000: bank = prg_bank[1]; break;
  case 0xc000: bank = prg_bank[2]; break;
  case 0xe000: bank = 0xff; break;
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
}

void power() {
}

void reset() {
  for(auto &n : prg_bank) n = 0;
  for(auto &n : chr_bank) n = 0;
  mirror = 0;

  irq_latch = 0;
  irq_mode = 0;
  irq_enable = 0;
  irq_acknowledge = 0;

  irq_counter = 0;
  irq_scalar = 0;
  irq_line = 0;
}

void serialize(serializer &s) {
  s.array(prg_bank);
  s.array(chr_bank);
  s.integer(mirror);

  s.integer(irq_latch);
  s.integer(irq_mode);
  s.integer(irq_enable);
  s.integer(irq_acknowledge);

  s.integer(irq_counter);
  s.integer(irq_scalar);
  s.integer(irq_line);
}

VRC7(Board &board) : Chip(board) {
}

};
