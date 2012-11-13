struct Coprocessor : Thread {
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_cpu();
};

#include <sfc/chip/icd2/icd2.hpp>
#include <sfc/chip/nss/nss.hpp>
#include <sfc/chip/superfx/superfx.hpp>
#include <sfc/chip/sa1/sa1.hpp>
#include <sfc/chip/necdsp/necdsp.hpp>
#include <sfc/chip/hitachidsp/hitachidsp.hpp>
#include <sfc/chip/armdsp/armdsp.hpp>
#include <sfc/chip/bsx/bsx.hpp>
#include <sfc/chip/srtc/srtc.hpp>
#include <sfc/chip/sdd1/sdd1.hpp>
#include <sfc/chip/spc7110/spc7110.hpp>
#include <sfc/chip/obc1/obc1.hpp>
#include <sfc/chip/sufamiturbo/sufamiturbo.hpp>
#include <sfc/chip/msu1/msu1.hpp>
#include <sfc/chip/link/link.hpp>

void Coprocessor::step(unsigned clocks) {
  clock += clocks * (uint64)cpu.frequency;
}

void Coprocessor::synchronize_cpu() {
  if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
}
