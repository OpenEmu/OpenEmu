//SUNSOFT-5B

struct Sunsoft5B : Board {

uint4 mmu_port;
uint4 apu_port;

uint8 prg_bank[4];
uint8 chr_bank[8];
uint2 mirror;
bool irq_enable;
bool irq_counter_enable;
uint16 irq_counter;

int16 dac[16];

struct Pulse {
  bool disable;
  uint12 frequency;
  uint4 volume;

  uint16 counter;  //12-bit countdown + 4-bit phase
  uint1 duty;
  uint4 output;

  void clock() {
    if(--counter == 0) {
      counter = frequency << 4;
      duty ^= 1;
    }
    output = duty ? volume : (uint4)0;
    if(disable) output = 0;
  }

  void reset() {
    disable = 1;
    frequency = 1;
    volume = 0;

    counter = 0;
    duty = 0;
    output = 0;
  }

  void serialize(serializer &s) {
    s.integer(disable);
    s.integer(frequency);
    s.integer(volume);

    s.integer(counter);
    s.integer(duty);
    s.integer(output);
  }
} pulse[3];

void main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(irq_counter_enable) {
      if(--irq_counter == 0xffff) {
        cpu.set_irq_line(irq_enable);
      }
    }

    pulse[0].clock();
    pulse[1].clock();
    pulse[2].clock();
    int16 output = dac[pulse[0].output] + dac[pulse[1].output] + dac[pulse[2].output];
    apu.set_sample(-output);

    tick();
  }
}

uint8 prg_read(unsigned addr) {
  if(addr < 0x6000) return cpu.mdr();

  uint8 bank = 0x3f;  //((addr & 0xe000) == 0xe000
  if((addr & 0xe000) == 0x6000) bank = prg_bank[0];
  if((addr & 0xe000) == 0x8000) bank = prg_bank[1];
  if((addr & 0xe000) == 0xa000) bank = prg_bank[2];
  if((addr & 0xe000) == 0xc000) bank = prg_bank[3];

  bool ram_enable = bank & 0x80;
  bool ram_select = bank & 0x40;
  bank &= 0x3f;

  if(ram_select) {
    if(ram_enable == false) return cpu.mdr();
    return prgram.data[addr & 0x1fff];
  }

  addr = (bank << 13) | (addr & 0x1fff);
  return prgrom.read(addr);
}

void prg_write(unsigned addr, uint8 data) {
  if((addr & 0xe000) == 0x6000) {
    prgram.data[addr & 0x1fff] = data;
  }

  if(addr == 0x8000) {
    mmu_port = data & 0x0f;
  }

  if(addr == 0xa000) {
    switch(mmu_port) {
    case  0: chr_bank[0] = data; break;
    case  1: chr_bank[1] = data; break;
    case  2: chr_bank[2] = data; break;
    case  3: chr_bank[3] = data; break;
    case  4: chr_bank[4] = data; break;
    case  5: chr_bank[5] = data; break;
    case  6: chr_bank[6] = data; break;
    case  7: chr_bank[7] = data; break;
    case  8: prg_bank[0] = data; break;
    case  9: prg_bank[1] = data; break;
    case 10: prg_bank[2] = data; break;
    case 11: prg_bank[3] = data; break;
    case 12: mirror = data & 3; break;
    case 13:
      irq_enable = data & 0x80;
      irq_counter_enable = data & 0x01;
      if(irq_enable == 0) cpu.set_irq_line(0);
      break;
    case 14: irq_counter = (irq_counter & 0xff00) | (data << 0); break;
    case 15: irq_counter = (irq_counter & 0x00ff) | (data << 8); break;
    }
  }

  if(addr == 0xc000) {
    apu_port = data & 0x0f;
  }

  if(addr == 0xe000) {
    switch(apu_port) {
    case  0: pulse[0].frequency = (pulse[0].frequency & 0xff00) | (data << 0); break;
    case  1: pulse[0].frequency = (pulse[0].frequency & 0x00ff) | (data << 8); break;
    case  2: pulse[1].frequency = (pulse[1].frequency & 0xff00) | (data << 0); break;
    case  3: pulse[1].frequency = (pulse[1].frequency & 0x00ff) | (data << 8); break;
    case  4: pulse[2].frequency = (pulse[2].frequency & 0xff00) | (data << 0); break;
    case  5: pulse[2].frequency = (pulse[2].frequency & 0x00ff) | (data << 8); break;
    case  7:
      pulse[0].disable = data & 0x01;
      pulse[1].disable = data & 0x02;
      pulse[2].disable = data & 0x04;
      break;
    case  8: pulse[0].volume = data & 0x0f; break;
    case  9: pulse[1].volume = data & 0x0f; break;
    case 10: pulse[2].volume = data & 0x0f; break;
    }
  }
}

unsigned chr_addr(unsigned addr) {
  uint8 bank = (addr >> 10) & 7;
  return (chr_bank[bank] << 10) | (addr & 0x03ff);
}

unsigned ciram_addr(unsigned addr) {
  switch(mirror) {
  case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);  //vertical
  case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);  //horizontal
  case 2: return 0x0000 | (addr & 0x03ff);  //first
  case 3: return 0x0400 | (addr & 0x03ff);  //second
  }
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(ciram_addr(addr));
  return Board::chr_read(chr_addr(addr));
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(ciram_addr(addr), data);
  return Board::chr_write(chr_addr(addr), data);
}

void power() {
  for(signed n = 0; n < 16; n++) {
    double volume = 1.0 / pow(2, 1.0 / 2 * (15 - n));
    dac[n] = volume * 8192.0;
  }
}

void reset() {
  mmu_port = 0;
  apu_port = 0;

  for(auto &n : prg_bank) n = 0;
  for(auto &n : chr_bank) n = 0;
  mirror = 0;
  irq_enable = 0;
  irq_counter_enable = 0;
  irq_counter = 0;

  pulse[0].reset();
  pulse[1].reset();
  pulse[2].reset();
}

void serialize(serializer &s) {
  Board::serialize(s);

  s.integer(mmu_port);
  s.integer(apu_port);

  s.array(prg_bank);
  s.array(chr_bank);
  s.integer(mirror);
  s.integer(irq_enable);
  s.integer(irq_counter_enable);
  s.integer(irq_counter);

  pulse[0].serialize(s);
  pulse[1].serialize(s);
  pulse[2].serialize(s);
}

Sunsoft5B(XML::Document &document, const stream &memory) : Board(document, memory) {
}

};
