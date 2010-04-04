class CPUcore {
public:
  #include "registers.hpp"
  #include "memory.hpp"
  #include "disasm/disasm.hpp"

  regs_t regs;
  reg24_t aa, rd;
  uint8_t sp, dp;

  virtual void op_io() = 0;
  virtual uint8_t op_read(uint32_t addr) = 0;
  virtual void op_write(uint32_t addr, uint8_t data) = 0;
  virtual void last_cycle() = 0;
  virtual bool interrupt_pending() = 0;

  void op_io_irq();
  void op_io_cond2();
  void op_io_cond4(uint16 x, uint16 y);
  void op_io_cond6(uint16 addr);

  void op_adc_b();
  void op_adc_w();
  void op_and_b();
  void op_and_w();
  void op_bit_b();
  void op_bit_w();
  void op_cmp_b();
  void op_cmp_w();
  void op_cpx_b();
  void op_cpx_w();
  void op_cpy_b();
  void op_cpy_w();
  void op_eor_b();
  void op_eor_w();
  void op_lda_b();
  void op_lda_w();
  void op_ldx_b();
  void op_ldx_w();
  void op_ldy_b();
  void op_ldy_w();
  void op_ora_b();
  void op_ora_w();
  void op_sbc_b();
  void op_sbc_w();

  void op_inc_b();
  void op_inc_w();
  void op_dec_b();
  void op_dec_w();
  void op_asl_b();
  void op_asl_w();
  void op_lsr_b();
  void op_lsr_w();
  void op_rol_b();
  void op_rol_w();
  void op_ror_b();
  void op_ror_w();
  void op_trb_b();
  void op_trb_w();
  void op_tsb_b();
  void op_tsb_w();

  template<void (CPUcore::*)()> void op_read_const_b();
  template<void (CPUcore::*)()> void op_read_const_w();
  void op_read_bit_const_b();
  void op_read_bit_const_w();
  template<void (CPUcore::*)()> void op_read_addr_b();
  template<void (CPUcore::*)()> void op_read_addr_w();
  template<void (CPUcore::*)()> void op_read_addrx_b();
  template<void (CPUcore::*)()> void op_read_addrx_w();
  template<void (CPUcore::*)()> void op_read_addry_b();
  template<void (CPUcore::*)()> void op_read_addry_w();
  template<void (CPUcore::*)()> void op_read_long_b();
  template<void (CPUcore::*)()> void op_read_long_w();
  template<void (CPUcore::*)()> void op_read_longx_b();
  template<void (CPUcore::*)()> void op_read_longx_w();
  template<void (CPUcore::*)()> void op_read_dp_b();
  template<void (CPUcore::*)()> void op_read_dp_w();
  template<void (CPUcore::*)(), int> void op_read_dpr_b();
  template<void (CPUcore::*)(), int> void op_read_dpr_w();
  template<void (CPUcore::*)()> void op_read_idp_b();
  template<void (CPUcore::*)()> void op_read_idp_w();
  template<void (CPUcore::*)()> void op_read_idpx_b();
  template<void (CPUcore::*)()> void op_read_idpx_w();
  template<void (CPUcore::*)()> void op_read_idpy_b();
  template<void (CPUcore::*)()> void op_read_idpy_w();
  template<void (CPUcore::*)()> void op_read_ildp_b();
  template<void (CPUcore::*)()> void op_read_ildp_w();
  template<void (CPUcore::*)()> void op_read_ildpy_b();
  template<void (CPUcore::*)()> void op_read_ildpy_w();
  template<void (CPUcore::*)()> void op_read_sr_b();
  template<void (CPUcore::*)()> void op_read_sr_w();
  template<void (CPUcore::*)()> void op_read_isry_b();
  template<void (CPUcore::*)()> void op_read_isry_w();

  template<int> void op_write_addr_b();
  template<int> void op_write_addr_w();
  template<int, int> void op_write_addrr_b();
  template<int, int> void op_write_addrr_w();
  template<int> void op_write_longr_b();
  template<int> void op_write_longr_w();
  template<int> void op_write_dp_b();
  template<int> void op_write_dp_w();
  template<int, int> void op_write_dpr_b();
  template<int, int> void op_write_dpr_w();
  void op_sta_idp_b();
  void op_sta_idp_w();
  void op_sta_ildp_b();
  void op_sta_ildp_w();
  void op_sta_idpx_b();
  void op_sta_idpx_w();
  void op_sta_idpy_b();
  void op_sta_idpy_w();
  void op_sta_ildpy_b();
  void op_sta_ildpy_w();
  void op_sta_sr_b();
  void op_sta_sr_w();
  void op_sta_isry_b();
  void op_sta_isry_w();

  template<int, int> void op_adjust_imm_b();
  template<int, int> void op_adjust_imm_w();
  void op_asl_imm_b();
  void op_asl_imm_w();
  void op_lsr_imm_b();
  void op_lsr_imm_w();
  void op_rol_imm_b();
  void op_rol_imm_w();
  void op_ror_imm_b();
  void op_ror_imm_w();
  template<void (CPUcore::*)()> void op_adjust_addr_b();
  template<void (CPUcore::*)()> void op_adjust_addr_w();
  template<void (CPUcore::*)()> void op_adjust_addrx_b();
  template<void (CPUcore::*)()> void op_adjust_addrx_w();
  template<void (CPUcore::*)()> void op_adjust_dp_b();
  template<void (CPUcore::*)()> void op_adjust_dp_w();
  template<void (CPUcore::*)()> void op_adjust_dpx_b();
  template<void (CPUcore::*)()> void op_adjust_dpx_w();

  template<int, int> void op_branch();
  void op_bra();
  void op_brl();
  void op_jmp_addr();
  void op_jmp_long();
  void op_jmp_iaddr();
  void op_jmp_iaddrx();
  void op_jmp_iladdr();
  void op_jsr_addr();
  void op_jsr_long_e();
  void op_jsr_long_n();
  void op_jsr_iaddrx_e();
  void op_jsr_iaddrx_n();
  void op_rti_e();
  void op_rti_n();
  void op_rts();
  void op_rtl_e();
  void op_rtl_n();

  void op_nop();
  void op_wdm();
  void op_xba();
  template<int> void op_move_b();
  template<int> void op_move_w();
  template<int, int> void op_interrupt_e();
  template<int, int> void op_interrupt_n();
  void op_stp();
  void op_wai();
  void op_xce();
  template<int, int> void op_flag();
  template<int> void op_pflag_e();
  template<int> void op_pflag_n();
  template<int, int> void op_transfer_b();
  template<int, int> void op_transfer_w();
  void op_tcs_e();
  void op_tcs_n();
  void op_tsc_e();
  void op_tsc_n();
  void op_tsx_b();
  void op_tsx_w();
  void op_txs_e();
  void op_txs_n();
  template<int> void op_push_b();
  template<int> void op_push_w();
  void op_phd_e();
  void op_phd_n();
  void op_phb();
  void op_phk();
  void op_php();
  template<int> void op_pull_b();
  template<int> void op_pull_w();
  void op_pld_e();
  void op_pld_n();
  void op_plb();
  void op_plp_e();
  void op_plp_n();
  void op_pea_e();
  void op_pea_n();
  void op_pei_e();
  void op_pei_n();
  void op_per_e();
  void op_per_n();

  void (CPUcore::**opcode_table)();
  void (CPUcore::*op_table[256 * 5])();
  void initialize_opcode_table();
  void update_table();

  enum {
    table_EM =    0,  // 8-bit accumulator,  8-bit index (emulation mode)
    table_MX =  256,  // 8-bit accumulator,  8-bit index
    table_Mx =  512,  // 8-bit accumulator, 16-bit index
    table_mX =  768,  //16-bit accumulator,  8-bit index
    table_mx = 1024,  //16-bit accumulator, 16-bit index
  };

  void core_serialize(serializer&);
  CPUcore();
};
