#ifndef PROCESSOR_ARM_HPP
#define PROCESSOR_ARM_HPP

namespace Processor {

//ARMv3
//ARMv4TDMI

struct ARM {
  enum : unsigned { Byte = 8, Half = 16, Word = 32 };
  #include "registers.hpp"
  #include "instructions-arm.hpp"
  #include "instructions-thumb.hpp"
  #include "disassembler.hpp"
  virtual void step(unsigned clocks) = 0;
  virtual void bus_idle(uint32 addr) = 0;
  virtual uint32 bus_read(uint32 addr, uint32 size) = 0;
  virtual void bus_write(uint32 addr, uint32 size, uint32 word) = 0;

  void power();
  void exec();
  void idle();
  uint32 read(uint32 addr, uint32 size);
  uint32 load(uint32 addr, uint32 size);
  void write(uint32 addr, uint32 size, uint32 word);
  void store(uint32 addr, uint32 size, uint32 word);
  void vector(uint32 addr, Processor::Mode mode);

  bool condition(uint4 condition);
  uint32 bit(uint32 result);
  uint32 add(uint32 source, uint32 modify, bool carry);
  uint32 sub(uint32 source, uint32 modify, bool carry);
  uint32 mul(uint32 product, uint32 multiplicand, uint32 multiplier);
  uint32 lsl(uint32 source, uint8 shift);
  uint32 lsr(uint32 source, uint8 shift);
  uint32 asr(uint32 source, uint8 shift);
  uint32 ror(uint32 source, uint8 shift);
  uint32 rrx(uint32 source);

  void serialize(serializer&);

  bool trace;
  uintmax_t instructions;
};

}

#endif
