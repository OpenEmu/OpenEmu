struct VRC3 : Chip {

uint4 prg_bank;
bool irq_mode;
bool irq_enable;
bool irq_acknowledge;
uint16 irq_latch;
struct {
  union {
    uint16 w;
    struct { uint8 order_lsb2(l, h); };
  };
} irq_counter;
bool irq_line;

void main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(irq_enable) {
      if(irq_mode == 0) {  //16-bit
        if(++irq_counter.w == 0) {
          irq_line = 1;
          irq_enable = irq_acknowledge;
          irq_counter.w = irq_latch;
        }
      }
      if(irq_mode == 1) {  //8-bit
        if(++irq_counter.l == 0) {
          irq_line = 1;
          irq_enable = irq_acknowledge;
          irq_counter.l = irq_latch;
        }
      }
    }

    cpu.set_irq_line(irq_line);
    tick();
  }
}

unsigned prg_addr(unsigned addr) const {
  unsigned bank = (addr < 0xc000 ? (unsigned)prg_bank : 0x0f);
  return (bank * 0x4000) + (addr & 0x3fff);
}

void reg_write(unsigned addr, uint8 data) {
  switch(addr & 0xf000) {
  case 0x8000: irq_latch = (irq_latch & 0xfff0) | ((data & 0x0f) <<  0); break;
  case 0x9000: irq_latch = (irq_latch & 0xff0f) | ((data & 0x0f) <<  4); break;
  case 0xa000: irq_latch = (irq_latch & 0xf0ff) | ((data & 0x0f) <<  8); break;
  case 0xb000: irq_latch = (irq_latch & 0x0fff) | ((data & 0x0f) << 12); break;

  case 0xc000:
    irq_mode = data & 0x04;
    irq_enable = data & 0x02;
    irq_acknowledge = data & 0x01;
    if(irq_enable) irq_counter.w = irq_latch;
    break;

  case 0xd000:
    irq_line = 0;
    irq_enable = irq_acknowledge;
    break;

  case 0xf000:
    prg_bank = data & 0x0f;
    break;
  }
}

void power() {
}

void reset() {
  prg_bank = 0;
  irq_mode = 0;
  irq_enable = 0;
  irq_acknowledge = 0;
  irq_latch = 0;
  irq_counter.w = 0;
  irq_line = 0;
}

void serialize(serializer &s) {
  s.integer(prg_bank);
  s.integer(irq_mode);
  s.integer(irq_enable);
  s.integer(irq_acknowledge);
  s.integer(irq_latch);
  s.integer(irq_counter.w);
  s.integer(irq_line);
}

VRC3(Board &board) : Chip(board) {
}

};
