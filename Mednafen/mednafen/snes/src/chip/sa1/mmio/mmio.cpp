#ifdef SA1_CPP

//BS-X flash carts, when present, are mapped to 0x400000+
Memory& SA1::mmio_access(unsigned &addr) {
  if(!memory::bsxflash.data()) return memory::cartrom;
  if(addr < 0x400000) return memory::cartrom;
  addr &= 0x3fffff;
  return bsxflash;
}

//(CCNT) SA-1 control
void SA1::mmio_w2200(uint8 data) {
  if(mmio.sa1_resb && !(data & 0x80)) {
    //reset SA-1 CPU
    regs.pc.w = mmio.crv;
    regs.pc.b = 0x00;
  }

  mmio.sa1_irq  = (data & 0x80);
  mmio.sa1_rdyb = (data & 0x40);
  mmio.sa1_resb = (data & 0x20);
  mmio.sa1_nmi  = (data & 0x10);
  mmio.smeg     = (data & 0x0f);

  if(mmio.sa1_irq) {
    mmio.sa1_irqfl = true;
    if(mmio.sa1_irqen) mmio.sa1_irqcl = 0;
  }

  if(mmio.sa1_nmi) {
    mmio.sa1_nmifl = true;
    if(mmio.sa1_nmien) mmio.sa1_nmicl = 0;
  }
}

//(SIE) S-CPU interrupt enable
void SA1::mmio_w2201(uint8 data) {
  if(!mmio.cpu_irqen && (data & 0x80)) {
    if(mmio.cpu_irqfl) {
      mmio.cpu_irqcl = 0;
      cpu.regs.irq = 1;
    }
  }

  if(!mmio.chdma_irqen && (data & 0x20)) {
    if(mmio.chdma_irqfl) {
      mmio.chdma_irqcl = 0;
      cpu.regs.irq = 1;
    }
  }

  mmio.cpu_irqen   = (data & 0x80);
  mmio.chdma_irqen = (data & 0x20);
}

//(SIC) S-CPU interrupt clear
void SA1::mmio_w2202(uint8 data) {
  mmio.cpu_irqcl   = (data & 0x80);
  mmio.chdma_irqcl = (data & 0x20);

  if(mmio.cpu_irqcl  ) mmio.cpu_irqfl   = false;
  if(mmio.chdma_irqcl) mmio.chdma_irqfl = false;

  if(!mmio.cpu_irqfl && !mmio.chdma_irqfl) cpu.regs.irq = 0;
}

//(CRV) SA-1 reset vector
void SA1::mmio_w2203(uint8 data) { mmio.crv = (mmio.crv & 0xff00) | data; }
void SA1::mmio_w2204(uint8 data) { mmio.crv = (data << 8) | (mmio.crv & 0xff); }

//(CNV) SA-1 NMI vector
void SA1::mmio_w2205(uint8 data) { mmio.cnv = (mmio.cnv & 0xff00) | data; }
void SA1::mmio_w2206(uint8 data) { mmio.cnv = (data << 8) | (mmio.cnv & 0xff); }

//(CIV) SA-1 IRQ vector
void SA1::mmio_w2207(uint8 data) { mmio.civ = (mmio.civ & 0xff00) | data; }
void SA1::mmio_w2208(uint8 data) { mmio.civ = (data << 8) | (mmio.civ & 0xff); }

//(SCNT) S-CPU control
void SA1::mmio_w2209(uint8 data) {
  mmio.cpu_irq  = (data & 0x80);
  mmio.cpu_ivsw = (data & 0x40);
  mmio.cpu_nvsw = (data & 0x10);
  mmio.cmeg     = (data & 0x0f);

  if(mmio.cpu_irq) {
    mmio.cpu_irqfl = true;
    if(mmio.cpu_irqen) {
      mmio.cpu_irqcl = 0;
      cpu.regs.irq = 1;
    }
  }
}

