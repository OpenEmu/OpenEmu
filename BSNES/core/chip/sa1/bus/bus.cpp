#ifdef SA1_CPP

VBRBus vbrbus;
SA1Bus sa1bus;

namespace memory {
  static StaticRAM iram(2048);
                                        //accessed by:
  static VectorSelectionPage vectorsp;  //S-CPU + SA-1
  static CPUIRAM cpuiram;               //S-CPU
  static SA1IRAM sa1iram;               //SA-1
  static SA1BWRAM sa1bwram;             //SA-1
  static CC1BWRAM cc1bwram;             //S-CPU
  static BitmapRAM bitmapram;           //SA-1
}

//$230c (VDPL), $230d (VDPH) use this bus to read variable-length data.
//this is used both to avoid VBR-reads from accessing MMIO registers, and
//to avoid syncing the S-CPU and SA-1*; as both chips are able to access
//these ports.
//(* eg, memory::cartram is used directly, as memory::sa1bwram syncs to the S-CPU)
void VBRBus::init() {
  map(MapDirect, 0x00, 0xff, 0x0000, 0xffff, memory::memory_unmapped);

  map(MapLinear, 0x00, 0x3f, 0x0000, 0x07ff, memory::iram);
  map(MapLinear, 0x00, 0x3f, 0x3000, 0x37ff, memory::iram);
  map(MapLinear, 0x00, 0x3f, 0x6000, 0x7fff, memory::cartram);
  map(MapLinear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
  map(MapLinear, 0x40, 0x4f, 0x0000, 0xffff, memory::cartram);
  map(MapLinear, 0x80, 0xbf, 0x0000, 0x07ff, memory::iram);
  map(MapLinear, 0x80, 0xbf, 0x3000, 0x37ff, memory::iram);
  map(MapLinear, 0x80, 0xbf, 0x6000, 0x7fff, memory::cartram);
  map(MapLinear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
  map(MapLinear, 0xc0, 0xff, 0x0000, 0xffff, memory::cartrom);
}

void SA1Bus::init() {
  map(MapDirect, 0x00, 0xff, 0x0000, 0xffff, memory::memory_unmapped);
  for(unsigned i = 0x2200; i <= 0x23ff; i++) memory::mmio.map(i, sa1);

  map(MapLinear, 0x00, 0x3f, 0x0000, 0x07ff, memory::sa1iram);
  map(MapDirect, 0x00, 0x3f, 0x2200, 0x23ff, memory::mmio);
  map(MapLinear, 0x00, 0x3f, 0x3000, 0x37ff, memory::sa1iram);
  map(MapLinear, 0x00, 0x3f, 0x6000, 0x7fff, memory::sa1bwram);
  map(MapLinear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
  map(MapLinear, 0x40, 0x4f, 0x0000, 0xffff, memory::sa1bwram);
  map(MapLinear, 0x60, 0x6f, 0x0000, 0xffff, memory::bitmapram);
  map(MapLinear, 0x80, 0xbf, 0x0000, 0x07ff, memory::sa1iram);
  map(MapDirect, 0x80, 0xbf, 0x2200, 0x23ff, memory::mmio);
  map(MapLinear, 0x80, 0xbf, 0x3000, 0x37ff, memory::sa1iram);
  map(MapLinear, 0x80, 0xbf, 0x6000, 0x7fff, memory::sa1bwram);
  map(MapLinear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
  map(MapLinear, 0xc0, 0xff, 0x0000, 0xffff, memory::cartrom);

  bus.map(MapLinear, 0x00, 0x3f, 0x3000, 0x37ff, memory::cpuiram);
  bus.map(MapLinear, 0x00, 0x3f, 0x6000, 0x7fff, memory::cc1bwram);
  bus.map(MapLinear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
  bus.map(MapLinear, 0x40, 0x4f, 0x0000, 0xffff, memory::cc1bwram);
  bus.map(MapLinear, 0x80, 0xbf, 0x3000, 0x37ff, memory::cpuiram);
  bus.map(MapLinear, 0x80, 0xbf, 0x6000, 0x7fff, memory::cc1bwram);
  bus.map(MapLinear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
  bus.map(MapLinear, 0xc0, 0xff, 0x0000, 0xffff, memory::cartrom);

  memory::vectorsp.sync();
}

//===================
//VectorSelectionPage
//===================

//this class maps $00:[ff00-ffff] for the purpose of supporting:
//$2209.d6 IVSW (S-CPU IRQ vector selection) (0 = cart, 1 = SA-1)
//$2209.d4 NVSW (S-CPU NMI vector selection) (0 = cart, 1 = SA-1)
//when set, vector addresses are over-ridden with SA-1 register settings:
//SIV = S-CPU IRQ vector address override
//SNV = S-CPU NMI vector address override
//
//$00:[ffea-ffeb|ffee-ffef] are special cased on read;
//all other addresses return original mapped data.

uint8 VectorSelectionPage::read(unsigned addr) {
  switch(0xff00 | (addr & 0xff)) {
    case 0xffea: case 0xffeb: {
      if(sa1.mmio.cpu_nvsw == true) return (sa1.mmio.snv >> ((addr & 1) << 3));
    } break;

    case 0xffee: case 0xffef: {
      if(sa1.mmio.cpu_ivsw == true) return (sa1.mmio.siv >> ((addr & 1) << 3));
    } break;
  }

  return access->read(addr);
}

void VectorSelectionPage::write(unsigned addr, uint8 data) {
  return access->write(addr, data);
}

//call this whenever bus is remapped.
//note: S-CPU and SA-1 bus always share $00:[ff00-ffff] as cartridge ROM data;
//the SA-1 MMC does not allow mapping these independently between processors.
//this allows this class to be shared for both, caching only ones' access class.
void VectorSelectionPage::sync() {
  if(bus.page[0x00ff00 >> 8].access != this) {
    //bus was re-mapped, hook access routine
    access = bus.page[0x00ff00 >> 8].access;
       bus.page[0x00ff00 >> 8].access = this;
    sa1bus.page[0x00ff00 >> 8].access = this;
  }
}

//=======
//SA1IRAM
//=======

unsigned SA1IRAM::size() const {
  return memory::iram.size();
}

uint8 SA1IRAM::read(unsigned addr) {
  scheduler.sync_copcpu();
  return memory::iram.read(addr);
}

void SA1IRAM::write(unsigned addr, uint8 data) {
  scheduler.sync_copcpu();
  memory::iram.write(addr, data);
}

//=======
//CPUIRAM
//=======

unsigned CPUIRAM::size() const {
  return memory::iram.size();
}

uint8 CPUIRAM::read(unsigned addr) {
  scheduler.sync_cpucop();
  return memory::iram.read(addr);
}

void CPUIRAM::write(unsigned addr, uint8 data) {
  scheduler.sync_cpucop();
  memory::iram.write(addr, data);
}

//========
//SA1BWRAM
//========

unsigned SA1BWRAM::size() const {
  return memory::cartram.size();
}

uint8 SA1BWRAM::read(unsigned addr) {
  scheduler.sync_copcpu();
  return memory::cartram.read(addr);
}

void SA1BWRAM::write(unsigned addr, uint8 data) {
  scheduler.sync_copcpu();
  memory::cartram.write(addr, data);
}

//========
//CC1BWRAM
//========

unsigned CC1BWRAM::size() const {
  return memory::cartram.size();
}

uint8 CC1BWRAM::read(unsigned addr) {
  scheduler.sync_cpucop();
  if(dma) return sa1.dma_cc1_read(addr);
  return memory::cartram.read(addr);
}

void CC1BWRAM::write(unsigned addr, uint8 data) {
  scheduler.sync_cpucop();
  memory::cartram.write(addr, data);
}

//=========
//BitmapRAM
//=========

unsigned BitmapRAM::size() const {
  return 0x100000;
}

uint8 BitmapRAM::read(unsigned addr) {
  scheduler.sync_copcpu();

  if(sa1.mmio.bbf == 0) {
    //4bpp
    unsigned shift = addr & 1;
    addr = (addr >> 1) & (memory::cartram.size() - 1);
    switch(shift) { default:
      case 0: return (memory::cartram.read(addr) >> 0) & 15;
      case 1: return (memory::cartram.read(addr) >> 4) & 15;
    }
  } else {
    //2bpp
    unsigned shift = addr & 3;
    addr = (addr >> 2) & (memory::cartram.size() - 1);
    switch(shift) { default:
      case 0: return (memory::cartram.read(addr) >> 0) & 3;
      case 1: return (memory::cartram.read(addr) >> 2) & 3;
      case 2: return (memory::cartram.read(addr) >> 4) & 3;
      case 3: return (memory::cartram.read(addr) >> 6) & 3;
    }
  }
}

void BitmapRAM::write(unsigned addr, uint8 data) {
  scheduler.sync_copcpu();

  if(sa1.mmio.bbf == 0) {
    //4bpp
    unsigned shift = addr & 1;
    addr = (addr >> 1) & (memory::cartram.size() - 1);
    switch(shift) { default:
      case 0: data = (memory::cartram.read(addr) & 0xf0) | ((data & 15) << 0); break;
      case 1: data = (memory::cartram.read(addr) & 0x0f) | ((data & 15) << 4); break;
    }
  } else {
    //2bpp
    unsigned shift = addr & 3;
    addr = (addr >> 2) & (memory::cartram.size() - 1);
    switch(shift) { default:
      case 0: data = (memory::cartram.read(addr) & 0xfc) | ((data &  3) << 0); break;
      case 1: data = (memory::cartram.read(addr) & 0xf3) | ((data &  3) << 2); break;
      case 2: data = (memory::cartram.read(addr) & 0xcf) | ((data &  3) << 4); break;
      case 3: data = (memory::cartram.read(addr) & 0x3f) | ((data &  3) << 6); break;
    }
  }

  memory::cartram.write(addr, data);
}

#endif
