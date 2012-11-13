//BANDAI-FCG

struct BandaiFCG : Board {

uint8 chr_bank[8];
uint8 prg_bank;
uint2 mirror;
bool irq_counter_enable;
uint16 irq_counter;
uint16 irq_latch;

void main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(irq_counter_enable) {
      if(--irq_counter == 0xffff) {
        cpu.set_irq_line(1);
        irq_counter_enable = false;
      }
    }

    tick();
  }
}

unsigned ciram_addr(unsigned addr) const {
  switch(mirror) {
  case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
  case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
  case 2: return 0x0000 | (addr & 0x03ff);
  case 3: return 0x0400 | (addr & 0x03ff);
  }
}

uint8 prg_read(unsigned addr) {
  if(addr & 0x8000) {
    bool region = addr & 0x4000;
    unsigned bank = (region == 0 ? prg_bank : 0x0f);
    return prgrom.read((bank << 14) | (addr & 0x3fff));
  }
  return cpu.mdr();
}

void prg_write(unsigned addr, uint8 data) {
  if(addr >= 0x6000) {
    switch(addr & 15) {
    case 0x00: case 0x01: case 0x02: case 0x03:
    case 0x04: case 0x05: case 0x06: case 0x07:
      chr_bank[addr & 7] = data;
      break;
    case 0x08:
      prg_bank = data & 0x0f;
      break;
    case 0x09:
      mirror = data & 0x03;
      break;
    case 0x0a:
      cpu.set_irq_line(0);
      irq_counter_enable = data & 0x01;
      irq_counter = irq_latch;
      break;
    case 0x0b:
      irq_latch = (irq_latch & 0xff00) | (data << 0);
      break;
    case 0x0c:
      irq_latch = (irq_latch & 0x00ff) | (data << 8);
      break;
    case 0x0d:
      //TODO: serial EEPROM support
      break;
    }
  }
}

uint8 chr_read(unsigned addr) {
  if(addr & 0x2000) return ppu.ciram_read(ciram_addr(addr));
  addr = (chr_bank[addr >> 10] << 10) | (addr & 0x03ff);
  return Board::chr_read(addr);
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) return ppu.ciram_write(ciram_addr(addr), data);
  addr = (chr_bank[addr >> 10] << 10) | (addr & 0x03ff);
  return Board::chr_write(addr, data);
}

void power() {
  reset();
}

void reset() {
  for(auto &n : chr_bank) n = 0;
  prg_bank = 0;
  mirror = 0;
  irq_counter_enable = 0;
  irq_counter = 0;
  irq_latch = 0;
}

void serialize(serializer &s) {
  Board::serialize(s);

  s.array(chr_bank);
  s.integer(prg_bank);
  s.integer(mirror);
  s.integer(irq_counter_enable);
  s.integer(irq_counter);
  s.integer(irq_latch);
}

BandaiFCG(XML::Document &document, const stream &memory) : Board(document, memory) {
}

};