//(CIE) SA-1 interrupt enable
void SA1::mmio_w220a(uint8 data) {
  if(!mmio.sa1_irqen   && (data & 0x80) && mmio.sa1_irqfl  ) mmio.sa1_irqcl   = 0;
  if(!mmio.timer_irqen && (data & 0x40) && mmio.timer_irqfl) mmio.timer_irqcl = 0;
  if(!mmio.dma_irqen   && (data & 0x20) && mmio.dma_irqfl  ) mmio.dma_irqcl   = 0;
  if(!mmio.sa1_nmien   && (data & 0x10) && mmio.sa1_nmifl  ) mmio.sa1_nmicl   = 0;

  mmio.sa1_irqen   = (data & 0x80);
  mmio.timer_irqen = (data & 0x40);
  mmio.dma_irqen   = (data & 0x20);
  mmio.sa1_nmien   = (data & 0x10);
}

//(CIC) SA-1 interrupt clear
void SA1::mmio_w220b(uint8 data) {
  mmio.sa1_irqcl   = (data & 0x80);
  mmio.timer_irqcl = (data & 0x40);
  mmio.dma_irqcl   = (data & 0x20);
  mmio.sa1_nmicl   = (data & 0x10);

  if(mmio.sa1_irqcl)   mmio.sa1_irqfl   = false;
  if(mmio.timer_irqcl) mmio.timer_irqfl = false;
  if(mmio.dma_irqcl)   mmio.dma_irqfl   = false;
  if(mmio.sa1_nmicl)   mmio.sa1_nmifl   = false;
}

//(SNV) S-CPU NMI vector
void SA1::mmio_w220c(uint8 data) { mmio.snv = (mmio.snv & 0xff00) | data;      }
void SA1::mmio_w220d(uint8 data) { mmio.snv = (data << 8) | (mmio.snv & 0xff); }

//(SIV) S-CPU IRQ vector
void SA1::mmio_w220e(uint8 data) { mmio.siv = (mmio.siv & 0xff00) | data;      }
void SA1::mmio_w220f(uint8 data) { mmio.siv = (data << 8) | (mmio.siv & 0xff); }

//(TMC) H/V timer control
void SA1::mmio_w2210(uint8 data) {
  mmio.hvselb = (data & 0x80);
  mmio.ven    = (data & 0x02);
  mmio.hen    = (data & 0x01);
}

//(CTR) SA-1 timer restart
void SA1::mmio_w2211(uint8 data) {
  status.vcounter = 0;
  status.hcounter = 0;
}

//(HCNT) H-count
void SA1::mmio_w2212(uint8 data) { mmio.hcnt = (mmio.hcnt & 0xff00) | (data << 0); }
void SA1::mmio_w2213(uint8 data) { mmio.hcnt = (mmio.hcnt & 0x00ff) | (data << 8); }

//(VCNT) V-count
void SA1::mmio_w2214(uint8 data) { mmio.vcnt = (mmio.vcnt & 0xff00) | (data << 0); }
void SA1::mmio_w2215(uint8 data) { mmio.vcnt = (mmio.vcnt & 0x00ff) | (data << 8); }

//(CXB) Super MMC bank C
void SA1::mmio_w2220(uint8 data) {
  mmio.cbmode = (data & 0x80);
  mmio.cb     = (data & 0x07);

  unsigned addr = mmio.cb << 20;
  Memory &access = mmio_access(addr);

  if(mmio.cbmode == 0) {
       bus.map(Bus::MapLinear, 0x00, 0x1f, 0x8000, 0xffff, memory::cartrom, 0x000000);
    sa1bus.map(Bus::MapLinear, 0x00, 0x1f, 0x8000, 0xffff, memory::cartrom, 0x000000);
  } else {
       bus.map(Bus::MapLinear, 0x00, 0x1f, 0x8000, 0xffff, access, addr);
    sa1bus.map(Bus::MapLinear, 0x00, 0x1f, 0x8000, 0xffff, access, addr);
  }

     bus.map(Bus::MapLinear, 0xc0, 0xcf, 0x0000, 0xffff, access, addr);
  sa1bus.map(Bus::MapLinear, 0xc0, 0xcf, 0x0000, 0xffff, access, addr);

  memory::vectorsp.sync();
}

