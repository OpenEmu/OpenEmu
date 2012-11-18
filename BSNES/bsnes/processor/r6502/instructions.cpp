//opcode functions
//================

void R6502::opf_adc() {
  signed result = regs.a + rd + regs.p.c;
  regs.p.v = ~(regs.a ^ rd) & (regs.a ^ result) & 0x80;
  regs.p.c = (result > 0xff);
  regs.p.n = (result & 0x80);
  regs.p.z = ((uint8)result == 0);
  regs.a = result;
}

void R6502::opf_and() {
  regs.a &= rd;
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void R6502::opf_asl() {
  regs.p.c = rd & 0x80;
  rd <<= 1;
  regs.p.n = (rd & 0x80);
  regs.p.z = (rd == 0);
}

void R6502::opf_bit() {
  regs.p.n = (rd & 0x80);
  regs.p.v = (rd & 0x40);
  regs.p.z = ((rd & regs.a) == 0);
}

void R6502::opf_cmp() {
  signed r = regs.a - rd;
  regs.p.n = (r & 0x80);
  regs.p.z = (uint8)(r == 0);
  regs.p.c = (r >= 0);
}

void R6502::opf_cpx() {
  signed r = regs.x - rd;
  regs.p.n = (r & 0x80);
  regs.p.z = (uint8)(r == 0);
  regs.p.c = (r >= 0);
}

void R6502::opf_cpy() {
  signed r = regs.y - rd;
  regs.p.n = (r & 0x80);
  regs.p.z = (uint8)(r == 0);
  regs.p.c = (r >= 0);
}

void R6502::opf_dec() {
  rd--;
  regs.p.n = (rd & 0x80);
  regs.p.z = (rd == 0);
}

void R6502::opf_eor() {
  regs.a ^= rd;
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void R6502::opf_inc() {
  rd++;
  regs.p.n = (rd & 0x80);
  regs.p.z = (rd == 0);
}

void R6502::opf_lda() {
  regs.a = rd;
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void R6502::opf_ldx() {
  regs.x = rd;
  regs.p.n = (regs.x & 0x80);
  regs.p.z = (regs.x == 0);
}

void R6502::opf_ldy() {
  regs.y = rd;
  regs.p.n = (regs.y & 0x80);
  regs.p.z = (regs.y == 0);
}

void R6502::opf_lsr() {
  regs.p.c = rd & 0x01;
  rd >>= 1;
  regs.p.n = (rd & 0x80);
  regs.p.z = (rd == 0);
}

void R6502::opf_ora() {
  regs.a |= rd;
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void R6502::opf_rla() {
  unsigned carry = (unsigned)regs.p.c;
  regs.p.c = regs.a & 0x80;
  regs.a = (regs.a << 1) | carry;
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void R6502::opf_rol() {
  unsigned carry = (unsigned)regs.p.c;
  regs.p.c = rd & 0x80;
  rd = (rd << 1) | carry;
  regs.p.n = (rd & 0x80);
  regs.p.z = (rd == 0);
}

void R6502::opf_ror() {
  unsigned carry = (unsigned)regs.p.c << 7;
  regs.p.c = rd & 0x01;
  rd = carry | (rd >> 1);
  regs.p.n = (rd & 0x80);
  regs.p.z = (rd == 0);
}

void R6502::opf_rra() {
  unsigned carry = (unsigned)regs.p.c << 7;
  regs.p.c = regs.a & 0x01;
  regs.a = carry | (regs.a >> 1);
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void R6502::opf_sbc() {
  rd ^= 0xff;
  return opf_adc();
}

void R6502::opf_sla() {
  regs.p.c = regs.a & 0x80;
  regs.a <<= 1;
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

void R6502::opf_sra() {
  regs.p.c = regs.a & 0x01;
  regs.a >>= 1;
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
}

//opcode implementations
//======================

void R6502::opi_branch(bool condition) {
  if(condition == false) {
L   rd = op_readpci();
  } else {
    rd = op_readpci();
    aa = regs.pc + (int8)rd;
    op_page(regs.pc, aa);
L   op_readpc();
    regs.pc = aa;
  }
}

void R6502::opi_clear_flag(bool &flag) {
L op_readpc();
  flag = 0;
}

void R6502::opi_decrement(uint8 &r) {
L op_readpc();
  r--;
  regs.p.n = (r & 0x80);
  regs.p.z = (r == 0);
}

void R6502::opi_increment(uint8 &r) {
L op_readpc();
  r++;
  regs.p.n = (r & 0x80);
  regs.p.z = (r == 0);
}

void R6502::opi_pull(uint8 &r) {
  op_readpc();
  op_readpc();
L r = op_readsp();
  regs.p.n = (r & 0x80);
  regs.p.z = (r == 0);
}

void R6502::opi_push(uint8 &r) {
  op_readpc();
L op_writesp(r);
}

template<void (R6502::*op)()>
void R6502::opi_read_absolute() {
  abs.l = op_readpci();
  abs.h = op_readpci();
L rd = op_read(abs.w);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_absolute_x() {
  abs.l = op_readpci();
  abs.h = op_readpci();
  op_page(abs.w, abs.w + regs.x);
L rd = op_read(abs.w + regs.x);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_absolute_y() {
  abs.l = op_readpci();
  abs.h = op_readpci();
  op_page(abs.w, abs.w + regs.y);
L rd = op_read(abs.w + regs.y);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_immediate() {
L rd = op_readpci();
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_indirect_zero_page_x() {
  zp = op_readpci();
  op_readzp(zp);
  abs.l = op_readzp(zp++ + regs.x);
  abs.h = op_readzp(zp++ + regs.x);
L rd = op_read(abs.w);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_indirect_zero_page_y() {
  rd = op_readpci();
  abs.l = op_readzp(rd++);
  abs.h = op_readzp(rd++);
  op_page(abs.w, abs.w + regs.y);
L rd = op_read(abs.w + regs.y);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_zero_page() {
  zp = op_readpci();
L rd = op_readzp(zp);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_zero_page_x() {
  zp = op_readpci();
  op_readzp(zp);
L rd = op_readzp(zp + regs.x);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_read_zero_page_y() {
  zp = op_readpci();
  op_readzp(zp);
L rd = op_readzp(zp + regs.y);
  call(op);
}

template<void (R6502::*op)()>
void R6502::opi_rmw_absolute() {
  abs.l = op_readpci();
  abs.h = op_readpci();
  rd = op_read(abs.w);
  op_write(abs.w, rd);
  call(op);
L op_write(abs.w, rd);
}

template<void (R6502::*op)()>
void R6502::opi_rmw_absolute_x() {
  abs.l = op_readpci();
  abs.h = op_readpci();
  op_page_always(abs.w, abs.w + regs.x);
  rd = op_read(abs.w + regs.x);
  op_write(abs.w + regs.x, rd);
  call(op);
L op_write(abs.w + regs.x, rd);
}

template<void (R6502::*op)()>
void R6502::opi_rmw_zero_page() {
  zp = op_readpci();
  rd = op_readzp(zp);
  op_writezp(zp, rd);
  call(op);
L op_writezp(zp, rd);
}

template<void (R6502::*op)()>
void R6502::opi_rmw_zero_page_x() {
  zp = op_readpci();
  op_readzp(zp);
  rd = op_readzp(zp + regs.x);
  op_writezp(zp + regs.x, rd);
  call(op);
L op_writezp(zp + regs.x, rd);
}

void R6502::opi_set_flag(bool &flag) {
L op_readpc();
  flag = 1;
}

template<void (R6502::*op)()>
void R6502::opi_shift() {
L op_readpc();
  call(op);
}

void R6502::opi_store_absolute(uint8 &r) {
  abs.l = op_readpci();
  abs.h = op_readpci();
L op_write(abs.w, r);
}

void R6502::opi_store_absolute_x(uint8 &r) {
  abs.l = op_readpci();
  abs.h = op_readpci();
  op_page_always(abs.w, abs.w + regs.x);
L op_write(abs.w + regs.x, r);
}

void R6502::opi_store_absolute_y(uint8 &r) {
  abs.l = op_readpci();
  abs.h = op_readpci();
  op_page_always(abs.w, abs.w + regs.y);
L op_write(abs.w + regs.y, r);
}

void R6502::opi_store_indirect_zero_page_x(uint8 &r) {
  zp = op_readpci();
  op_readzp(zp);
  abs.l = op_readzp(zp++ + regs.x);
  abs.h = op_readzp(zp++ + regs.x);
L op_write(abs.w, r);
}

void R6502::opi_store_indirect_zero_page_y(uint8 &r) {
  rd = op_readpci();
  abs.l = op_readzp(rd++);
  abs.h = op_readzp(rd++);
  op_page_always(abs.w, abs.w + regs.y);
L op_write(abs.w + regs.y, r);
}

void R6502::opi_store_zero_page(uint8 &r) {
  zp = op_readpci();
L op_writezp(zp, r);
}

void R6502::opi_store_zero_page_x(uint8 &r) {
  zp = op_readpci();
  op_readzp(zp);
L op_writezp(zp + regs.x, r);
}

void R6502::opi_store_zero_page_y(uint8 &r) {
  zp = op_readpci();
  op_readzp(zp);
L op_writezp(zp + regs.y, r);
}

void R6502::opi_transfer(uint8 &s, uint8 &d, bool flag) {
L op_readpc();
  d = s;
  if(flag == false) return;
  regs.p.n = (d & 0x80);
  regs.p.z = (d == 0);
}

//opcodes
//=======

void R6502::op_brk() {
  op_readpci();
  op_writesp(regs.pc >> 8);
  op_writesp(regs.pc >> 0);
  op_writesp(regs.p | 0x30);
  abs.l = op_read(0xfffe);
  regs.p.i = 1;
  regs.p.d = 0;
L abs.h = op_read(0xffff);
  regs.pc = abs.w;
}

void R6502::op_jmp_absolute() {
  abs.l = op_readpci();
L abs.h = op_readpci();
  regs.pc = abs.w;
}

void R6502::op_jmp_indirect_absolute() {
  abs.l = op_readpci();
  abs.h = op_readpci();
  iabs.l = op_read(abs.w); abs.l++;
L iabs.h = op_read(abs.w); abs.l++;
  regs.pc = iabs.w;
}

void R6502::op_jsr_absolute() {
  abs.l = op_readpci();
  abs.h = op_readpci();
  op_readpc();
  regs.pc--;
  op_writesp(regs.pc >> 8);
L op_writesp(regs.pc >> 0);
  regs.pc = abs.w;
}

void R6502::op_nop() {
L op_readpc();
}

void R6502::op_php() {
  op_readpc();
L op_writesp(regs.p | 0x30);
}

void R6502::op_plp() {
  op_readpc();
  op_readpc();
L regs.p = op_readsp();
}

void R6502::op_rti() {
  op_readpc();
  op_readpc();
  regs.p = op_readsp();
  abs.l = op_readsp();
L abs.h = op_readsp();
  regs.pc = abs.w;
}

void R6502::op_rts() {
  op_readpc();
  op_readpc();
  abs.l = op_readsp();
  abs.h = op_readsp();
L op_readpc();
  regs.pc = ++abs.w;
}

//illegal opcodes
//===============

void R6502::opill_arr_immediate() {
L rd = op_readpci();
  regs.a &= rd;
  regs.a = (regs.p.c << 7) | (regs.a >> 1);
  regs.p.n = (regs.a & 0x80);
  regs.p.z = (regs.a == 0);
  regs.p.c = (regs.a & 0x40);
  regs.p.v = regs.p.c ^ ((regs.a >> 5) & 1);
}

void R6502::opill_nop_absolute() {
  abs.l = op_readpci();
  abs.h = op_readpci();
L op_readpc();
}

void R6502::opill_nop_absolute_x() {
  abs.l = op_readpci();
  abs.h = op_readpci();
  op_page(abs.w, abs.w + regs.x);
L op_readpc();
}

void R6502::opill_nop_immediate() {
L rd = op_readpc();
}

void R6502::opill_nop_implied() {
L op_readpc();
}

void R6502::opill_nop_zero_page() {
  zp = op_readpci();
L op_readzp(zp);
}

void R6502::opill_nop_zero_page_x() {
  zp = op_readpci();
  op_readzp(zp);
L op_readzp(zp + regs.x);
}
