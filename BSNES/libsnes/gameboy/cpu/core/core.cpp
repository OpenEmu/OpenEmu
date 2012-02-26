#ifdef CPU_CPP

#include "table.cpp"

void CPU::op_xx() {
}

void CPU::op_cb() {
  uint8 opcode = op_read(r[PC]++);
  (this->*opcode_table_cb[opcode])();
}

//8-bit load commands

template<unsigned x, unsigned y> void CPU::op_ld_r_r() {
  r[x] = r[y];
}

template<unsigned x> void CPU::op_ld_r_n() {
  r[x] = op_read(r[PC]++);
}

template<unsigned x> void CPU::op_ld_r_hl() {
  r[x] = op_read(r[HL]);
}

template<unsigned x> void CPU::op_ld_hl_r() {
  op_write(r[HL], r[x]);
}

void CPU::op_ld_hl_n() {
  op_write(r[HL], op_read(r[PC]++));
}

template<unsigned x> void CPU::op_ld_a_rr() {
  r[A] = op_read(r[x]);
}

void CPU::op_ld_a_nn() {
  uint8 lo = op_read(r[PC]++);
  uint8 hi = op_read(r[PC]++);
  r[A] = op_read((hi << 8) | (lo << 0));
}

template<unsigned x> void CPU::op_ld_rr_a() {
  op_write(r[x], r[A]);
}

void CPU::op_ld_nn_a() {
  uint8 lo = op_read(r[PC]++);
  uint8 hi = op_read(r[PC]++);
  op_write((hi << 8) | (lo << 0), r[A]);
}

void CPU::op_ld_a_ffn() {
  r[A] = op_read(0xff00 + op_read(r[PC]++));
}

void CPU::op_ld_ffn_a() {
  op_write(0xff00 + op_read(r[PC]++), r[A]);
}

void CPU::op_ld_a_ffc() {
  r[A] = op_read(0xff00 + r[C]);
}

void CPU::op_ld_ffc_a() {
  op_write(0xff00 + r[C], r[A]);
}

void CPU::op_ldi_hl_a() {
  op_write(r[HL], r[A]);
  r[HL]++;
}

void CPU::op_ldi_a_hl() {
  r[A] = op_read(r[HL]);
  r[HL]++;
}

void CPU::op_ldd_hl_a() {
  op_write(r[HL], r[A]);
  r[HL]--;
}

void CPU::op_ldd_a_hl() {
  r[A] = op_read(r[HL]);
  r[HL]--;
}

//16-bit load commands

template<unsigned x> void CPU::op_ld_rr_nn() {
  r[x]  = op_read(r[PC]++) << 0;
  r[x] |= op_read(r[PC]++) << 8;
}

void CPU::op_ld_nn_sp() {
  uint16 addr = op_read(r[PC]++) << 0;
  addr |= op_read(r[PC]++) << 8;
  op_write(addr + 0, r[SP] >> 0);
  op_write(addr + 1, r[SP] >> 8);
}

void CPU::op_ld_sp_hl() {
  r[SP] = r[HL];
  op_io();
}

template<unsigned x> void CPU::op_push_rr() {
  op_write(--r[SP], r[x] >> 8);
  op_write(--r[SP], r[x] >> 0);
  op_io();
}

template<unsigned x> void CPU::op_pop_rr() {
  r[x]  = op_read(r[SP]++) << 0;
  r[x] |= op_read(r[SP]++) << 8;
}

//8-bit arithmetic commands

void CPU::opi_add_a(uint8 x) {
  uint16 rh = r[A] + x;
  uint16 rl = (r[A] & 0x0f) + (x & 0x0f);
  r[A] = rh;
  r.f.z = (uint8)rh == 0;
  r.f.n = 0;
  r.f.h = rl > 0x0f;
  r.f.c = rh > 0xff;
}

template<unsigned x> void CPU::op_add_a_r() { opi_add_a(r[x]); }
void CPU::op_add_a_n() { opi_add_a(op_read(r[PC]++)); }
void CPU::op_add_a_hl() { opi_add_a(op_read(r[HL])); }

void CPU::opi_adc_a(uint8 x) {
  uint16 rh = r[A] + x + r.f.c;
  uint16 rl = (r[A] & 0x0f) + (x & 0x0f) + r.f.c;
  r[A] = rh;
  r.f.z = (uint8)rh == 0;
  r.f.n = 0;
  r.f.h = rl > 0x0f;
  r.f.c = rh > 0xff;
}

template<unsigned x> void CPU::op_adc_a_r() { opi_adc_a(r[x]); }
void CPU::op_adc_a_n() { opi_adc_a(op_read(r[PC]++)); }
void CPU::op_adc_a_hl() { opi_adc_a(op_read(r[HL])); }