//(DXB) Super MMC bank D
void SA1::mmio_w2221(uint8 data) {
  mmio.dbmode = (data & 0x80);
  mmio.db     = (data & 0x07);

  unsigned addr = mmio.db << 20;
  Memory &access = mmio_access(addr);

  if(mmio.dbmode == 0) {
       bus.map(Bus::MapLinear, 0x20, 0x3f, 0x8000, 0xffff, memory::cartrom, 0x100000);
    sa1bus.map(Bus::MapLinear, 0x20, 0x3f, 0x8000, 0xffff, memory::cartrom, 0x100000);
  } else {
       bus.map(Bus::MapLinear, 0x20, 0x3f, 0x8000, 0xffff, access, addr);
    sa1bus.map(Bus::MapLinear, 0x20, 0x3f, 0x8000, 0xffff, access, addr);
  }

     bus.map(Bus::MapLinear, 0xd0, 0xdf, 0x0000, 0xffff, access, addr);
  sa1bus.map(Bus::MapLinear, 0xd0, 0xdf, 0x0000, 0xffff, access, addr);
}

//(EXB) Super MMC bank E
void SA1::mmio_w2222(uint8 data) {
  mmio.ebmode = (data & 0x80);
  mmio.eb     = (data & 0x07);

  unsigned addr = mmio.eb << 20;
  Memory &access = mmio_access(addr);

  if(mmio.ebmode == 0) {
       bus.map(Bus::MapLinear, 0x80, 0x9f, 0x8000, 0xffff, memory::cartrom, 0x200000);
    sa1bus.map(Bus::MapLinear, 0x80, 0x9f, 0x8000, 0xffff, memory::cartrom, 0x200000);
  } else {
       bus.map(Bus::MapLinear, 0x80, 0x9f, 0x8000, 0xffff, access, addr);
    sa1bus.map(Bus::MapLinear, 0x80, 0x9f, 0x8000, 0xffff, access, addr);
  }

     bus.map(Bus::MapLinear, 0xe0, 0xef, 0x0000, 0xffff, access, addr);
  sa1bus.map(Bus::MapLinear, 0xe0, 0xef, 0x0000, 0xffff, access, addr);
}

//(FXB) Super MMC bank F
void SA1::mmio_w2223(uint8 data) {
  mmio.fbmode = (data & 0x80);
  mmio.fb     = (data & 0x07);

  unsigned addr = mmio.fb << 20;
  Memory &access = mmio_access(addr);

  if(mmio.fbmode == 0) {
       bus.map(Bus::MapLinear, 0xa0, 0xbf, 0x8000, 0xffff, memory::cartrom, 0x300000);
    sa1bus.map(Bus::MapLinear, 0xa0, 0xbf, 0x8000, 0xffff, memory::cartrom, 0x300000);
  } else {
       bus.map(Bus::MapLinear, 0xa0, 0xbf, 0x8000, 0xffff, access, addr);
    sa1bus.map(Bus::MapLinear, 0xa0, 0xbf, 0x8000, 0xffff, access, addr);
  }

     bus.map(Bus::MapLinear, 0xf0, 0xff, 0x0000, 0xffff, access, addr);
  sa1bus.map(Bus::MapLinear, 0xf0, 0xff, 0x0000, 0xffff, access, addr);
}

//(BMAPS) S-CPU BW-RAM address mapping
void SA1::mmio_w2224(uint8 data) {
  mmio.sbm = (data & 0x1f);

  bus.map(Bus::MapLinear, 0x00, 0x3f, 0x6000, 0x7fff, memory::cc1bwram, mmio.sbm * 0x2000, 0x2000);
  bus.map(Bus::MapLinear, 0x80, 0xbf, 0x6000, 0x7fff, memory::cc1bwram, mmio.sbm * 0x2000, 0x2000);
}

