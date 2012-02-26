#include <snes/snes.hpp>

#define HITACHIDSP_CPP
namespace SNES {

#include "memory.cpp"
#include "opcodes.cpp"
#include "registers.cpp"
#include "serialization.cpp"
HitachiDSP hitachidsp;

void HitachiDSP::Enter() { hitachidsp.enter(); }

void HitachiDSP::enter() {
  while(true) {
    if(scheduler.sync.i == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    switch(state.i) {
    case State::Idle:
      step(1);
      break;
    case State::DMA:
      for(unsigned n = 0; n < regs.dma_length; n++) {
        bus.write(regs.dma_target + n, bus.read(regs.dma_source + n));
        step(2);
      }
      state.i = State::Idle;
      break;
    case State::Execute:
      unsigned offset = regs.program_offset + regs.pc * 2;
      opcode  = bus_read(offset + 0) << 0;
      opcode |= bus_read(offset + 1) << 8;
      regs.pc = (regs.pc & 0xffff00) | ((regs.pc + 1) & 0x0000ff);
      exec();
      step(1);
      break;
    }

    synchronize_cpu();
  }
}

void HitachiDSP::init() {
}

void HitachiDSP::load() {
}

void HitachiDSP::unload() {
}

void HitachiDSP::power() {
}

void HitachiDSP::reset() {
  create(HitachiDSP::Enter, frequency);
  state.i = State::Idle;

  regs.n = 0;
  regs.z = 0;
  regs.c = 0;

  regs.dma_source = 0x000000;
  regs.dma_length = 0x0000;
  regs.dma_target = 0x000000;
  regs.r1f48 = 0x00;
  regs.program_offset = 0x000000;
  regs.r1f4c = 0x00;
  regs.page_number = 0x0000;
  regs.program_counter = 0x00;
  regs.r1f50 = 0x33;
  regs.r1f51 = 0x00;
  regs.r1f52 = 0x01;
}

}
