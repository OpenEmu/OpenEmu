#include <../base.hpp>

#define SA1_CPP
namespace SNES {

SA1 sa1;

#include "serialization.cpp"
#include "bus/bus.cpp"
#include "dma/dma.cpp"
#include "memory/memory.cpp"
#include "mmio/mmio.cpp"

void SA1::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SyncAll) {
      scheduler.exit(Scheduler::SynchronizeEvent);
    }

    if(mmio.sa1_rdyb || mmio.sa1_resb) {
      //SA-1 co-processor is asleep
      tick();
      scheduler.sync_copcpu();
      continue;
    }

    if(status.interrupt_pending) {
      status.interrupt_pending = false;
      interrupt(status.interrupt_vector);
    }

    (this->*opcode_table[op_readpc()])();
  }
}

void SA1::last_cycle() {
  if(mmio.sa1_nmi && !mmio.sa1_nmicl) {
    status.interrupt_pending = true;
    status.interrupt_vector  = mmio.cnv;
    mmio.sa1_nmifl = true;
    mmio.sa1_nmicl = 1;
    regs.wai = false;
  } else if(!regs.p.i) {
    if(mmio.timer_irqen && !mmio.timer_irqcl) {
      status.interrupt_pending = true;
      status.interrupt_vector  = mmio.civ;
      mmio.timer_irqfl = true;
      regs.wai = false;
    } else if(mmio.dma_irqen && !mmio.dma_irqcl) {
      status.interrupt_pending = true;
      status.interrupt_vector  = mmio.civ;
      mmio.dma_irqfl = true;
      regs.wai = false;
    } else if(mmio.sa1_irq && !mmio.sa1_irqcl) {
      status.interrupt_pending = true;
      status.interrupt_vector  = mmio.civ;
      mmio.sa1_irqfl = true;
      regs.wai = false;
    }
  }
}

void SA1::interrupt(uint16 vector) {
  op_read(regs.pc.d);
  op_io();
  if(!regs.e) op_writestack(regs.pc.b);
  op_writestack(regs.pc.h);
  op_writestack(regs.pc.l);
  op_writestack(regs.e ? (regs.p & ~0x10) : regs.p);
  regs.pc.w = vector;
  regs.pc.b = 0x00;
  regs.p.i  = 1;
  regs.p.d  = 0;
}

bool SA1::interrupt_pending() {
  return status.interrupt_pending;
}

void SA1::tick() {
  scheduler.addclocks_cop(2);
  if(++status.tick_counter == 0) scheduler.sync_copcpu();

  //adjust counters:
  //note that internally, status counters are in clocks;
  //whereas MMIO register counters are in dots (4 clocks = 1 dot)
  if(mmio.hvselb == 0) {
    //HV timer
    status.hcounter += 2;
    if(status.hcounter >= 1364) {
      status.hcounter = 0;
      if(++status.vcounter >= status.scanlines) status.vcounter = 0;
    }
  } else {
    //linear timer
    status.hcounter += 2;
    status.vcounter += (status.hcounter >> 11);
    status.hcounter &= 0x07ff;
    status.vcounter &= 0x01ff;
  }

  //test counters for timer IRQ
  switch((mmio.ven << 1) + (mmio.hen << 0)) {
    case 0: break;
    case 1: if(status.hcounter == (mmio.hcnt << 2)) trigger_irq(); break;
    case 2: if(status.vcounter == mmio.vcnt && status.hcounter == 0) trigger_irq(); break;
    case 3: if(status.vcounter == mmio.hcnt && status.hcounter == (mmio.hcnt << 2)) trigger_irq(); break;
  }
}

void SA1::trigger_irq() {
  mmio.timer_irqfl = true;
  if(mmio.timer_irqen) mmio.timer_irqcl = 0;
}

void SA1::init() {
}

void SA1::enable() {
}

void SA1::power() {
  regs.a = regs.x = regs.y = 0x0000;
  regs.s = 0x01ff;

  reset();
}

