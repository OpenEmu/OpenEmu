#include <../base.hpp>

#define SCPU_CPP
namespace SNES {

#if defined(DEBUGGER)
  #include "debugger/debugger.cpp"
  sCPUDebug cpu;
#else
  sCPU cpu;
#endif

#include "serialization.cpp"
#include "dma/dma.cpp"
#include "memory/memory.cpp"
#include "mmio/mmio.cpp"
#include "timing/timing.cpp"

void sCPU::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SyncCpu) {
      scheduler.sync = Scheduler::SyncAll;
      scheduler.exit();
    }

    if(status.interrupt_pending) {
      status.interrupt_pending = false;
      if(status.nmi_pending) {
        status.nmi_pending = false;
        status.interrupt_vector = (regs.e == false ? 0xffea : 0xfffa);
        op_irq();
      } else if(status.irq_pending) {
        status.irq_pending = false;
        status.interrupt_vector = (regs.e == false ? 0xffee : 0xfffe);
        op_irq();
      } else if(status.reset_pending) {
        status.reset_pending = false;
        add_clocks(186);
        regs.pc.l = bus.read(0xfffc);
        regs.pc.h = bus.read(0xfffd);
      }
    }

    op_step();
  }
}

void sCPU::op_step() {
  (this->*opcode_table[op_readpc()])();
}

void sCPU::op_irq() {
  op_read(regs.pc.d);
  op_io();
  if(!regs.e) op_writestack(regs.pc.b);
  op_writestack(regs.pc.h);
  op_writestack(regs.pc.l);
  op_writestack(regs.e ? (regs.p & ~0x10) : regs.p);
  rd.l = op_read(status.interrupt_vector + 0);
  regs.pc.b = 0x00;
  regs.p.i  = 1;
  regs.p.d  = 0;
  rd.h = op_read(status.interrupt_vector + 1);
  regs.pc.w = rd.w;
}

void sCPU::power() {
  CPU::power();

  regs.a = regs.x = regs.y = 0x0000;
  regs.s = 0x01ff;

  mmio_power();
  dma_power();
  timing_power();

  reset();
}

void sCPU::reset() {
  CPU::reset();

  //note: some registers are not fully reset by SNES
  regs.pc   = 0x000000;
  regs.x.h  = 0x00;
  regs.y.h  = 0x00;
  regs.s.h  = 0x01;
  regs.d    = 0x0000;
  regs.db   = 0x00;
  regs.p    = 0x34;
  regs.e    = 1;
  regs.mdr  = 0x00;
  regs.wai  = false;
  update_table();

  mmio_reset();
  dma_reset();
  timing_reset();

  apu_port[0] = 0x00;
  apu_port[1] = 0x00;
  apu_port[2] = 0x00;
  apu_port[3] = 0x00;
}

sCPU::sCPU() : event(512, bind(&sCPU::queue_event, this)) {
  PPUcounter::scanline = bind(&sCPU::scanline, this);
}

sCPU::~sCPU() {
}

};
