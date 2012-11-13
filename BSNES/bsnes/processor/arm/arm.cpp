#include <processor/processor.hpp>
#include "arm.hpp"

namespace Processor {

#include "registers.cpp"
#include "algorithms.cpp"
#include "instructions-arm.cpp"
#include "instructions-thumb.cpp"
#include "disassembler.cpp"
#include "serialization.cpp"

void ARM::power() {
  processor.power();
  vector(0x00000000, Processor::Mode::SVC);
  pipeline.reload = true;
  crash = false;
  r(15).modify = [&] {
    pipeline.reload = true;
  };

  trace = false;
  instructions = 0;
}

void ARM::exec() {
  cpsr().t ? thumb_step() : arm_step();
}

void ARM::idle() {
  bus_idle(r(15));
}

uint32 ARM::read(uint32 addr, uint32 size) {
  uint32 word = bus_read(addr, size);
  sequential() = true;
  return word;
}

uint32 ARM::load(uint32 addr, uint32 size) {
  sequential() = false;
  uint32 word = read(addr, size);

  if(size == Half) { word &= 0xffff; word |= word << 16; }
  if(size == Byte) { word &= 0xff; word |= word << 8; word |= word << 16; }

  word = ror(word, 8 * (addr & 3));
  idle();

  if(size == Half) word &= 0xffff;
  if(size == Byte) word &= 0xff;
  return word;
}

void ARM::write(uint32 addr, uint32 size, uint32 word) {
  bus_write(addr, size, word);
  sequential() = true;
}

void ARM::store(uint32 addr, uint32 size, uint32 word) {
  if(size == Half) { word &= 0xffff; word |= word << 16; }
  if(size == Byte) { word &= 0xff; word |= word << 8; word |= word << 16; }

  sequential() = false;
  write(addr, size, word);
  sequential() = false;
}

void ARM::vector(uint32 addr, Processor::Mode mode) {
  auto psr = cpsr();
  processor.setMode(mode);
  spsr() = psr;
  cpsr().i = 1;
  cpsr().f |= mode == Processor::Mode::FIQ;
  cpsr().t = 0;
  r(14) = pipeline.decode.address;
  r(15) = addr;
}

}
