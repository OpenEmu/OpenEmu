alwaysinline uint8 op_readpc() {
  return op_read(regs.pc++);
}

alwaysinline uint8 op_readsp() {
  return op_read(0x0100 | ++regs.s);
}

alwaysinline void op_writesp(uint8 data) {
  return op_write(0x0100 | regs.s--, data);
}

alwaysinline uint8 op_readdp(uint8 addr) {
  return op_read((regs.p.p << 8) + addr);
}

alwaysinline void op_writedp(uint8 addr, uint8 data) {
  return op_write((regs.p.p << 8) + addr, data);
}
