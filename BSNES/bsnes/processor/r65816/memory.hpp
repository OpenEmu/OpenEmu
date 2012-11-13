alwaysinline uint8 op_readpc() {
  return op_read((regs.pc.b << 16) + regs.pc.w++);
}

alwaysinline uint8 op_readstack() {
  regs.e ? regs.s.l++ : regs.s.w++;
  return op_read(regs.s.w);
}

alwaysinline uint8 op_readstackn() {
  return op_read(++regs.s.w);
}

alwaysinline uint8 op_readaddr(uint32 addr) {
  return op_read(addr & 0xffff);
}

alwaysinline uint8 op_readlong(uint32 addr) {
  return op_read(addr & 0xffffff);
}

alwaysinline uint8 op_readdbr(uint32 addr) {
  return op_read(((regs.db << 16) + addr) & 0xffffff);
}

alwaysinline uint8 op_readpbr(uint32 addr) {
  return op_read((regs.pc.b << 16) + (addr & 0xffff));
}

alwaysinline uint8 op_readdp(uint32 addr) {
  if(regs.e && regs.d.l == 0x00) {
    return op_read((regs.d & 0xff00) + ((regs.d + (addr & 0xffff)) & 0xff));
  } else {
    return op_read((regs.d + (addr & 0xffff)) & 0xffff);
  }
}

alwaysinline uint8 op_readsp(uint32 addr) {
  return op_read((regs.s + (addr & 0xffff)) & 0xffff);
}

alwaysinline void op_writestack(uint8 data) {
  op_write(regs.s.w, data);
  regs.e ? regs.s.l-- : regs.s.w--;
}

alwaysinline void op_writestackn(uint8 data) {
  op_write(regs.s.w--, data);
}

alwaysinline void op_writeaddr(uint32 addr, uint8 data) {
  op_write(addr & 0xffff, data);
}

alwaysinline void op_writelong(uint32 addr, uint8 data) {
  op_write(addr & 0xffffff, data);
}

alwaysinline void op_writedbr(uint32 addr, uint8 data) {
  op_write(((regs.db << 16) + addr) & 0xffffff, data);
}

alwaysinline void op_writepbr(uint32 addr, uint8 data) {
  op_write((regs.pc.b << 16) + (addr & 0xffff), data);
}

alwaysinline void op_writedp(uint32 addr, uint8 data) {
  if(regs.e && regs.d.l == 0x00) {
    op_write((regs.d & 0xff00) + ((regs.d + (addr & 0xffff)) & 0xff), data);
  } else {
    op_write((regs.d + (addr & 0xffff)) & 0xffff, data);
  }
}

alwaysinline void op_writesp(uint32 addr, uint8 data) {
  op_write((regs.s + (addr & 0xffff)) & 0xffff, data);
}
