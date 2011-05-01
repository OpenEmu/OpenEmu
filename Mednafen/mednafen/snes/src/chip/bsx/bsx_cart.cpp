#ifdef BSX_CPP

BSXCart bsxcart;

void BSXCart::init() {
}

void BSXCart::enable() {
  for(uint16 i = 0x5000; i <= 0x5fff; i++) memory::mmio.map(i, *this);
}

void BSXCart::power() {
  reset();
}

void BSXCart::reset() {
  for(unsigned i = 0; i < 16; i++) regs.r[i] = 0x00;
  regs.r[0x07] = 0x80;
  regs.r[0x08] = 0x80;

  update_memory_map();
}

void BSXCart::update_memory_map() {
  Memory &cart = (regs.r[0x01] & 0x80) == 0x00 ? (Memory&)bsxflash : (Memory&)memory::bsxpram;

  if((regs.r[0x02] & 0x80) == 0x00) {
    //LoROM mapping
    bus.map(Bus::MapLinear, 0x00, 0x7d, 0x8000, 0xffff, cart);
    bus.map(Bus::MapLinear, 0x80, 0xff, 0x8000, 0xffff, cart);
  } else {
    //HiROM mapping
    bus.map(Bus::MapShadow, 0x00, 0x3f, 0x8000, 0xffff, cart);
    bus.map(Bus::MapLinear, 0x40, 0x7d, 0x0000, 0xffff, cart);
    bus.map(Bus::MapShadow, 0x80, 0xbf, 0x8000, 0xffff, cart);
    bus.map(Bus::MapLinear, 0xc0, 0xff, 0x0000, 0xffff, cart);
  }

  if(regs.r[0x03] & 0x80) {
    bus.map(Bus::MapLinear, 0x60, 0x6f, 0x0000, 0xffff, memory::bsxpram);
  //bus.map(Bus::MapLinear, 0x70, 0x77, 0x0000, 0xffff, memory::bsxpram);
  }

  if((regs.r[0x05] & 0x80) == 0x00) {
    bus.map(Bus::MapLinear, 0x40, 0x4f, 0x0000, 0xffff, memory::bsxpram);
  }

  if((regs.r[0x06] & 0x80) == 0x00) {
    bus.map(Bus::MapLinear, 0x50, 0x5f, 0x0000, 0xffff, memory::bsxpram);
  }

  if(regs.r[0x07] & 0x80) {
    bus.map(Bus::MapLinear, 0x00, 0x1f, 0x8000, 0xffff, memory::cartrom);
  }

  if(regs.r[0x08] & 0x80) {
    bus.map(Bus::MapLinear, 0x80, 0x9f, 0x8000, 0xffff, memory::cartrom);
  }

  bus.map(Bus::MapShadow, 0x20, 0x3f, 0x6000, 0x7fff, memory::bsxpram);
  bus.map(Bus::MapLinear, 0x70, 0x77, 0x0000, 0xffff, memory::bsxpram);
}

uint8 BSXCart::mmio_read(unsigned addr) {
  if((addr & 0xf0ffff) == 0x005000) {  //$[00-0f]:5000 MMIO
    uint8 n = (addr >> 16) & 15;
    return regs.r[n];
  }

  if((addr & 0xf8f000) == 0x105000) {  //$[10-17]:[5000-5fff] SRAM
    return memory::bsxram.read(((addr >> 16) & 7) * 0x1000 + (addr & 0xfff));
  }

  return 0x00;
}

void BSXCart::mmio_write(unsigned addr, uint8 data) {
  if((addr & 0xf0ffff) == 0x005000) {  //$[00-0f]:5000 MMIO
    uint8 n = (addr >> 16) & 15;
    regs.r[n] = data;
    if(n == 0x0e && data & 0x80) update_memory_map();
    return;
  }

  if((addr & 0xf8f000) == 0x105000) {  //$[10-17]:[5000-5fff] SRAM
    return memory::bsxram.write(((addr >> 16) & 7) * 0x1000 + (addr & 0xfff), data);
  }
}

BSXCart::BSXCart() {
}

BSXCart::~BSXCart() {
}

#endif

