struct Coprocessor : Processor {
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_cpu();
};

#include <snes/chip/nss/nss.hpp>
#include <snes/chip/icd2/icd2.hpp>
#include <snes/chip/superfx/superfx.hpp>
#include <snes/chip/sa1/sa1.hpp>
#include <snes/chip/necdsp/necdsp.hpp>
#include <snes/chip/hitachidsp/hitachidsp.hpp>
#include <snes/chip/bsx/bsx.hpp>
#include <snes/chip/srtc/srtc.hpp>
#include <snes/chip/sdd1/sdd1.hpp>
#include <snes/chip/spc7110/spc7110.hpp>
#include <snes/chip/obc1/obc1.hpp>
#include <snes/chip/st0018/st0018.hpp>
#include <snes/chip/sufamiturbo/sufamiturbo.hpp>
#include <snes/chip/msu1/msu1.hpp>
#include <snes/chip/link/link.hpp>

void Coprocessor::step(unsigned clocks) {
  clock += clocks * (uint64)cpu.frequency;
}

void Coprocessor::synchronize_cpu() {
  if(clock >= 0 && scheduler.sync.i != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
}
