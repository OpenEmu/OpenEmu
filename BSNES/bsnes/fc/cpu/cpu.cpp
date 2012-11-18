#include <fc/fc.hpp>

namespace Famicom {

#include "timing.cpp"
#include "serialization.cpp"
CPU cpu;

void CPU::Enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    cpu.main();
  }
}

void CPU::main() {
  if(status.interrupt_pending) {
    interrupt();
    return;
  }

  exec();
}

void CPU::add_clocks(unsigned clocks) {
  apu.clock -= clocks;
  if(apu.clock < 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(apu.thread);

  ppu.clock -= clocks;
  if(ppu.clock < 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(ppu.thread);

  cartridge.clock -= clocks;
  if(cartridge.clock < 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cartridge.thread);
}

void CPU::power() {
  R6502::power();

  for(unsigned addr = 0; addr < 0x0800; addr++) ram[addr] = 0xff;
  ram[0x0008] = 0xf7;
  ram[0x0009] = 0xef;
  ram[0x000a] = 0xdf;
  ram[0x000f] = 0xbf;
}

void CPU::reset() {
  R6502::reset();
  create(CPU::Enter, 21477272);

  regs.pc  = bus.read(0xfffc) << 0;
  regs.pc |= bus.read(0xfffd) << 8;

  status.interrupt_pending = false;
  status.nmi_pending = false;
  status.nmi_line = 0;
  status.irq_line = 0;
  status.irq_apu_line = 0;

  status.rdy_line = 1;
  status.rdy_addr = { false, 0x0000 };

  status.oam_dma_pending = false;
  status.oam_dma_page = 0x00;

  status.controller_latch = false;
  status.controller_port0 = 0;
  status.controller_port1 = 0;
}

uint8 CPU::debugger_read(uint16 addr) {
  return bus.read(addr);
}

uint8 CPU::ram_read(uint16 addr) {
  return ram[addr & 0x07ff];
}

void CPU::ram_write(uint16 addr, uint8 data) {
  ram[addr & 0x07ff] = data;
}

uint8 CPU::read(uint16 addr) {
  if(addr == 0x4016) {
    return (mdr() & 0xc0) | input.data(0);
  }

  if(addr == 0x4017) {
    return (mdr() & 0xc0) | input.data(1);
  }

  return apu.read(addr);
}

void CPU::write(uint16 addr, uint8 data) {
  if(addr == 0x4014) {
    status.oam_dma_page = data;
    status.oam_dma_pending = true;
  }

  if(addr == 0x4016) {
    input.latch(data & 0x01);
  }

  return apu.write(addr, data);
}

}
