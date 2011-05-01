#ifdef SCPU_CPP

void sCPU::dma_add_clocks(unsigned clocks) {
  status.dma_clocks += clocks;
  add_clocks(clocks);
  scheduler.sync_cpucop();
  scheduler.sync_cpuppu();
}

bool sCPU::dma_addr_valid(uint32 abus) {
  //reads from B-bus or S-CPU registers are invalid
  if((abus & 0x40ff00) == 0x2100) return false;  //$[00-3f|80-bf]:[2100-21ff]
  if((abus & 0x40fe00) == 0x4000) return false;  //$[00-3f|80-bf]:[4000-41ff]
  if((abus & 0x40ffe0) == 0x4200) return false;  //$[00-3f|80-bf]:[4200-421f]
  if((abus & 0x40ff80) == 0x4300) return false;  //$[00-3f|80-bf]:[4300-437f]
  return true;
}

uint8 sCPU::dma_read(uint32 abus) {
  if(dma_addr_valid(abus) == false) return 0x00;  //does not return S-CPU MDR
  return bus.read(abus);
}

void sCPU::dma_transfer(bool direction, uint8 bbus, uint32 abus) {
  if(direction == 0) {
    //a->b transfer (to $21xx)
    if(bbus == 0x80 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000)) {
      //illegal WRAM->WRAM transfer (bus conflict)
      //read most likely occurs; no write occurs
      //read is irrelevent, as it cannot be observed by software
      dma_add_clocks(8);
    } else {
      dma_add_clocks(4);
      uint8 data = dma_read(abus);
      dma_add_clocks(4);
      bus.write(0x2100 | bbus, data);
    }
  } else {
    //b->a transfer (from $21xx)
    if(bbus == 0x80 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000)) {
      //illegal WRAM->WRAM transfer (bus conflict)
      //no read occurs; write does occur
      dma_add_clocks(8);
      bus.write(abus, 0x00);  //does not write S-CPU MDR
    } else {
      dma_add_clocks(4);
      uint8 data = bus.read(0x2100 | bbus);
      dma_add_clocks(4);
      if(dma_addr_valid(abus) == true) {
        bus.write(abus, data);
      }
    }
  }
}

/*****
 * address calculation functions
 *****/

uint8 sCPU::dma_bbus(uint8 i, uint8 index) {
  switch(channel[i].xfermode) { default:
    case 0: return (channel[i].destaddr);                       //0
    case 1: return (channel[i].destaddr + (index & 1));         //0,1
    case 2: return (channel[i].destaddr);                       //0,0
    case 3: return (channel[i].destaddr + ((index >> 1) & 1));  //0,0,1,1
    case 4: return (channel[i].destaddr + (index & 3));         //0,1,2,3
    case 5: return (channel[i].destaddr + (index & 1));         //0,1,0,1
    case 6: return (channel[i].destaddr);                       //0,0     [2]
    case 7: return (channel[i].destaddr + ((index >> 1) & 1));  //0,0,1,1 [3]
  }
}

inline uint32 sCPU::dma_addr(uint8 i) {
  uint32 r = (channel[i].srcbank << 16) | (channel[i].srcaddr);

  if(channel[i].fixedxfer == false) {
    if(channel[i].reversexfer == false) {
      channel[i].srcaddr++;
    } else {
      channel[i].srcaddr--;
    }
  }

  return r;
}

inline uint32 sCPU::hdma_addr(uint8 i) {
  return (channel[i].srcbank << 16) | (channel[i].hdma_addr++);
}

inline uint32 sCPU::hdma_iaddr(uint8 i) {
  return (channel[i].hdma_ibank << 16) | (channel[i].hdma_iaddr++);
}

/*****
 * DMA functions
 *****/

uint8 sCPU::dma_enabled_channels() {
  uint8 r = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].dma_enabled) r++;
  }
  return r;
}

void sCPU::dma_run() {
  dma_add_clocks(8);
  cycle_edge();

  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].dma_enabled == false) continue;
    dma_add_clocks(8);
    cycle_edge();

    unsigned index = 0;
    do {
      dma_transfer(channel[i].direction, dma_bbus(i, index++), dma_addr(i));
      cycle_edge();
    } while(channel[i].dma_enabled && --channel[i].xfersize);

    channel[i].dma_enabled = false;
  }

  status.irq_lock = true;
  event.enqueue(2, EventIrqLockRelease);
}

