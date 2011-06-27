alwaysinline uint8_t op_readpc() {
  return op_read((regs.pc.b << 16) + regs.pc.w++);
}

alwaysinline uint8_t op_readstack() {
  regs.e ? regs.s.l++ : regs.s.w++;
  return op_read(regs.s.w);
}

alwaysinline uint8_t op_readstackn() {
  return op_read(++regs.s.w);
}

alwaysinline uint8_t op_readaddr(uint32_t addr) {
  return op_read(addr & 0xffff);
}

alwaysinline uint8_t op_readlong(uint32_t addr) {
  return op_read(addr & 0xffffff);
}

alwaysinline uint8_t op_readdbr(uint32_t addr) {
  return op_read(((regs.db << 16) + addr) & 0xffffff);
}

alwaysinline uint8_t op_readpbr(uint32_t addr) {
  return op_read((regs.pc.b << 16) + (addr & 0xffff));
}

alwaysinline uint8_t op_readdp(uint32_t addr) {
  if(regs.e && regs.d.l == 0x00) {
    return op_read((regs.d & 0xff00) + ((regs.d + (addr & 0xffff)) & 0xff));
  } else {
    return op_read((regs.d + (addr & 0xffff)) & 0xffff);
  }
}

alwaysinline uint8_t op_readsp(uint32_t addr) {
  return op_read((regs.s + (addr & 0xffff)) & 0xffff);
}

alwaysinline void op_writestack(uint8_t data) {
  op_write(regs.s.w, data);
  regs.e ? regs.s.l-- : regs.s.w--;
}

alwaysinline void op_writestackn(uint8_t data) {
  op_write(regs.s.w--, data);
}

alwaysinline void op_writeaddr(uint32_t addr, uint8_t data) {
  op_write(addr & 0xffff, data);
}

alwaysinline void op_writelong(uint32_t addr, uint8_t data) {
  op_write(addr & 0xffffff, data);
}

alwaysinline void op_writedbr(uint32_t addr, uint8_t data) {
  op_write(((regs.db << 16) + addr) & 0xffffff, data);
}

alwaysinline void op_writepbr(uint32_t addr, uint8_t data) {
  op_write((regs.pc.b << 16) + (addr & 0xffff), data);
}

alwaysinline void op_writedp(uint32_t addr, uint8_t data) {
  if(regs.e && regs.d.l == 0x00) {
    op_write((regs.d & 0xff00) + ((regs.d + (addr & 0xffff)) & 0xff), data);
  } else {
    op_write((regs.d + (addr & 0xffff)) & 0xffff, data);
  }
}

alwaysinline void op_writesp(uint32_t addr, uint8_t data) {
  op_write((regs.s + (addr & 0xffff)) & 0xffff, data);
}
