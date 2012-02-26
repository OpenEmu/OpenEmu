#include "registers.hpp"
void (CPU::*opcode_table[256])();
void (CPU::*opcode_table_cb[256])();
void initialize_opcode_table();

void op_xx();
void op_cb();

//8-bit load commands
template<unsigned x, unsigned y> void op_ld_r_r();
template<unsigned x> void op_ld_r_n();
template<unsigned x> void op_ld_r_hl();
template<unsigned x> void op_ld_hl_r();
void op_ld_hl_n();
template<unsigned x> void op_ld_a_rr();
void op_ld_a_nn();
template<unsigned x> void op_ld_rr_a();
void op_ld_nn_a();
void op_ld_a_ffn();
void op_ld_ffn_a();
void op_ld_a_ffc();
void op_ld_ffc_a();
void op_ldi_hl_a();
void op_ldi_a_hl();
void op_ldd_hl_a();
void op_ldd_a_hl();

//16-bit load commands
template<unsigned x> void op_ld_rr_nn();
void op_ld_nn_sp();
void op_ld_sp_hl();
template<unsigned x> void op_push_rr();
template<unsigned x> void op_pop_rr();

//8-bit arithmetic commands
void opi_add_a(uint8 x);
template<unsigned x> void op_add_a_r();
void op_add_a_n();
void op_add_a_hl();

void opi_adc_a(uint8 x);
template<unsigned x> void op_adc_a_r();
void op_adc_a_n();
void op_adc_a_hl();

void opi_sub_a(uint8 x);
template<unsigned x> void op_sub_a_r();
void op_sub_a_n();
void op_sub_a_hl();

void opi_sbc_a(uint8 x);
template<unsigned x> void op_sbc_a_r();
void op_sbc_a_n();
void op_sbc_a_hl();

void opi_and_a(uint8 x);
template<unsigned x> void op_and_a_r();
void op_and_a_n();
void op_and_a_hl();

void opi_xor_a(uint8 x);
template<unsigned x> void op_xor_a_r();
void op_xor_a_n();
void op_xor_a_hl();

void opi_or_a(uint8 x);
template<unsigned x> void op_or_a_r();
void op_or_a_n();
void op_or_a_hl();

void opi_cp_a(uint8 x);
template<unsigned x> void op_cp_a_r();
void op_cp_a_n();
void op_cp_a_hl();

template<unsigned x> void op_inc_r();
void op_inc_hl();
template<unsigned x> void op_dec_r();
void op_dec_hl();
void op_daa();
void op_cpl();

//16-bit arithmetic commands
template<unsigned x> void op_add_hl_rr();
template<unsigned x> void op_inc_rr();
template<unsigned x> void op_dec_rr();
void op_add_sp_n();
void op_ld_hl_sp_n();

//rotate/shift commands
void op_rlca();
void op_rla();
void op_rrca();
void op_rra();
template<unsigned x> void op_rlc_r();
void op_rlc_hl();
template<unsigned x> void op_rl_r();
void op_rl_hl();
template<unsigned x> void op_rrc_r();
void op_rrc_hl();
template<unsigned x> void op_rr_r();
void op_rr_hl();
template<unsigned x> void op_sla_r();
void op_sla_hl();
template<unsigned x> void op_swap_r();
void op_swap_hl();
template<unsigned x> void op_sra_r();
void op_sra_hl();
template<unsigned x> void op_srl_r();
void op_srl_hl();

//single-bit commands
template<unsigned b, unsigned x> void op_bit_n_r();
template<unsigned b> void op_bit_n_hl();
template<unsigned b, unsigned x> void op_set_n_r();
template<unsigned b> void op_set_n_hl();
template<unsigned b, unsigned x> void op_res_n_r();
template<unsigned b> void op_res_n_hl();

//control commands
void op_ccf();
void op_scf();
void op_nop();
void op_halt();
void op_stop();
void op_di();
void op_ei();

//jump commands
void op_jp_nn();
void op_jp_hl();
template<unsigned x, bool y> void op_jp_f_nn();
void op_jr_n();
template<unsigned x, bool y> void op_jr_f_n();
void op_call_nn();
template<unsigned x, bool y> void op_call_f_nn();
void op_ret();
template<unsigned x, bool y> void op_ret_f();
void op_reti();
template<unsigned n> void op_rst_n();

//disassembler.cpp
string disassemble(uint16 pc);
string disassemble_opcode(uint16 pc);
string disassemble_opcode_cb(uint16 pc);