//(BMAP) SA-1 BW-RAM address mapping
void SA1::mmio_w2225(uint8 data) {
  mmio.sw46 = (data & 0x80);
  mmio.cbm  = (data & 0x7f);

  if(mmio.sw46 == 0) {
    //$[40-43]:[0000-ffff] x  32 projection
    sa1bus.map(Bus::MapLinear, 0x00, 0x3f, 0x6000, 0x7fff, memory::sa1bwram, (mmio.cbm & 0x1f) * 0x2000, 0x2000);
    sa1bus.map(Bus::MapLinear, 0x80, 0xbf, 0x6000, 0x7fff, memory::sa1bwram, (mmio.cbm & 0x1f) * 0x2000, 0x2000);
  } else {
    //$[60-6f]:[0000-ffff] x 128 projection
    sa1bus.map(Bus::MapLinear, 0x00, 0x3f, 0x6000, 0x7fff, memory::bitmapram, mmio.cbm * 0x2000, 0x2000);
    sa1bus.map(Bus::MapLinear, 0x80, 0xbf, 0x6000, 0x7fff, memory::bitmapram, mmio.cbm * 0x2000, 0x2000);
  }
}

//(SWBE) S-CPU BW-RAM write enable
void SA1::mmio_w2226(uint8 data) {
  mmio.swen = (data & 0x80);
}

//(CWBE) SA-1 BW-RAM write enable
void SA1::mmio_w2227(uint8 data) {
  mmio.cwen = (data & 0x80);
}

//(BWPA) BW-RAM write-protected area
void SA1::mmio_w2228(uint8 data) {
  mmio.bwp = (data & 0x0f);
}

//(SIWP) S-CPU I-RAM write protection
void SA1::mmio_w2229(uint8 data) {
  mmio.siwp = data;
}

//(CIWP) SA-1 I-RAM write protection
void SA1::mmio_w222a(uint8 data) {
  mmio.ciwp = data;
}

//(DCNT) DMA control
void SA1::mmio_w2230(uint8 data) {
  mmio.dmaen = (data & 0x80);
  mmio.dprio = (data & 0x40);
  mmio.cden  = (data & 0x20);
  mmio.cdsel = (data & 0x10);
  mmio.dd    = (data & 0x04);
  mmio.sd    = (data & 0x03);

  if(mmio.dmaen == 0) dma.line = 0;
}

//(CDMA) character conversion DMA parameters
void SA1::mmio_w2231(uint8 data) {
  mmio.chdend  = (data & 0x80);
  mmio.dmasize = (data >> 2) & 7;
  mmio.dmacb   = (data & 0x03);

  if(mmio.chdend) memory::cc1bwram.dma = false;
  if(mmio.dmasize > 5) mmio.dmasize = 5;
  if(mmio.dmacb   > 2) mmio.dmacb   = 2;
}

//(SDA) DMA source device start address
void SA1::mmio_w2232(uint8 data) { mmio.dsa = (mmio.dsa & 0xffff00) | (data <<  0); }
void SA1::mmio_w2233(uint8 data) { mmio.dsa = (mmio.dsa & 0xff00ff) | (data <<  8); }
void SA1::mmio_w2234(uint8 data) { mmio.dsa = (mmio.dsa & 0x00ffff) | (data << 16); }

//(DDA) DMA destination start address
void SA1::mmio_w2235(uint8 data) {
  mmio.dda = (mmio.dda & 0xffff00) | (data <<  0);
}

void SA1::mmio_w2236(uint8 data) {
  mmio.dda = (mmio.dda & 0xff00ff) | (data <<  8);

  if(mmio.dmaen == true) {
    if(mmio.cden == 0 && mmio.dd == DMA::DestIRAM) {
      dma_normal();
    } else if(mmio.cden == 1 && mmio.cdsel == 1) {
      dma_cc1();
    }
  }
}

void SA1::mmio_w2237(uint8 data) {
  mmio.dda = (mmio.dda & 0x00ffff) | (data << 16);

  if(mmio.dmaen == true) {
    if(mmio.cden == 0 && mmio.dd == DMA::DestBWRAM) {
      dma_normal();
    }
  }
}

//(DTC) DMA terminal counter
void SA1::mmio_w2238(uint8 data) { mmio.dtc = (mmio.dtc & 0xff00) | (data << 0); }
void SA1::mmio_w2239(uint8 data) { mmio.dtc = (mmio.dtc & 0x00ff) | (data << 8); }

