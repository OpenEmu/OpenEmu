#include <snes/snes.hpp>

#define SUPERFX_CPP
namespace SNES {

#include "serialization.cpp"
#include "bus/bus.cpp"
#include "core/core.cpp"
#include "memory/memory.cpp"
#include "mmio/mmio.cpp"
#include "timing/timing.cpp"
#include "disasm/disasm.cpp"

SuperFX superfx;

void SuperFX::Enter() { superfx.enter(); }

void SuperFX::enter() {
  while(true) {
    if(scheduler.sync.i == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(regs.sfr.g == 0) {
      add_clocks(6);
      synchronize_cpu();
      continue;
    }

    (this->*opcode_table[(regs.sfr & 0x0300) + peekpipe()])();
    if(r15_modified == false) regs.r[15]++;

    if(++instruction_counter >= 128) {
      instruction_counter = 0;
      synchronize_cpu();
    }
  }
}

void SuperFX::init() {
  initialize_opcode_table();
  regs.r[14].on_modify = bind( &SuperFX::r14_modify, this );
  regs.r[15].on_modify = bind( &SuperFX::r15_modify, this );
}

void SuperFX::load() {
}

void SuperFX::unload() {
}

void SuperFX::power() {
  clockmode = config.superfx.speed;
}

void SuperFX::reset() {
  create(SuperFX::Enter, system.cpu_frequency);
  instruction_counter = 0;

  for(unsigned n = 0; n < 16; n++) regs.r[n] = 0x0000;
  regs.sfr   = 0x0000;
  regs.pbr   = 0x00;
  regs.rombr = 0x00;
  regs.rambr = 0;
  regs.cbr   = 0x0000;
  regs.scbr  = 0x00;
  regs.scmr  = 0x00;
  regs.colr  = 0x00;
  regs.por   = 0x00;
  regs.bramr = 0;
  regs.vcr   = 0x04;
  regs.cfgr  = 0x00;
  regs.clsr  = 0;
  regs.pipeline = 0x01;  //nop
  regs.ramaddr = 0x0000;
  regs.reset();

  memory_reset();
  timing_reset();
}

}
