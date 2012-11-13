#include <sfc/sfc.hpp>

#define HITACHIDSP_CPP
namespace SuperFamicom {

#include "memory.cpp"
#include "serialization.cpp"
HitachiDSP hitachidsp;

void HitachiDSP::Enter() { hitachidsp.enter(); }

void HitachiDSP::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(mmio.dma) {
      for(unsigned n = 0; n < mmio.dma_length; n++) {
        bus.write(mmio.dma_target + n, bus.read(mmio.dma_source + n));
        step(2);
      }
      mmio.dma = false;
    }

    exec(mmio.program_offset);
    step(1);

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
  mmio.dma = false;

  mmio.dma_source = 0x000000;
  mmio.dma_length = 0x0000;
  mmio.dma_target = 0x000000;
  mmio.r1f48 = 0x00;
  mmio.program_offset = 0x000000;
  mmio.r1f4c = 0x00;
  mmio.page_number = 0x0000;
  mmio.program_counter = 0x00;
  mmio.r1f50 = 0x33;
  mmio.r1f51 = 0x00;
  mmio.r1f52 = 0x01;
}

void HitachiDSP::reset() {
  create(HitachiDSP::Enter, frequency);
  HG51B::power();
}

}
