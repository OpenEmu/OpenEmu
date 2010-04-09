#include <../base.hpp>

#define CPU_CPP
namespace SNES {

void CPU::power() {
  cpu_version = config.cpu.version;
}

void CPU::reset() {
  PPUcounter::reset();
}

void CPU::serialize(serializer &s) {
  PPUcounter::serialize(s);
  s.integer(cpu_version);
}

CPU::CPU() {
}

CPU::~CPU() {
}

};
