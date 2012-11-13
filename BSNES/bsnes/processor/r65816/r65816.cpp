#include <processor/processor.hpp>
#include "r65816.hpp"

namespace Processor {

#include "algorithms.cpp"
#include "disassembler.cpp"
#include "serialization.cpp"

#define L last_cycle();
#define A 0
#define X 1
#define Y 2
#define Z 3
#define S 4
#define D 5
#define call(op) (this->*op)()

#include "opcode_read.cpp"
#include "opcode_write.cpp"
#include "opcode_rmw.cpp"
#include "opcode_pc.cpp"
#include "opcode_misc.cpp"
#include "table.cpp"

#undef L
#undef A
#undef X
#undef Y
#undef Z
#undef S
#undef D
#undef call

//immediate, 2-cycle opcodes with I/O cycle will become bus read
//when an IRQ is to be triggered immediately after opcode completion.
//this affects the following opcodes:
//  clc, cld, cli, clv, sec, sed, sei,
//  tax, tay, txa, txy, tya, tyx,
//  tcd, tcs, tdc, tsc, tsx, txs,
//  inc, inx, iny, dec, dex, dey,
//  asl, lsr, rol, ror, nop, xce.
alwaysinline void R65816::op_io_irq() {
  if(interrupt_pending()) {
    //modify I/O cycle to bus read cycle, do not increment PC
    op_read(regs.pc.d);
  } else {
    op_io();
  }
}

alwaysinline void R65816::op_io_cond2() {
  if(regs.d.l != 0x00) {
    op_io();
  }
}

alwaysinline void R65816::op_io_cond4(uint16 x, uint16 y) {
  if(!regs.p.x || (x & 0xff00) != (y & 0xff00)) {
    op_io();
  }
}

alwaysinline void R65816::op_io_cond6(uint16 addr) {
  if(regs.e && (regs.pc.w & 0xff00) != (addr & 0xff00)) {
    op_io();
  }
}

void R65816::op_irq() {
  op_read(regs.pc.d);
  op_io();
  if(!regs.e) op_writestack(regs.pc.b);
  op_writestack(regs.pc.h);
  op_writestack(regs.pc.l);
  op_writestack(regs.e ? (regs.p & ~0x10) : regs.p);
  rd.l = op_read(regs.vector + 0);
  regs.pc.b = 0x00;
  regs.p.i  = 1;
  regs.p.d  = 0;
  rd.h = op_read(regs.vector + 1);
  regs.pc.w = rd.w;
}

R65816::R65816() {
  initialize_opcode_table();
}

}
