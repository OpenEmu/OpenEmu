#include <snes/snes.hpp>

#define SMPCORE_CPP
namespace SNES {

#include "serialization.cpp"
#include "algorithms.cpp"
#include "disassembler/disassembler.cpp"

#define A  0
#define X  1
#define Y  2
#define SP 3

#include "opcode_mov.cpp"
#include "opcode_pc.cpp"
#include "opcode_read.cpp"
#include "opcode_rmw.cpp"
#include "opcode_misc.cpp"
#include "table.cpp"

#undef A
#undef X
#undef Y
#undef SP

void SMPcore::op_step() {
  (this->*opcode_table[op_readpc()])();
}

SMPcore::SMPcore() {
  initialize_opcode_table();
}

}