void CPU::opi_sub_a(uint8 x) {
  uint16 rh = r[A] - x;
  uint16 rl = (r[A] & 0x0f) - (x & 0x0f);
  r[A] = rh;
  r.f.z = (uint8)rh == 0;
  r.f.n = 1;
  r.f.h = rl > 0x0f;
  r.f.c = rh > 0xff;
}

template<unsigned x> void CPU::op_sub_a_r() { opi_sub_a(r[x]); }
void CPU::op_sub_a_n() { opi_sub_a(op_read(r[PC]++)); }
void CPU::op_sub_a_hl() { opi_sub_a(op_read(r[HL])); }

void CPU::opi_sbc_a(uint8 x) {
  uint16 rh = r[A] - x - r.f.c;
  uint16 rl = (r[A] & 0x0f) - (x & 0x0f) - r.f.c;
  r[A] = rh;
  r.f.z = (uint8)rh == 0;
  r.f.n = 1;
  r.f.h = rl > 0x0f;
  r.f.c = rh > 0xff;
}

template<unsigned x> void CPU::op_sbc_a_r() { opi_sbc_a(r[x]); }
void CPU::op_sbc_a_n() { opi_sbc_a(op_read(r[PC]++)); }
void CPU::op_sbc_a_hl() { opi_sbc_a(op_read(r[HL])); }

void CPU::opi_and_a(uint8 x) {
  r[A] &= x;
  r.f.z = r[A] == 0;
  r.f.n = 0;
  r.f.h = 1;
  r.f.c = 0;
}

template<unsigned x> void CPU::op_and_a_r() { opi_and_a(r[x]); }
void CPU::op_and_a_n() { opi_and_a(op_read(r[PC]++)); }
void CPU::op_and_a_hl() { opi_and_a(op_read(r[HL])); }

void CPU::opi_xor_a(uint8 x) {
  r[A] ^= x;
  r.f.z = r[A] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = 0;
}

template<unsigned x> void CPU::op_xor_a_r() { opi_xor_a(r[x]); }
void CPU::op_xor_a_n() { opi_xor_a(op_read(r[PC]++)); }
void CPU::op_xor_a_hl() { opi_xor_a(op_read(r[HL])); }

void CPU::opi_or_a(uint8 x) {
  r[A] |= x;
  r.f.z = r[A] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = 0;
}

template<unsigned x> void CPU::op_or_a_r() { opi_or_a(r[x]); }
void CPU::op_or_a_n() { opi_or_a(op_read(r[PC]++)); }
void CPU::op_or_a_hl() { opi_or_a(op_read(r[HL])); }

void CPU::opi_cp_a(uint8 x) {
  uint16 rh = r[A] - x;
  uint16 rl = (r[A] & 0x0f) - (x & 0x0f);
  r.f.z = (uint8)rh == 0;
  r.f.n = 1;
  r.f.h = rl > 0x0f;
  r.f.c = rh > 0xff;
}

template<unsigned x> void CPU::op_cp_a_r() { opi_cp_a(r[x]); }
void CPU::op_cp_a_n() { opi_cp_a(op_read(r[PC]++)); }
void CPU::op_cp_a_hl() { opi_cp_a(op_read(r[HL])); }

template<unsigned x> void CPU::op_inc_r() {
  r[x]++;
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = (r[x] & 0x0f) == 0x00;
}

