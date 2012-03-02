#ifdef CPU_CPP

void CPU::dma_add_clocks(unsigned clocks) {
  status.dma_clocks += clocks;
  add_clocks(clocks);
}

//=============
//memory access
//=============

bool CPU::dma_transfer_valid(uint8 bbus, uint32 abus) {
  //transfers from WRAM to WRAM are invalid; chip only has one address bus
  if(bbus == 0x80 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000)) return false;
  return true;
}

bool CPU::dma_addr_valid(uint32 abus) {
  //A-bus access to B-bus or S-CPU registers are invalid
  if((abus & 0x40ff00) == 0x2100) return false;  //$[00-3f|80-bf]:[2100-21ff]
  if((abus & 0x40fe00) == 0x4000) return false;  //$[00-3f|80-bf]:[4000-41ff]
  if((abus & 0x40ffe0) == 0x4200) return false;  //$[00-3f|80-bf]:[4200-421f]
  if((abus & 0x40ff80) == 0x4300) return false;  //$[00-3f|80-bf]:[4300-437f]
  return true;
}

uint8 CPU::dma_read(uint32 abus) {
  if(dma_addr_valid(abus) == false) return 0x00;
  return bus.read(abus);
}

//simulate two-stage pipeline for DMA transfers; example:
//cycle 0: read N+0
//cycle 1: write N+0 & read N+1 (parallel; one on A-bus, one on B-bus)
//cycle 2: write N+1 & read N+2 (parallel)
//cycle 3: write N+2
void CPU::dma_write(bool valid, unsigned addr, uint8 data) {
  if(pipe.valid) bus.write(pipe.addr, pipe.data);
  pipe.valid = valid;
  pipe.addr = addr;
  pipe.data = data;
}

void CPU::dma_transfer(bool direction, uint8 bbus, uint32 abus) {
  if(direction == 0) {
    dma_add_clocks(4);
    regs.mdr = dma_read(abus);
    dma_add_clocks(4);
    dma_write(dma_transfer_valid(bbus, abus), 0x2100 | bbus, regs.mdr);
  } else {
    dma_add_clocks(4);
    regs.mdr = dma_transfer_valid(bbus, abus) ? bus.read(0x2100 | bbus) : 0x00;
    dma_add_clocks(4);
    dma_write(dma_addr_valid(abus), abus, regs.mdr);
  }
}

//===================
//address calculation
//===================

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

inline uint32 CPU::dma_addr(unsigned i) {
  uint32 r = (channel[i].source_bank << 16) | (channel[i].source_addr);

  if(channel[i].fixed_transfer == false) {
    if(channel[i].reverse_transfer == false) {
      channel[i].source_addr++;
    } else {
      channel[i].source_addr--;
    }
  }

  return r;
}

inline uint32 CPU::hdma_addr(unsigned i) {
  return (channel[i].source_bank << 16) | (channel[i].hdma_addr++);
}

inline uint32 CPU::hdma_iaddr(unsigned i) {
  return (channel[i].indirect_bank << 16) | (channel[i].indirect_addr++);
}

//==============
//channel status
//==============

uint8 CPU::dma_enabled_channels() {
  uint8 r = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].dma_enabled) r++;
  }
  return r;
}

inline bool CPU::hdma_active(unsigned i) {
  return (channel[i].hdma_enabled && !channel[i].hdma_completed);
}

inline bool CPU::hdma_active_after(unsigned i) {
  for(unsigned n = i + 1; n < 8; n++) {
    if(hdma_active(n) == true) return true;
  }
  return false;
}

inline uint8 CPU::hdma_enabled_channels() {
  uint8 r = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].hdma_enabled) r++;
  }
  return r;
}

inline uint8 CPU::hdma_active_channels() {
  uint8 r = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(hdma_active(i) == true) r++;
  }
  return r;
}

//==============
//core functions
//==============

