void R6502::serialize(serializer &s) {
  s.integer(regs.mdr);
  s.integer(regs.pc);
  s.integer(regs.a);
  s.integer(regs.x);
  s.integer(regs.y);
  s.integer(regs.s);
  s.integer(regs.p.n);
  s.integer(regs.p.v);
  s.integer(regs.p.d);
  s.integer(regs.p.i);
  s.integer(regs.p.z);
  s.integer(regs.p.c);

  s.integer(abs.w);
  s.integer(iabs.w);
  s.integer(rd);
  s.integer(zp);
  s.integer(aa);
}