/*****
 * HDMA functions
 *****/

inline bool sCPU::hdma_active(uint8 i) {
  return (channel[i].hdma_enabled && !channel[i].hdma_completed);
}

inline bool sCPU::hdma_active_after(uint8 i) {
  for(unsigned n = i + 1; n < 8; n++) {
    if(hdma_active(n) == true) return true;
  }
  return false;
}

inline uint8 sCPU::hdma_enabled_channels() {
  uint8 r = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(channel[i].hdma_enabled) r++;
  }
  return r;
}

inline uint8 sCPU::hdma_active_channels() {
  uint8 r = 0;
  for(unsigned i = 0; i < 8; i++) {
    if(hdma_active(i) == true) r++;
  }
  return r;
}

void sCPU::hdma_update(uint8 i) {
  channel[i].hdma_line_counter = dma_read(hdma_addr(i));
  dma_add_clocks(8);

  channel[i].hdma_completed   = (channel[i].hdma_line_counter == 0);
  channel[i].hdma_do_transfer = !channel[i].hdma_completed;

  if(channel[i].hdma_indirect) {
    channel[i].hdma_iaddr = dma_read(hdma_addr(i)) << 8;
    dma_add_clocks(8);

    if(!channel[i].hdma_completed || hdma_active_after(i)) {
      channel[i].hdma_iaddr >>= 8;
      channel[i].hdma_iaddr  |= dma_read(hdma_addr(i)) << 8;
      dma_add_clocks(8);
    }
  }
}

void sCPU::hdma_run() {
  dma_add_clocks(8);

  for(unsigned i = 0; i < 8; i++) {
    if(hdma_active(i) == false) continue;
    channel[i].dma_enabled = false;  //HDMA run during DMA will stop DMA mid-transfer

    if(channel[i].hdma_do_transfer) {
      static const unsigned transfer_length[8] = { 1, 2, 2, 4, 4, 4, 2, 4 };
      unsigned length = transfer_length[channel[i].xfermode];
      for(unsigned index = 0; index < length; index++) {
        unsigned addr = !channel[i].hdma_indirect ? hdma_addr(i) : hdma_iaddr(i);
        dma_transfer(channel[i].direction, dma_bbus(i, index), addr);
      }
    }
  }

  for(unsigned i = 0; i < 8; i++) {
    if(hdma_active(i) == false) continue;

    channel[i].hdma_line_counter--;
    channel[i].hdma_do_transfer = bool(channel[i].hdma_line_counter & 0x80);
    if((channel[i].hdma_line_counter & 0x7f) == 0) {
      hdma_update(i);
    } else {
      dma_add_clocks(8);
    }
  }

  status.irq_lock = true;
  event.enqueue(2, EventIrqLockRelease);
}

void sCPU::hdma_init_reset() {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].hdma_completed   = false;
    channel[i].hdma_do_transfer = false;
  }
}

void sCPU::hdma_init() {
  dma_add_clocks(8);

  for(unsigned i = 0; i < 8; i++) {
    if(!channel[i].hdma_enabled) continue;
    channel[i].dma_enabled = false;  //HDMA init during DMA will stop DMA mid-transfer

    channel[i].hdma_addr = channel[i].srcaddr;
    hdma_update(i);
  }

  status.irq_lock = true;
  event.enqueue(2, EventIrqLockRelease);
}

/*****
 * power / reset functions
 *****/

void sCPU::dma_power() {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].dmap              = 0xff;
    channel[i].direction         = 1;
    channel[i].hdma_indirect     = true;
    channel[i].reversexfer       = true;
    channel[i].fixedxfer         = true;
    channel[i].xfermode          = 7;

    channel[i].destaddr          = 0xff;

    channel[i].srcaddr           = 0xffff;
    channel[i].srcbank           = 0xff;

    channel[i].xfersize          = 0xffff;
  //channel[i].hdma_iaddr        = 0xffff;  //union with xfersize
    channel[i].hdma_ibank        = 0xff;

    channel[i].hdma_addr         = 0xffff;
    channel[i].hdma_line_counter = 0xff;
    channel[i].unknown           = 0xff;
  }
}

void sCPU::dma_reset() {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].dma_enabled       = false;
    channel[i].hdma_enabled      = false;

    channel[i].hdma_completed    = false;
    channel[i].hdma_do_transfer  = false;
  }
}

#endif