void CPU::op_inc_hl() {
  uint8 n = op_read(r[HL]);
  op_write(r[HL], ++n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = (n & 0x0f) == 0x00;
}

template<unsigned x> void CPU::op_dec_r() {
  r[x]--;
  r.f.z = r[x] == 0;
  r.f.n = 1;
  r.f.h = (r[x] & 0x0f) == 0x0f;
}

void CPU::op_dec_hl() {
  uint8 n = op_read(r[HL]);
  op_write(r[HL], --n);
  r.f.z = n == 0;
  r.f.n = 1;
  r.f.h = (n & 0x0f) == 0x0f;
}

void CPU::op_daa() {
  uint16 a = r[A];
  if(r.f.n == 0) {
    if(r.f.h || (a & 0x0f) > 0x09) a += 0x06;
    if(r.f.c || (a       ) > 0x9f) a += 0x60;
  } else {
    if(r.f.h) {
      a -= 0x06;
      if(r.f.c == 0) a &= 0xff;
    }
    if(r.f.c) a -= 0x60;
  }
  r[A] = a;
  r.f.z = r[A] == 0;
  r.f.h = 0;
  r.f.c |= a & 0x100;
}

void CPU::op_cpl() {
  r[A] ^= 0xff;
  r.f.n = 1;
  r.f.h = 1;
}

//16-bit arithmetic commands

template<unsigned x> void CPU::op_add_hl_rr() {
  op_io();
  uint32 rb = (r[HL] + r[x]);
  uint32 rn = (r[HL] & 0xfff) + (r[x] & 0xfff);
  r[HL] = rb;
  r.f.n = 0;
  r.f.h = rn > 0x0fff;
  r.f.c = rb > 0xffff;
}

template<unsigned x> void CPU::op_inc_rr() {
  op_io();
  r[x]++;
}

template<unsigned x> void CPU::op_dec_rr() {
  op_io();
  r[x]--;
}

void CPU::op_add_sp_n() {
  op_io();
  op_io();
  signed n = (int8)op_read(r[PC]++);
  r.f.z = 0;
  r.f.n = 0;
  r.f.h = ((r[SP] & 0x0f) + (n & 0x0f)) > 0x0f;
  r.f.c = ((r[SP] & 0xff) + (n & 0xff)) > 0xff;
  r[SP] += n;
}

void CPU::op_ld_hl_sp_n() {
  op_io();
  signed n = (int8)op_read(r[PC]++);
  r.f.z = 0;
  r.f.n = 0;
  r.f.h = ((r[SP] & 0x0f) + (n & 0x0f)) > 0x0f;
  r.f.c = ((r[SP] & 0xff) + (n & 0xff)) > 0xff;
  r[HL] = r[SP] + n;
}

//rotate/shift commands

void CPU::op_rlca() {
  r[A] = (r[A] << 1) | (r[A] >> 7);
  r.f.z = 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = r[A] & 0x01;
}

void CPU::op_rla() {
  bool c = r[A] & 0x80;
  r[A] = (r[A] << 1) | (r.f.c << 0);
  r.f.z = 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

void CPU::op_rrca() {
  r[A] = (r[A] >> 1) | (r[A] << 7);
  r.f.z = 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = r[A] & 0x80;
}

void CPU::op_rra() {
  bool c = r[A] & 0x01;
  r[A] = (r[A] >> 1) | (r.f.c << 7);
  r.f.z = 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

template<unsigned x> void CPU::op_rlc_r() {
  r[x] = (r[x] << 1) | (r[x] >> 7);
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = r[x] & 0x01;
}

void CPU::op_rlc_hl() {
  uint8 n = op_read(r[HL]);
  n = (n << 1) | (n >> 7);
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = n & 0x01;
}

template<unsigned x> void CPU::op_rl_r() {
  bool c = r[x] & 0x80;
  r[x] = (r[x] << 1) | (r.f.c << 0);
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

void CPU::op_rl_hl() {
  uint8 n = op_read(r[HL]);
  bool c = n & 0x80;
  n = (n << 1) | (r.f.c << 0);
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

template<unsigned x> void CPU::op_rrc_r() {
  r[x] = (r[x] >> 1) | (r[x] << 7);
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = r[x] & 0x80;
}

void CPU::op_rrc_hl() {
  uint8 n = op_read(r[HL]);
  n = (n >> 1) | (n << 7);
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = n & 0x80;
}

template<unsigned x> void CPU::op_rr_r() {
  bool c = r[x] & 0x01;
  r[x] = (r[x] >> 1) | (r.f.c << 7);
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

void CPU::op_rr_hl() {
  uint8 n = op_read(r[HL]);
  bool c = n & 0x01;
  n = (n >> 1) | (r.f.c << 7);
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

template<unsigned x> void CPU::op_sla_r() {
  bool c = r[x] & 0x80;
  r[x] <<= 1;
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

void CPU::op_sla_hl() {
  uint8 n = op_read(r[HL]);
  bool c = n & 0x80;
  n <<= 1;
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

template<unsigned x> void CPU::op_swap_r() {
  r[x] = (r[x] << 4) | (r[x] >> 4);
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = 0;
}

void CPU::op_swap_hl() {
  uint8 n = op_read(r[HL]);
  n = (n << 4) | (n >> 4);
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = 0;
}

template<unsigned x> void CPU::op_sra_r() {
  bool c = r[x] & 0x01;
  r[x] = (int8)r[x] >> 1;
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

void CPU::op_sra_hl() {
  uint8 n = op_read(r[HL]);
  bool c = n & 0x01;
  n = (int8)n >> 1;
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

template<unsigned x> void CPU::op_srl_r() {
  bool c = r[x] & 0x01;
  r[x] >>= 1;
  r.f.z = r[x] == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

void CPU::op_srl_hl() {
  uint8 n = op_read(r[HL]);
  bool c = n & 0x01;
  n >>= 1;
  op_write(r[HL], n);
  r.f.z = n == 0;
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = c;
}

//single-bit commands

template<unsigned b, unsigned x> void CPU::op_bit_n_r() {
  r.f.z = (r[x] & (1 << b)) == 0;
  r.f.n = 0;
  r.f.h = 1;
}

template<unsigned b> void CPU::op_bit_n_hl() {
  uint8 n = op_read(r[HL]);
  r.f.z = (n & (1 << b)) == 0;
  r.f.n = 0;
  r.f.h = 1;
}

template<unsigned b, unsigned x> void CPU::op_set_n_r() {
  r[x] |= 1 << b;
}

template<unsigned b> void CPU::op_set_n_hl() {
  uint8 n = op_read(r[HL]);
  n |= 1 << b;
  op_write(r[HL], n);
}

template<unsigned b, unsigned x> void CPU::op_res_n_r() {
  r[x] &= ~(1 << b);
}

template<unsigned b> void CPU::op_res_n_hl() {
  uint8 n = op_read(r[HL]);
  n &= ~(1 << b);
  op_write(r[HL], n);
}

//control commands

void CPU::op_ccf() {
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = !r.f.c;
}

void CPU::op_scf() {
  r.f.n = 0;
  r.f.h = 0;
  r.f.c = 1;
}

void CPU::op_nop() {
}

void CPU::op_halt() {
  status.halt = true;
  while(status.halt == true) op_io();
}

void CPU::op_stop() {
  if(status.speed_switch) {
    status.speed_switch = 0;
    status.speed_double ^= 1;
    frequency = 4 * 1024 * 1024;
    if(status.speed_double) frequency *= 2;
    return;
  }
  status.stop = true;
  while(status.stop == true) op_io();
}

void CPU::op_di() {
  status.ime = 0;
}

void CPU::op_ei() {
  status.ei = true;
//status.ime = 1;
}

//jump commands

void CPU::op_jp_nn() {
  uint8 lo = op_read(r[PC]++);
  uint8 hi = op_read(r[PC]++);
  r[PC] = (hi << 8) | (lo << 0);
  op_io();
}

void CPU::op_jp_hl() {
  r[PC] = r[HL];
}

template<unsigned x, bool y> void CPU::op_jp_f_nn() {
  uint8 lo = op_read(r[PC]++);
  uint8 hi = op_read(r[PC]++);
  if(r.f[x] == y) {
    r[PC] = (hi << 8) | (lo << 0);
    op_io();
  }
}

void CPU::op_jr_n() {
  int8 n = op_read(r[PC]++);
  r[PC] += n;
  op_io();
}

template<unsigned x, bool y> void CPU::op_jr_f_n() {
  int8 n = op_read(r[PC]++);
  if(r.f[x] == y) {
    r[PC] += n;
    op_io();
  }
}

void CPU::op_call_nn() {
  uint8 lo = op_read(r[PC]++);
  uint8 hi = op_read(r[PC]++);
  op_write(--r[SP], r[PC] >> 8);
  op_write(--r[SP], r[PC] >> 0);
  r[PC] = (hi << 8) | (lo << 0);
  op_io();
}

template<unsigned x, bool y> void CPU::op_call_f_nn() {
  uint8 lo = op_read(r[PC]++);
  uint8 hi = op_read(r[PC]++);
  if(r.f[x] == y) {
    op_write(--r[SP], r[PC] >> 8);
    op_write(--r[SP], r[PC] >> 0);
    r[PC] = (hi << 8) | (lo << 0);
    op_io();
  }
}

void CPU::op_ret() {
  uint8 lo = op_read(r[SP]++);
  uint8 hi = op_read(r[SP]++);
  r[PC] = (hi << 8) | (lo << 0);
  op_io();
}

template<unsigned x, bool y> void CPU::op_ret_f() {
  op_io();
  if(r.f[x] == y) {
    uint8 lo = op_read(r[SP]++);
    uint8 hi = op_read(r[SP]++);
    r[PC] = (hi << 8) | (lo << 0);
    op_io();
  }
}

void CPU::op_reti() {
  uint8 lo = op_read(r[SP]++);
  uint8 hi = op_read(r[SP]++);
  r[PC] = (hi << 8) | (lo << 0);
  op_io();
  status.ime = 1;
}

template<unsigned n> void CPU::op_rst_n() {
  op_write(--r[SP], r[PC] >> 8);
  op_write(--r[SP], r[PC] >> 0);
  r[PC] = n;
  op_io();
}

#endif
