#ifndef PROCESSOR_HG51B_HPP
#define PROCESSOR_HG51B_HPP

namespace Processor {

//Hitachi HG51B169 (HG51BS family/derivative?)

struct HG51B {
//uint16 programROM[2][256];
  uint24 dataROM[1024];
  uint8  dataRAM[3072];
  #include "registers.hpp"
  void exec(uint24 addr);
  virtual uint8 bus_read(uint24 addr) = 0;
  virtual void bus_write(uint24 addr, uint8 data) = 0;

  void power();
  void serialize(serializer&);

protected:
  void push();
  void pull();
  unsigned sa();
  unsigned ri();
  unsigned np();
  void instruction();
};

}

#endif