void SA1::reset() {
  memory::vectorsp.access = 0;
  memory::cc1bwram.dma = false;
  for(unsigned addr = 0; addr < memory::iram.size(); addr++) {
    memory::iram.write(addr, 0x00);
  }
  vbrbus.init();
  sa1bus.init();

  regs.pc.d = 0x000000;
  regs.x.h  = 0x00;
  regs.y.h  = 0x00;
  regs.s.h  = 0x01;
  regs.d    = 0x0000;
  regs.db   = 0x00;
  regs.p    = 0x34;
  regs.e    = 1;
  regs.mdr  = 0x00;
  regs.wai  = false;
  CPUcore::update_table();

  status.tick_counter = 0;

  status.interrupt_pending = false;
  status.interrupt_vector  = 0x0000;

  status.scanlines = (system.region() == System::NTSC ? 262 : 312);
  status.vcounter  = 0;
  status.hcounter  = 0;

  dma.line = 0;

  //$2200 CCNT
  mmio.sa1_irq  = false;
  mmio.sa1_rdyb = false;
  mmio.sa1_resb = true;
  mmio.sa1_nmi  = false;
  mmio.smeg     = 0;

  //$2201 SIE
  mmio.cpu_irqen   = false;
  mmio.chdma_irqen = false;

  //$2202 SIC
  mmio.cpu_irqcl   = false;
  mmio.chdma_irqcl = false;

  //$2203,$2204 CRV
  mmio.crv = 0x0000;

  //$2205,$2206 CNV
  mmio.cnv = 0x0000;

  //$2207,$2208 CIV
  mmio.civ = 0x0000;

  //$2209 SCNT
  mmio.cpu_irq  = false;
  mmio.cpu_ivsw = false;
  mmio.cpu_nvsw = false;
  mmio.cmeg     = 0;

  //$220a CIE
  mmio.sa1_irqen   = false;
  mmio.timer_irqen = false;
  mmio.dma_irqen   = false;
  mmio.sa1_nmien   = false;

  //$220b CIC
  mmio.sa1_irqcl   = false;
  mmio.timer_irqcl = false;
  mmio.dma_irqcl   = false;
  mmio.sa1_nmicl   = false;

  //$220c,$220d SNV
  mmio.snv = 0x0000;

  //$220e,$220f SIV
  mmio.siv = 0x0000;

  //$2210
  mmio.hvselb = false;
  mmio.ven    = false;
  mmio.hen    = false;

  //$2212,$2213 HCNT
  mmio.hcnt = 0x0000;

  //$2214,$2215 VCNT
  mmio.vcnt = 0x0000;

  //$2220-2223 CXB, DXB, EXB, FXB
  mmio.cbmode = 0;
  mmio.dbmode = 0;
  mmio.ebmode = 0;
  mmio.fbmode = 0;

  mmio.cb = 0x00;
  mmio.db = 0x01;
  mmio.eb = 0x02;
  mmio.fb = 0x03;

  //$2224 BMAPS
  mmio.sbm = 0x00;

  //$2225 BMAP
  mmio.sw46 = false;
  mmio.cbm  = 0x00;

  //$2226 SWBE
  mmio.swen = false;

  //$2227 CWBE
  mmio.cwen = false;

  //$2228 BWPA
  mmio.bwp = 0x0f;

  //$2229 SIWP
  mmio.siwp = 0x00;

  //$222a CIWP
  mmio.ciwp = 0x00;

  //$2230 DCNT
  mmio.dmaen = false;
  mmio.dprio = false;
  mmio.cden  = false;
  mmio.cdsel = false;
  mmio.dd    = 0;
  mmio.sd    = 0;

  //$2231 CDMA
  mmio.chdend  = false;
  mmio.dmasize = 0;
  mmio.dmacb   = 0;

  //$2232-$2234 SDA
  mmio.dsa = 0x000000;

  //$2235-$2237 DDA
  mmio.dda = 0x000000;

  //$2238,$2239 DTC
  mmio.dtc = 0x0000;

  //$223f BBF
  mmio.bbf = 0;

  //$2240-$224f BRF
  for(unsigned i = 0; i < 16; i++) {
    mmio.brf[i] = 0x00;
  }

  //$2250 MCNT
  mmio.acm = 0;
  mmio.md  = 0;

  //$2251,$2252 MA
  mmio.ma = 0x0000;

  //$2253,$2254 MB
  mmio.mb = 0x0000;

  //$2258 VBD
  mmio.hl = false;
  mmio.vb = 16;

  //$2259-$225b
  mmio.va   = 0x000000;
  mmio.vbit = 0;

  //$2300 SFR
  mmio.cpu_irqfl   = false;
  mmio.chdma_irqfl = false;

  //$2301 CFR
  mmio.sa1_irqfl   = false;
  mmio.timer_irqfl = false;
  mmio.dma_irqfl   = false;
  mmio.sa1_nmifl   = false;

  //$2302,$2303 HCR
  mmio.hcr = 0x0000;

  //$2304,$2305 VCR
  mmio.vcr = 0x0000;

  //$2306-$230a MR
  mmio.mr = 0;

  //$230b
  mmio.overflow = false;
}

SA1::SA1() {
}

};
