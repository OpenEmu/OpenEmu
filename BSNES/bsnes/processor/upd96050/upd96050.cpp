#include <processor/processor.hpp>
#include "upd96050.hpp"

namespace Processor {

#include "instructions.cpp"
#include "memory.cpp"
#include "disassembler.cpp"
#include "serialization.cpp"

void uPD96050::power() {
  if(revision == Revision::uPD7725) {
    regs.pc.bits(11);
    regs.rp.bits(10);
    regs.dp.bits( 8);
  }

  if(revision == Revision::uPD96050) {
    regs.pc.bits(14);
    regs.rp.bits(11);
    regs.dp.bits(11);
  }

  for(unsigned n = 0; n < 16; n++) regs.stack[n] = 0x0000;
  regs.pc = 0x0000;
  regs.rp = 0x0000;
  regs.dp = 0x0000;
  regs.sp = 0x0;
  regs.k = 0x0000;
  regs.l = 0x0000;
  regs.m = 0x0000;
  regs.n = 0x0000;
  regs.a = 0x0000;
  regs.b = 0x0000;
  regs.flaga = 0x00;
  regs.flagb = 0x00;
  regs.tr = 0x0000;
  regs.trb = 0x0000;
  regs.sr = 0x0000;
  regs.dr = 0x0000;
  regs.si = 0x0000;
  regs.so = 0x0000;
}

}