//(BBF) BW-RAM bitmap format
void SA1::mmio_w223f(uint8 data) {
  mmio.bbf = (data & 0x80);
}

//(BRF) bitmap register files
void SA1::mmio_w2240(uint8 data) { mmio.brf[ 0] = data; }
void SA1::mmio_w2241(uint8 data) { mmio.brf[ 1] = data; }
void SA1::mmio_w2242(uint8 data) { mmio.brf[ 2] = data; }
void SA1::mmio_w2243(uint8 data) { mmio.brf[ 3] = data; }
void SA1::mmio_w2244(uint8 data) { mmio.brf[ 4] = data; }
void SA1::mmio_w2245(uint8 data) { mmio.brf[ 5] = data; }
void SA1::mmio_w2246(uint8 data) { mmio.brf[ 6] = data; }
void SA1::mmio_w2247(uint8 data) { mmio.brf[ 7] = data;
  if(mmio.dmaen == true) {
    if(mmio.cden == 1 && mmio.cdsel == 0) {
      dma_cc2();
    }
  }
}

void SA1::mmio_w2248(uint8 data) { mmio.brf[ 8] = data; }
void SA1::mmio_w2249(uint8 data) { mmio.brf[ 9] = data; }
void SA1::mmio_w224a(uint8 data) { mmio.brf[10] = data; }
void SA1::mmio_w224b(uint8 data) { mmio.brf[11] = data; }
void SA1::mmio_w224c(uint8 data) { mmio.brf[12] = data; }
void SA1::mmio_w224d(uint8 data) { mmio.brf[13] = data; }
void SA1::mmio_w224e(uint8 data) { mmio.brf[14] = data; }
void SA1::mmio_w224f(uint8 data) { mmio.brf[15] = data;
  if(mmio.dmaen == true) {
    if(mmio.cden == 1 && mmio.cdsel == 0) {
      dma_cc2();
    }
  }
}

//(MCNT) arithmetic control
void SA1::mmio_w2250(uint8 data) {
  mmio.acm = (data & 0x02);
  mmio.md  = (data & 0x01);

  if(mmio.acm) mmio.mr = 0;
}

//(MAL) multiplicand / dividend low
void SA1::mmio_w2251(uint8 data) {
  mmio.ma = (mmio.ma & 0xff00) | data;
}

//(MAH) multiplicand / dividend high
void SA1::mmio_w2252(uint8 data) {
  mmio.ma = (data << 8) | (mmio.ma & 0x00ff);
}

//(MBL) multiplier / divisor low
void SA1::mmio_w2253(uint8 data) {
  mmio.mb = (mmio.mb & 0xff00) | data;
}

//(MBH) multiplier / divisor high
//multiplication / cumulative sum only resets MB
//division resets both MA and MB
void SA1::mmio_w2254(uint8 data) {
  mmio.mb = (data << 8) | (mmio.mb & 0x00ff);

  if(mmio.acm == 0) {
    if(mmio.md == 0) {
      //signed multiplication
      mmio.mr = (int16)mmio.ma * (int16)mmio.mb;
      mmio.mb = 0;
    } else {
      //unsigned division
      if(mmio.mb == 0) {
        mmio.mr = 0;
      } else {
        int16  quotient  = (int16)mmio.ma / (uint16)mmio.mb;
        uint16 remainder = (int16)mmio.ma % (uint16)mmio.mb;
        mmio.mr = (remainder << 16) | quotient;
      }
      mmio.ma = 0;
      mmio.mb = 0;
    }
  } else {
    //sigma (accumulative multiplication)
    mmio.mr += (int16)mmio.ma * (int16)mmio.mb;
    mmio.overflow = (mmio.mr >= (1ULL << 40));
    mmio.mr &= (1ULL << 40) - 1;
    mmio.mb = 0;
  }
}

//(VBD) variable-length bit processing
void SA1::mmio_w2258(uint8 data) {
  mmio.hl = (data & 0x80);
  mmio.vb = (data & 0x0f);
  if(mmio.vb == 0) mmio.vb = 16;

  if(mmio.hl == 0) {
    //fixed mode
    mmio.vbit += mmio.vb;
    mmio.va += (mmio.vbit >> 3);
    mmio.vbit &= 7;
  }
}

