struct SMPcore {
  virtual void op_io() = 0;
  virtual uint8 op_read(uint16 addr) = 0;
  virtual void op_write(uint16 addr, uint8 data) = 0;
  void op_step();

  #include "registers.hpp"
  #include "memory.hpp"

  regs_t regs;
  word_t dp, sp, rd, wr, bit, ya;
  uint8 opcode;

  void core_serialize(serializer&);
  string disassemble_opcode(uint16 addr);

protected:
  uint8 op_adc(uint8, uint8);
  uint8 op_and(uint8, uint8);
  uint8 op_asl(uint8);
  uint8 op_cmp(uint8, uint8);
  uint8 op_dec(uint8);
  uint8 op_eor(uint8, uint8);
  uint8 op_inc(uint8);
  uint8 op_ld (uint8, uint8);
  uint8 op_lsr(uint8);
  uint8 op_or (uint8, uint8);
  uint8 op_rol(uint8);
  uint8 op_ror(uint8);
  uint8 op_sbc(uint8, uint8);
  uint8 op_st (uint8, uint8);
  uint16 op_adw(uint16, uint16);
  uint16 op_cpw(uint16, uint16);
  uint16 op_ldw(uint16, uint16);
  uint16 op_sbw(uint16, uint16);

  template<uint8 (SMPcore::*op)(uint8)> void op_adjust(uint8&);
  template<uint8 (SMPcore::*op)(uint8)> void op_adjust_addr();
  template<uint8 (SMPcore::*op)(uint8)> void op_adjust_dp();
  void op_adjust_dpw(signed);
  template<uint8 (SMPcore::*op)(uint8)> void op_adjust_dpx();
  void op_branch(bool);
  void op_branch_bit();
  void op_pull(uint8&);
  void op_push(uint8);
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_addr(uint8&);
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_addri(uint8&);
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_const(uint8&);
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_dp(uint8&);
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_dpi(uint8&, uint8&);
  template<uint16 (SMPcore::*op)(uint16, uint16)> void op_read_dpw();
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_idpx();
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_idpy();
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_read_ix();
  void op_set_addr_bit();
  void op_set_bit();
  void op_set_flag(bool&, bool);
  void op_test_addr(bool);
  void op_transfer(uint8&, uint8&);
  void op_write_addr(uint8&);
  void op_write_addri(uint8&);
  void op_write_dp(uint8&);
  void op_write_dpi(uint8&, uint8&);
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_write_dp_const();
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_write_dp_dp();
  template<uint8 (SMPcore::*op)(uint8, uint8)> void op_write_ix_iy();

  void op_bne_dp();
  void op_bne_dpdec();
  void op_bne_dpx();
  void op_bne_ydec();
  void op_brk();
  void op_clv();
  void op_cmc();
  void op_daa();
  void op_das();
  void op_div_ya_x();
  void op_jmp_addr();
  void op_jmp_iaddrx();
  void op_jsp_dp();
  void op_jsr_addr();
  void op_jst();
  void op_lda_ixinc();
  void op_mul_ya();
  void op_nop();
  void op_plp();
  void op_rti();
  void op_rts();
  void op_sta_idpx();
  void op_sta_idpy();
  void op_sta_ix();
  void op_sta_ixinc();
  void op_stw_dp();
  void op_wait();
  void op_xcn();
};