void CPU::dma_run() {
  dma_add_clocks(8);
  dma_write(false);
  dma_edge();

  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].dma_enabled == false) continue;

    unsigned index = 0;
    do {
      dma_transfer(channel[i].direction, dma_bbus(i, index++), dma_addr(i));
      dma_edge();
    } while(channel[i].dma_enabled && --channel[i].transfer_size);

    dma_add_clocks(8);
    dma_write(false);
    dma_edge();

    channel[i].dma_enabled = false;
  }

  status.irq_lock = true;
}

void CPU::hdma_update(unsigned i) {
  dma_add_clocks(4);
  regs.mdr = dma_read((channel[i].source_bank << 16) | channel[i].hdma_addr);
  dma_add_clocks(4);
  dma_write(false);

  if((channel[i].line_counter & 0x7f) == 0) {
    channel[i].line_counter = regs.mdr;
    channel[i].hdma_addr++;

    channel[i].hdma_completed = (channel[i].line_counter == 0);
    channel[i].hdma_do_transfer = !channel[i].hdma_completed;

    if(channel[i].indirect) {
      dma_add_clocks(4);
      regs.mdr = dma_read(hdma_addr(i));
      channel[i].indirect_addr = regs.mdr << 8;
      dma_add_clocks(4);
      dma_write(false);

      if(!channel[i].hdma_completed || hdma_active_after(i)) {
        dma_add_clocks(4);
        regs.mdr = dma_read(hdma_addr(i));
        channel[i].indirect_addr >>= 8;
        channel[i].indirect_addr |= regs.mdr << 8;
        dma_add_clocks(4);
        dma_write(false);
      }
    }
  }
}

void CPU::hdma_run() {
  dma_add_clocks(8);
  dma_write(false);

  for(unsigned i = 0; i < 8; i++) {
    if(hdma_active(i) == false) continue;
    channel[i].dma_enabled = false;  //HDMA run during DMA will stop DMA mid-transfer

    if(channel[i].hdma_do_transfer) {
      static const unsigned transfer_length[8] = { 1, 2, 2, 4, 4, 4, 2, 4 };
      unsigned length = transfer_length[channel[i].transfer_mode];
      for(unsigned index = 0; index < length; index++) {
        unsigned addr = channel[i].indirect == false ? hdma_addr(i) : hdma_iaddr(i);
        dma_transfer(channel[i].direction, dma_bbus(i, index), addr);
      }
    }
  }

  for(unsigned i = 0; i < 8; i++) {
    if(hdma_active(i) == false) continue;

    channel[i].line_counter--;
    channel[i].hdma_do_transfer = channel[i].line_counter & 0x80;
    hdma_update(i);
  }

  status.irq_lock = true;
}

void CPU::hdma_init_reset() {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].hdma_completed = false;
    channel[i].hdma_do_transfer = false;
  }
}

void CPU::hdma_init() {
  dma_add_clocks(8);
  dma_write(false);

  for(unsigned i = 0; i < 8; i++) {
    if(!channel[i].hdma_enabled) continue;
    channel[i].dma_enabled = false;  //HDMA init during DMA will stop DMA mid-transfer

    channel[i].hdma_addr = channel[i].source_addr;
    channel[i].line_counter = 0;
    hdma_update(i);
  }

  status.irq_lock = true;
}

//==============
//initialization
//==============

void CPU::dma_power() {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].direction = 1;
    channel[i].indirect = true;
    channel[i].unused = true;
    channel[i].reverse_transfer = true;
    channel[i].fixed_transfer = true;
    channel[i].transfer_mode = 7;

    channel[i].dest_addr = 0xff;

    channel[i].source_addr = 0xffff;
    channel[i].source_bank = 0xff;

    channel[i].transfer_size = 0xffff;
    channel[i].indirect_bank = 0xff;

    channel[i].hdma_addr = 0xffff;
    channel[i].line_counter = 0xff;
    channel[i].unknown = 0xff;
  }
}

void CPU::dma_reset() {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].dma_enabled = false;
    channel[i].hdma_enabled = false;

    channel[i].hdma_completed = false;
    channel[i].hdma_do_transfer = false;
  }

  pipe.valid = false;
  pipe.addr = 0;
  pipe.data = 0;
}

#endif
