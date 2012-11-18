void SPC700::serialize(serializer &s) {
  s.integer(regs.pc);
  s.integer(regs.a);
  s.integer(regs.x);
  s.integer(regs.y);
  s.integer(regs.s);
  s.integer(regs.p.n);
  s.integer(regs.p.v);
  s.integer(regs.p.p);
  s.integer(regs.p.b);
  s.integer(regs.p.h);
  s.integer(regs.p.i);
  s.integer(regs.p.z);
  s.integer(regs.p.c);

  s.integer(opcode);
  s.integer(dp.w);
  s.integer(sp.w);
  s.integer(rd.w);
  s.integer(wr.w);
  s.integer(bit.w);
  s.integer(ya.w);
}
