#include <sfc/sfc.hpp>

#define CPU_CPP
namespace SuperFamicom {

CPU cpu;

#include "serialization.cpp"
#include "dma.cpp"
#include "memory.cpp"
#include "mmio.cpp"
#include "timing.cpp"

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

    if(status.nmi_pending) {
      status.nmi_pending = false;
      regs.vector = (regs.e == false ? 0xffea : 0xfffa);
      op_irq();
    }

    if(status.irq_pending) {
      status.irq_pending = false;
      regs.vector = (regs.e == false ? 0xffee : 0xfffe);
      op_irq();
    }

    op_step();
  }
}

alwaysinline void CPU::op_step() {
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
  regs.a = 0x0000;
  regs.x = 0x0000;
  regs.y = 0x0000;
  regs.s = 0x01ff;

  reset();
}

void CPU::reset() {
  create(Enter, system.cpu_frequency());
  coprocessors.reset();
  PPUcounter::reset();

  regs.pc = 0x000000;
  regs.x.h = 0x00;
  regs.y.h = 0x00;
  regs.s.h = 0x01;
  regs.d = 0x0000;
  regs.db = 0x00;
  regs.p = 0x34;
  regs.e = 1;
  regs.mdr = 0x00;
  regs.wai = false;
  update_table();

  regs.pc.l = bus.read(0xfffc);
  regs.pc.h = bus.read(0xfffd);
  regs.pc.b = 0x00;

  status.nmi_valid = false;
  status.nmi_line = false;
  status.nmi_transition = false;
  status.nmi_pending = false;

  status.irq_valid = false;
  status.irq_line = false;
  status.irq_transition = false;
  status.irq_pending = false;

  status.irq_lock = false;
  status.hdma_pending = false;

  status.wram_addr = 0x000000;

  status.joypad_strobe_latch = 0;

  status.nmi_enabled = false;
  status.virq_enabled = false;
  status.hirq_enabled = false;
  status.auto_joypad_poll_enabled = false;

  status.pio = 0xff;

  status.htime = 0x0000;
  status.vtime = 0x0000;

  status.rom_speed = 8;

  status.joy1l = status.joy1h = 0x00;
  status.joy2l = status.joy2h = 0x00;
  status.joy3l = status.joy3h = 0x00;
  status.joy4l = status.joy4h = 0x00;

  dma_reset();
}

CPU::CPU() : queue(512, { &CPU::queue_event, this }) {
  PPUcounter::scanline = { &CPU::scanline, this };
}

CPU::~CPU() {
}

}
