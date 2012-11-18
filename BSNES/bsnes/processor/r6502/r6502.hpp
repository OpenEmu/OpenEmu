#ifndef PROCESSOR_R6502_HPP
#define PROCESSOR_R6502_HPP

namespace Processor {

//Ricoh 6502
//* Ricoh 2A03
//* Ricoh 2A07

struct R6502 {
  #include "registers.hpp"

  virtual uint8 op_read(uint16 addr) = 0;
  virtual void op_write(uint16 addr, uint8 data) = 0;
  virtual void last_cycle() = 0;
  virtual void nmi(uint16 &vector) = 0;
  virtual uint8 debugger_read(uint16 addr) { return 0u; }

  uint8 mdr() const;
  void power();
  void reset();
  void interrupt();
  void exec();

  void serialize(serializer&);

  //memory.cpp
  uint8 op_readpc();
  uint8 op_readpci();
  uint8 op_readsp();
  uint8 op_readzp(uint8 addr);

  void op_writesp(uint8 data);
  void op_writezp(uint8 addr, uint8 data);

  void op_page(uint16 x, uint16 y);
  void op_page_always(uint16 x, uint16 y);

  //instructions.cpp
  void opf_asl();
  void opf_adc();
  void opf_and();
  void opf_bit();
  void opf_cmp();
  void opf_cpx();
  void opf_cpy();
  void opf_dec();
  void opf_eor();
  void opf_inc();
  void opf_lda();
  void opf_ldx();
  void opf_ldy();
  void opf_lsr();
  void opf_ora();
  void opf_rla();
  void opf_rol();
  void opf_ror();
  void opf_rra();
  void opf_sbc();
  void opf_sla();
  void opf_sra();

  void opi_branch(bool condition);
  void opi_clear_flag(bool &flag);
  void opi_decrement(uint8 &r);
  void opi_increment(uint8 &r);
  void opi_pull(uint8 &r);
  void opi_push(uint8 &r);
  template<void (R6502::*op)()> void opi_read_absolute();
  template<void (R6502::*op)()> void opi_read_absolute_x();
  template<void (R6502::*op)()> void opi_read_absolute_y();
  template<void (R6502::*op)()> void opi_read_immediate();
  template<void (R6502::*op)()> void opi_read_indirect_zero_page_x();
  template<void (R6502::*op)()> void opi_read_indirect_zero_page_y();
  template<void (R6502::*op)()> void opi_read_zero_page();
  template<void (R6502::*op)()> void opi_read_zero_page_x();
  template<void (R6502::*op)()> void opi_read_zero_page_y();
  template<void (R6502::*op)()> void opi_rmw_absolute();
  template<void (R6502::*op)()> void opi_rmw_absolute_x();
  template<void (R6502::*op)()> void opi_rmw_zero_page();
  template<void (R6502::*op)()> void opi_rmw_zero_page_x();
  void opi_set_flag(bool &flag);
  template<void (R6502::*op)()> void opi_shift();
  void opi_store_absolute(uint8 &r);
  void opi_store_absolute_x(uint8 &r);
  void opi_store_absolute_y(uint8 &r);
  void opi_store_indirect_zero_page_x(uint8 &r);
  void opi_store_indirect_zero_page_y(uint8 &r);
  void opi_store_zero_page(uint8 &r);
  void opi_store_zero_page_x(uint8 &r);
  void opi_store_zero_page_y(uint8 &r);
  void opi_transfer(uint8 &s, uint8 &d, bool flag);

  void op_brk();
  void op_jmp_absolute();
  void op_jmp_indirect_absolute();
  void op_jsr_absolute();
  void op_nop();
  void op_php();
  void op_plp();
  void op_rti();
  void op_rts();

  void opill_arr_immediate();
  void opill_nop_absolute();
  void opill_nop_absolute_x();
  void opill_nop_immediate();
  void opill_nop_implied();
  void opill_nop_zero_page();
  void opill_nop_zero_page_x();

  //disassembler.cpp
  string disassemble();
};

}

#endif