//(VDA) variable-length bit game pak ROM start address
void SA1::mmio_w2259(uint8 data) { mmio.va = (mmio.va & 0xffff00) | (data <<  0); }
void SA1::mmio_w225a(uint8 data) { mmio.va = (mmio.va & 0xff00ff) | (data <<  8); }
void SA1::mmio_w225b(uint8 data) { mmio.va = (mmio.va & 0x00ffff) | (data << 16); mmio.vbit = 0; }

//(SFR) S-CPU flag read
uint8 SA1::mmio_r2300() {
  uint8 data;
  data  = mmio.cpu_irqfl   << 7;
  data |= mmio.cpu_ivsw    << 6;
  data |= mmio.chdma_irqfl << 5;
  data |= mmio.cpu_nvsw    << 4;
  data |= mmio.cmeg;
  return data;
}

//(CFR) SA-1 flag read
uint8 SA1::mmio_r2301() {
  uint8 data;
  data  = mmio.sa1_irqfl   << 7;
  data |= mmio.timer_irqfl << 6;
  data |= mmio.dma_irqfl   << 5;
  data |= mmio.sa1_nmifl   << 4;
  data |= mmio.smeg;
  return data;
}

//(HCR) hcounter read
uint8 SA1::mmio_r2302() {
  //latch counters
  mmio.hcr = status.hcounter >> 2;
  mmio.vcr = status.vcounter;
                            return mmio.hcr >> 0; }
uint8 SA1::mmio_r2303() { return mmio.hcr >> 8; }

//(VCR) vcounter read
uint8 SA1::mmio_r2304() { return mmio.vcr >> 0; }
uint8 SA1::mmio_r2305() { return mmio.vcr >> 8; }

//(MR) arithmetic result
uint8 SA1::mmio_r2306() { return mmio.mr >>  0; }
uint8 SA1::mmio_r2307() { return mmio.mr >>  8; }
uint8 SA1::mmio_r2308() { return mmio.mr >> 16; }
uint8 SA1::mmio_r2309() { return mmio.mr >> 24; }
uint8 SA1::mmio_r230a() { return mmio.mr >> 32; }

//(OF) arithmetic overflow flag
uint8 SA1::mmio_r230b() { return mmio.overflow << 7; }

//(VDPL) variable-length data read port low
uint8 SA1::mmio_r230c() {
  uint32 data = (vbrbus.read(mmio.va + 0) <<  0)
              | (vbrbus.read(mmio.va + 1) <<  8)
              | (vbrbus.read(mmio.va + 2) << 16);
  data >>= mmio.vbit;
  return data >> 0;
}

//(VDPH) variable-length data read port high
uint8 SA1::mmio_r230d() {
  uint32 data = (vbrbus.read(mmio.va + 0) <<  0)
              | (vbrbus.read(mmio.va + 1) <<  8)
              | (vbrbus.read(mmio.va + 2) << 16);
  data >>= mmio.vbit;

  if(mmio.hl == 1) {
    //auto-increment mode
    mmio.vbit += mmio.vb;
    mmio.va += (mmio.vbit >> 3);
    mmio.vbit &= 7;
  }

  return data >> 8;
}

//(VC) version code register
uint8 SA1::mmio_r230e() {
  return 0x01;  //true value unknown
}

uint8 SA1::mmio_read(unsigned addr) {
  (co_active() == scheduler.thread_cpu ? scheduler.sync_cpucop() : scheduler.sync_copcpu());
  addr &= 0xffff;

  switch(addr) {
    case 0x2300: return mmio_r2300();
    case 0x2301: return mmio_r2301();
    case 0x2302: return mmio_r2302();
    case 0x2303: return mmio_r2303();
    case 0x2304: return mmio_r2304();
    case 0x2305: return mmio_r2305();
    case 0x2306: return mmio_r2306();
    case 0x2307: return mmio_r2307();
    case 0x2308: return mmio_r2308();
    case 0x2309: return mmio_r2309();
    case 0x230a: return mmio_r230a();
    case 0x230b: return mmio_r230b();
    case 0x230c: return mmio_r230c();
    case 0x230d: return mmio_r230d();
    case 0x230e: return mmio_r230e();
  }

  return 0x00;
}

