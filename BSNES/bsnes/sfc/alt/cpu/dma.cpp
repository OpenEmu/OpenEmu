#ifdef CPU_CPP

bool CPU::dma_transfer_valid(uint8 bbus, unsigned abus) {
  //transfers from WRAM to WRAM are invalid; chip only has one address bus
  if(bbus == 0x80 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000)) return false;
  return true;
}

bool CPU::dma_addr_valid(unsigned abus) {
  //A-bus access to B-bus or S-CPU registers are invalid
  if((abus & 0x40ff00) == 0x2100) return false;  //$[00-3f|80-bf]:[2100-21ff]
  if((abus & 0x40fe00) == 0x4000) return false;  //$[00-3f|80-bf]:[4000-41ff]
  if((abus & 0x40ffe0) == 0x4200) return false;  //$[00-3f|80-bf]:[4200-421f]
  if((abus & 0x40ff80) == 0x4300) return false;  //$[00-3f|80-bf]:[4300-437f]
  return true;
}

uint8 CPU::dma_read(unsigned abus) {
  if(dma_addr_valid(abus) == false) return 0x00;
  return bus.read(abus);
}

void CPU::dma_write(bool valid, unsigned addr, uint8 data) {
  if(valid) bus.write(addr, data);
}

void CPU::dma_transfer(bool direction, uint8 bbus, unsigned abus) {
  if(direction == 0) {
    uint8 data = dma_read(abus);
    add_clocks(8);
    dma_write(dma_transfer_valid(bbus, abus), 0x2100 | bbus, data);
  } else {
    uint8 data = dma_transfer_valid(bbus, abus) ? bus.read(0x2100 | bbus) : 0x00;
    add_clocks(8);
    dma_write(dma_addr_valid(abus), abus, data);
  }
}

uint8 CPU::dma_bbus(unsigned i, unsigned index) {
  switch(channel[i].transfer_mode) { default:
    case 0: return (channel[i].dest_addr);                       //0
    case 1: return (channel[i].dest_addr + (index & 1));         //0,1
    case 2: return (channel[i].dest_addr);                       //0,0
    case 3: return (channel[i].dest_addr + ((index >> 1) & 1));  //0,0,1,1
    case 4: return (channel[i].dest_addr + (index & 3));         //0,1,2,3
    case 5: return (channel[i].dest_addr + (index & 1));         //0,1,0,1
    case 6: return (channel[i].dest_addr);                       //0,0     [2]
    case 7: return (channel[i].dest_addr + ((index >> 1) & 1));  //0,0,1,1 [3]
  }
}

unsigned CPU::dma_addr(unsigned i) {
  unsigned result = (channel[i].source_bank << 16) | (channel[i].source_addr);

  if(channel[i].fixed_transfer == false) {
    if(channel[i].reverse_transfer == false) {
      channel[i].source_addr++;
    } else {
      channel[i].source_addr--;
    }
  }

  return result;
}

unsigned CPU::hdma_addr(unsigned i) {
  return (channel[i].source_bank << 16) | (channel[i].hdma_addr++);
}

unsigned CPU::hdma_iaddr(unsigned i) {
  return (channel[i].indirect_bank << 16) | (channel[i].indirect_addr++);
}

void CPU::dma_run() {
  add_clocks(16);

  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].dma_enabled == false) continue;
    add_clocks(8);

    unsigned index = 0;
    do {
      dma_transfer(channel[i].direction, dma_bbus(i, index++), dma_addr(i));
    } while(channel[i].dma_enabled && --channel[i].transfer_size);

    channel[i].dma_enabled = false;
  }

  status.irq_lock = true;
}

bool CPU::hdma_active_after(unsigned i) {
  for(unsigned n = i + 1; i < 8; i++) {
    if(channel[i].hdma_enabled && !channel[i].hdma_completed) return true;
  }
  return false;
}

void CPU::hdma_update(unsigned i) {
  if((channel[i].line_counter & 0x7f) == 0) {
    channel[i].line_counter = dma_read(hdma_addr(i));
    channel[i].hdma_completed = (channel[i].line_counter == 0);
    channel[i].hdma_do_transfer = !channel[i].hdma_completed;
    add_clocks(8);

    if(channel[i].indirect) {
      channel[i].indirect_addr = dma_read(hdma_addr(i)) << 8;
      add_clocks(8);

    //emulating this glitch causes a slight slowdown; only enable if needed
    //if(!channel[i].hdma_completed || hdma_active_after(i)) {
        channel[i].indirect_addr >>= 8;
        channel[i].indirect_addr |= dma_read(hdma_addr(i)) << 8;
        add_clocks(8);
    //}
    }
  }
}

void CPU::hdma_run() {
  unsigned channels = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].hdma_enabled) channels++;
  }
  if(channels == 0) return;

  add_clocks(16);
  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].hdma_enabled == false || channel[i].hdma_completed == true) continue;
    channel[i].dma_enabled = false;

    if(channel[i].hdma_do_transfer) {
      static const unsigned transfer_length[] = { 1, 2, 2, 4, 4, 4, 2, 4 };
      unsigned length = transfer_length[channel[i].transfer_mode];
      for(unsigned index = 0; index < length; index++) {
        unsigned addr = channel[i].indirect == false ? hdma_addr(i) : hdma_iaddr(i);
        dma_transfer(channel[i].direction, dma_bbus(i, index), addr);
      }
    }
  }

  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].hdma_enabled == false || channel[i].hdma_completed == true) continue;

    channel[i].line_counter--;
    channel[i].hdma_do_transfer = channel[i].line_counter & 0x80;
    hdma_update(i);
  }

  status.irq_lock = true;
}

void CPU::hdma_init() {
  unsigned channels = 0;
  for(unsigned i = 0; i < 8; i++) {
    channel[i].hdma_completed = false;
    channel[i].hdma_do_transfer = false;
    if(channel[i].hdma_enabled) channels++;
  }
  if(channels == 0) return;

  add_clocks(16);
  for(unsigned i = 0; i < 8; i++) {
    if(!channel[i].hdma_enabled) continue;
    channel[i].dma_enabled = false;

    channel[i].hdma_addr = channel[i].source_addr;
    channel[i].line_counter = 0;
    hdma_update(i);
  }

  status.irq_lock = true;
}

void CPU::dma_reset() {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].dma_enabled = false;
    channel[i].hdma_enabled = false;

    channel[i].direction = 1;
    channel[i].indirect = true;
    channel[i].unused = true;
    channel[i].reverse_transfer = true;
    channel[i].fixed_transfer = true;
    channel[i].transfer_mode = 0x07;

    channel[i].dest_addr = 0xff;
    channel[i].source_addr = 0xffff;
    channel[i].source_bank = 0xff;

    channel[i].transfer_size = 0xffff;
    channel[i].indirect_addr = 0xffff;

    channel[i].indirect_bank = 0xff;
    channel[i].hdma_addr = 0xff;
    channel[i].line_counter = 0xff;
    channel[i].unknown = 0xff;

    channel[i].hdma_completed = false;
    channel[i].hdma_do_transfer = false;
  }
}

#endif
