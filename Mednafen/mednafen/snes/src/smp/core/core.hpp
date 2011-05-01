class SMPcore {
public:
  #include "registers.hpp"
  #include "memory.hpp"
  #include "disassembler/disassembler.hpp"

  regs_t regs;
  uint16 dp, sp, rd, wr, bit, ya;

  virtual void op_io() = 0;
  virtual uint8 op_read(uint16 addr) = 0;
  virtual void op_write(uint16 addr, uint8 data) = 0;

  uint8  op_adc (uint8  x, uint8  y);
  uint16 op_addw(uint16 x, uint16 y);
  uint8  op_and (uint8  x, uint8  y);
  uint8  op_cmp (uint8  x, uint8  y);
  uint16 op_cmpw(uint16 x, uint16 y);
  uint8  op_eor (uint8  x, uint8  y);
  uint8  op_inc (uint8  x);
  uint8  op_dec (uint8  x);
  uint8  op_or  (uint8  x, uint8  y);
  uint8  op_sbc (uint8  x, uint8  y);
  uint16 op_subw(uint16 x, uint16 y);
  uint8  op_asl (uint8  x);
  uint8  op_lsr (uint8  x);
  uint8  op_rol (uint8  x);
  uint8  op_ror (uint8  x);

  template<int, int> void op_mov_reg_reg();
  void op_mov_sp_x();
  template<int> void op_mov_reg_const();
  void op_mov_a_ix();
  void op_mov_a_ixinc();
  template<int> void op_mov_reg_dp();
  template<int, int> void op_mov_reg_dpr();
  template<int> void op_mov_reg_addr();
  template<int> void op_mov_a_addrr();
  void op_mov_a_idpx();
  void op_mov_a_idpy();
  void op_mov_dp_dp();
  void op_mov_dp_const();
  void op_mov_ix_a();
  void op_mov_ixinc_a();
  template<int> void op_mov_dp_reg();
  template<int, int> void op_mov_dpr_reg();
  template<int> void op_mov_addr_reg();
  template<int> void op_mov_addrr_a();
  void op_mov_idpx_a();
  void op_mov_idpy_a();
  void op_movw_ya_dp();
  void op_movw_dp_ya();
  void op_mov1_c_bit();
  void op_mov1_bit_c();

  void op_bra();
  template<int, int> void op_branch();
  template<int, int> void op_bitbranch();
  void op_cbne_dp();
  void op_cbne_dpx();
  void op_dbnz_dp();
  void op_dbnz_y();
  void op_jmp_addr();
  void op_jmp_iaddrx();
  void op_call();
  void op_pcall();
  template<int> void op_tcall();
  void op_brk();
  void op_ret();
  void op_reti();

  template<uint8 (SMPcore::*)(uint8, uint8), int> void op_read_reg_const();
  template<uint8 (SMPcore::*)(uint8, uint8)> void op_read_a_ix();
  template<uint8 (SMPcore::*)(uint8, uint8), int> void op_read_reg_dp();
  template<uint8 (SMPcore::*)(uint8, uint8)> void op_read_a_dpx();
  template<uint8 (SMPcore::*)(uint8, uint8), int> void op_read_reg_addr();
  template<uint8 (SMPcore::*)(uint8, uint8), int> void op_read_a_addrr();
  template<uint8 (SMPcore::*)(uint8, uint8)> void op_read_a_idpx();
  template<uint8 (SMPcore::*)(uint8, uint8)> void op_read_a_idpy();
  template<uint8 (SMPcore::*)(uint8, uint8)> void op_read_ix_iy();
  template<uint8 (SMPcore::*)(uint8, uint8)> void op_read_dp_dp();
  template<uint8 (SMPcore::*)(uint8, uint8)> void op_read_dp_const();
  template<uint16 (SMPcore::*)(uint16, uint16)> void op_read_ya_dp();
  void op_cmpw_ya_dp();
  template<int> void op_and1_bit();
  void op_eor1_bit();
  void op_not1_bit();
  template<int> void op_or1_bit();

  template<uint8 (SMPcore::*)(uint8), int> void op_adjust_reg();
  template<uint8 (SMPcore::*)(uint8)> void op_adjust_dp();
  template<uint8 (SMPcore::*)(uint8)> void op_adjust_dpx();
  template<uint8 (SMPcore::*)(uint8)> void op_adjust_addr();
  template<int> void op_adjust_addr_a();
  template<int> void op_adjustw_dp();

  void op_nop();
  void op_wait();
  void op_xcn();
  void op_daa();
  void op_das();
  template<int, int> void op_setbit();
  void op_notc();
  template<int> void op_seti();
  template<int, int> void op_setbit_dp();
  template<int> void op_push_reg();
  void op_push_p();
  template<int> void op_pop_reg();
  void op_pop_p();
  void op_mul_ya();
  void op_div_ya_x();

  void (SMPcore::*opcode_table[256])();
  void initialize_opcode_table();

  void core_serialize(serializer&);
  SMPcore();
};