void SA1::mmio_write(unsigned addr, uint8 data) {
  (co_active() == scheduler.thread_cpu ? scheduler.sync_cpucop() : scheduler.sync_copcpu());
  addr &= 0xffff;

  switch(addr) {
    case 0x2200: return mmio_w2200(data);
    case 0x2201: return mmio_w2201(data);
    case 0x2202: return mmio_w2202(data);
    case 0x2203: return mmio_w2203(data);
    case 0x2204: return mmio_w2204(data);
    case 0x2205: return mmio_w2205(data);
    case 0x2206: return mmio_w2206(data);
    case 0x2207: return mmio_w2207(data);
    case 0x2208: return mmio_w2208(data);
    case 0x2209: return mmio_w2209(data);
    case 0x220a: return mmio_w220a(data);
    case 0x220b: return mmio_w220b(data);
    case 0x220c: return mmio_w220c(data);
    case 0x220d: return mmio_w220d(data);
    case 0x220e: return mmio_w220e(data);
    case 0x220f: return mmio_w220f(data);

    case 0x2210: return mmio_w2210(data);
    case 0x2211: return mmio_w2211(data);
    case 0x2212: return mmio_w2212(data);
    case 0x2213: return mmio_w2213(data);
    case 0x2214: return mmio_w2214(data);
    case 0x2215: return mmio_w2215(data);

    case 0x2220: return mmio_w2220(data);
    case 0x2221: return mmio_w2221(data);
    case 0x2222: return mmio_w2222(data);
    case 0x2223: return mmio_w2223(data);
    case 0x2224: return mmio_w2224(data);
    case 0x2225: return mmio_w2225(data);
    case 0x2226: return mmio_w2226(data);
    case 0x2227: return mmio_w2227(data);
    case 0x2228: return mmio_w2228(data);
    case 0x2229: return mmio_w2229(data);
    case 0x222a: return mmio_w222a(data);

    case 0x2230: return mmio_w2230(data);
    case 0x2231: return mmio_w2231(data);
    case 0x2232: return mmio_w2232(data);
    case 0x2233: return mmio_w2233(data);
    case 0x2234: return mmio_w2234(data);
    case 0x2235: return mmio_w2235(data);
    case 0x2236: return mmio_w2236(data);
    case 0x2237: return mmio_w2237(data);
    case 0x2238: return mmio_w2238(data);
    case 0x2239: return mmio_w2239(data);

    case 0x223f: return mmio_w223f(data);
    case 0x2240: return mmio_w2240(data);
    case 0x2241: return mmio_w2241(data);
    case 0x2242: return mmio_w2242(data);
    case 0x2243: return mmio_w2243(data);
    case 0x2244: return mmio_w2244(data);
    case 0x2245: return mmio_w2245(data);
    case 0x2246: return mmio_w2246(data);
    case 0x2247: return mmio_w2247(data);
    case 0x2248: return mmio_w2248(data);
    case 0x2249: return mmio_w2249(data);
    case 0x224a: return mmio_w224a(data);
    case 0x224b: return mmio_w224b(data);
    case 0x224c: return mmio_w224c(data);
    case 0x224d: return mmio_w224d(data);
    case 0x224e: return mmio_w224e(data);
    case 0x224f: return mmio_w224f(data);

    case 0x2250: return mmio_w2250(data);
    case 0x2251: return mmio_w2251(data);
    case 0x2252: return mmio_w2252(data);
    case 0x2253: return mmio_w2253(data);
    case 0x2254: return mmio_w2254(data);

    case 0x2258: return mmio_w2258(data);
    case 0x2259: return mmio_w2259(data);
    case 0x225a: return mmio_w225a(data);
    case 0x225b: return mmio_w225b(data);
  }
}

#endif
