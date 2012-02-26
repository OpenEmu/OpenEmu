alwaysinline uint8_t op_readpc() {
  return op_read(regs.pc++);
}

alwaysinline uint8_t op_readsp() {
  return op_read(0x0100 | ++regs.sp);
}

alwaysinline void op_writesp(uint8_t data) {
  op_write(0x0100 | regs.sp--, data);
}

alwaysinline uint8_t op_readdp(uint8_t addr) {
  return op_read(((unsigned)regs.p.p << 8) + addr);
}

alwaysinline void op_writedp(uint8_t addr, uint8_t data) {
  op_write(((unsigned)regs.p.p << 8) + addr, data);
}
