#ifndef PROCESSOR_GSU_HPP
#define PROCESSOR_GSU_HPP

namespace Processor {

struct GSU {
  #include "registers.hpp"

  virtual void step(unsigned clocks) = 0;

  virtual void stop() = 0;
  virtual uint8 color(uint8 source) = 0;
  virtual void plot(uint8 x, uint8 y) = 0;
  virtual uint8 rpix(uint8 x, uint8 y) = 0;

  virtual uint8 pipe() = 0;
  virtual void rombuffer_sync() = 0;
  virtual uint8 rombuffer_read() = 0;
  virtual void rambuffer_sync() = 0;
  virtual uint8 rambuffer_read(uint16 addr) = 0;
  virtual void rambuffer_write(uint16 addr, uint8 data) = 0;
  virtual void cache_flush() = 0;

  void power();
  void reset();
  void serialize(serializer&);

  //table.cpp
  void (GSU::*opcode_table[1024])();
  void initialize_opcode_table();

  //instructions.cpp
  template<int> void op_adc_i();
  template<int> void op_adc_r();
  template<int> void op_add_i();
  template<int> void op_add_r();
  void op_alt1();
  void op_alt2();
  void op_alt3();
  template<int> void op_and_i();
  template<int> void op_and_r();
  void op_asr();
  void op_bge();
  void op_bcc();
  void op_bcs();
  void op_beq();
  template<int> void op_bic_i();
  template<int> void op_bic_r();
  void op_blt();
  void op_bmi();
  void op_bne();
  void op_bpl();
  void op_bra();
  void op_bvc();
  void op_bvs();
  void op_cache();
  void op_cmode();
  template<int> void op_cmp_r();
  void op_color();
  template<int> void op_dec_r();
  void op_div2();
  void op_fmult();
  template<int> void op_from_r();
  void op_getb();
  void op_getbl();
  void op_getbh();
  void op_getbs();
  void op_getc();
  void op_hib();
  template<int> void op_ibt_r();
  template<int> void op_inc_r();
  template<int> void op_iwt_r();
  template<int> void op_jmp_r();
  template<int> void op_ldb_ir();
  template<int> void op_ldw_ir();
  template<int> void op_link();
  template<int> void op_ljmp_r();
  template<int> void op_lm_r();
  template<int> void op_lms_r();
  void op_lmult();
  void op_lob();
  void op_loop();
  void op_lsr();
  void op_merge();
  template<int> void op_mult_i();
  template<int> void op_mult_r();
  void op_nop();
  void op_not();
  template<int> void op_or_i();
  template<int> void op_or_r();
  void op_plot();
  void op_ramb();
  void op_rol();
  void op_romb();
  void op_ror();
  void op_rpix();
  template<int> void op_sbc_r();
  void op_sbk();
  void op_sex();
  template<int> void op_sm_r();
  template<int> void op_sms_r();
  template<int> void op_stb_ir();
  void op_stop();
  template<int> void op_stw_ir();
  template<int> void op_sub_i();
  template<int> void op_sub_r();
  void op_swap();
  template<int> void op_to_r();
  template<int> void op_umult_i();
  template<int> void op_umult_r();
  template<int> void op_with_r();
  template<int> void op_xor_i();
  template<int> void op_xor_r();
};

}

#endif
