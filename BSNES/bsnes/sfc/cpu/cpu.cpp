#include <sfc/sfc.hpp>

#define CPU_CPP
namespace SuperFamicom {

CPU cpu;

#include "serialization.cpp"
#include "dma/dma.cpp"
#include "memory/memory.cpp"
#include "mmio/mmio.cpp"
#include "timing/timing.cpp"

void CPU::step(unsigned clocks) {
  smp.clock -= clocks * (uint64)smp.frequency;
  ppu.clock -= clocks;
  for(unsigned i = 0; i < coprocessors.size(); i++) {
    auto &chip = *coprocessors[i];
    chip.clock -= clocks * (uint64)chip.frequency;
  }
  input.port1->clock -= clocks * (uint64)input.port1->frequency;
  input.port2->clock -= clocks * (uint64)input.port2->frequency;
  synchronize_controllers();
}

void CPU::synchronize_smp() {
  if(SMP::Threaded == true) {
    if(smp.clock < 0) co_switch(smp.thread);
  } else {
    while(smp.clock < 0) smp.enter();
  }
}

void CPU::synchronize_ppu() {
  if(PPU::Threaded == true) {
    if(ppu.clock < 0) co_switch(ppu.thread);
  } else {
    while(ppu.clock < 0) ppu.enter();
  }
}

void CPU::synchronize_coprocessors() {
  for(unsigned i = 0; i < coprocessors.size(); i++) {
    auto &chip = *coprocessors[i];
    if(chip.clock < 0) co_switch(chip.thread);
  }
}

void CPU::synchronize_controllers() {
  if(input.port1->clock < 0) co_switch(input.port1->thread);
  if(input.port2->clock < 0) co_switch(input.port2->thread);
}

void CPU::Enter() { cpu.enter(); }

void CPU::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::CPU) {
      scheduler.sync = Scheduler::SynchronizeMode::All;
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(status.interrupt_pending) {
      status.interrupt_pending = false;
      if(status.nmi_pending) {
        status.nmi_pending = false;
        regs.vector = (regs.e == false ? 0xffea : 0xfffa);
        op_irq();
        debugger.op_nmi();
      } else if(status.irq_pending) {
        status.irq_pending = false;
        regs.vector = (regs.e == false ? 0xffee : 0xfffe);
        op_irq();
        debugger.op_irq();
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

void CPU::op_step() {
  debugger.op_exec(regs.pc.d);

  (this->*opcode_table[op_readpc()])();
}

void CPU::enable() {
  function<uint8 (unsigned)> read = { &CPU::mmio_read, (CPU*)&cpu };
  function<void (unsigned, uint8)> write = { &CPU::mmio_write, (CPU*)&cpu };

  bus.map(Bus::MapMode::Direct, 0x00, 0x3f, 0x2140, 0x2183, read, write);
  bus.map(Bus::MapMode::Direct, 0x80, 0xbf, 0x2140, 0x2183, read, write);

  bus.map(Bus::MapMode::Direct, 0x00, 0x3f, 0x4016, 0x4017, read, write);
  bus.map(Bus::MapMode::Direct, 0x80, 0xbf, 0x4016, 0x4017, read, write);

  bus.map(Bus::MapMode::Direct, 0x00, 0x3f, 0x4200, 0x421f, read, write);
  bus.map(Bus::MapMode::Direct, 0x80, 0xbf, 0x4200, 0x421f, read, write);

  bus.map(Bus::MapMode::Direct, 0x00, 0x3f, 0x4300, 0x437f, read, write);
  bus.map(Bus::MapMode::Direct, 0x80, 0xbf, 0x4300, 0x437f, read, write);

  read = [](unsigned addr) { return cpu.wram[addr]; };
  write = [](unsigned addr, uint8 data) { cpu.wram[addr] = data; };

  bus.map(Bus::MapMode::Linear, 0x00, 0x3f, 0x0000, 0x1fff, read, write, 0x000000, 0x002000);
  bus.map(Bus::MapMode::Linear, 0x80, 0xbf, 0x0000, 0x1fff, read, write, 0x000000, 0x002000);
  bus.map(Bus::MapMode::Linear, 0x7e, 0x7f, 0x0000, 0xffff, read, write);
}

void CPU::power() {
  cpu_version = config.cpu.version;
  for(auto &n : wram) n = random(config.cpu.wram_init_value);

  regs.a = regs.x = regs.y = 0x0000;
  regs.s = 0x01ff;

  mmio_power();
  dma_power();
  timing_power();
}

void CPU::reset() {
  create(Enter, system.cpu_frequency());
  coprocessors.reset();
  PPUcounter::reset();

  //note: some registers are not fully reset by SNES
  regs.pc     = 0x000000;
  regs.x.h    = 0x00;
  regs.y.h    = 0x00;
  regs.s.h    = 0x01;
  regs.d      = 0x0000;
  regs.db     = 0x00;
  regs.p      = 0x34;
  regs.e      = 1;
  regs.mdr    = 0x00;
  regs.wai    = false;
  regs.vector = 0xfffc;  //reset vector address
  update_table();

  mmio_reset();
  dma_reset();
  timing_reset();
}

CPU::CPU() {
  PPUcounter::scanline = { &CPU::scanline, this };
}

CPU::~CPU() {
}

}
