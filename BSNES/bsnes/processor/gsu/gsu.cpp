#include <processor/processor.hpp>
#include "gsu.hpp"

namespace Processor {

#include "instructions.cpp"
#include "table.cpp"
#include "serialization.cpp"

void GSU::power() {
}

void GSU::reset() {
  for(auto &r : regs.r) r = 0x0000;
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
}

}
