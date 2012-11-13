#include <sfc/sfc.hpp>

#define NECDSP_CPP
namespace SuperFamicom {

#include "serialization.cpp"
NECDSP necdsp;

void NECDSP::Enter() { necdsp.enter(); }

void NECDSP::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    exec();
    step(1);
    synchronize_cpu();
  }
}

uint8 NECDSP::sr_read(unsigned) { cpu.synchronize_coprocessors(); return uPD96050::sr_read(); }
void NECDSP::sr_write(unsigned, uint8 data) { cpu.synchronize_coprocessors(); return uPD96050::sr_write(data); }

uint8 NECDSP::dr_read(unsigned) { cpu.synchronize_coprocessors(); return uPD96050::dr_read(); }
void NECDSP::dr_write(unsigned, uint8 data) { cpu.synchronize_coprocessors(); return uPD96050::dr_write(data); }

uint8 NECDSP::dp_read(unsigned addr) { cpu.synchronize_coprocessors(); return uPD96050::dp_read(addr); }
void NECDSP::dp_write(unsigned addr, uint8 data) { cpu.synchronize_coprocessors(); return uPD96050::dp_write(addr, data); }

void NECDSP::init() {
}

void NECDSP::load() {
  if(revision == Revision::uPD96050) {
    interface->memory.append({ID::NecDSPRAM, "upd96050.ram"});
  }
}

void NECDSP::unload() {
}

void NECDSP::power() {
}

void NECDSP::reset() {
  create(NECDSP::Enter, frequency);
  uPD96050::power();
}

}
