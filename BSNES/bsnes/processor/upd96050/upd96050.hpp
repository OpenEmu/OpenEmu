#ifndef PROCESSOR_UPD96050_HPP
#define PROCESSOR_UPD96050_HPP

namespace Processor {

//NEC uPD7720 (not supported)
//NEC uPD7725
//NEC uPD96050

struct uPD96050 {
  enum class Revision : unsigned { uPD7725, uPD96050 } revision;
  uint24 programROM[16384];
  uint16 dataROM[2048];
  uint16 dataRAM[2048];
  #include "registers.hpp"

  void power();
  void exec();
  void serialize(serializer&);

  void exec_op(uint24 opcode);
  void exec_rt(uint24 opcode);
  void exec_jp(uint24 opcode);
  void exec_ld(uint24 opcode);

  uint8 sr_read();
  void sr_write(uint8 data);

  uint8 dr_read();
  void dr_write(uint8 data);

  uint8 dp_read(uint12 addr);
  void dp_write(uint12 addr, uint8 data);

  string disassemble(uint14 ip);
};

}

#endif
