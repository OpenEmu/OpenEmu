#ifdef SMPCORE_CPP

void SMPcore::core_serialize(serializer &s) {
  s.integer(regs.pc);
  s.integer(regs.a);
  s.integer(regs.x);
  s.integer(regs.y);
  s.integer(regs.sp);
  s.integer(regs.p.n);
  s.integer(regs.p.v);
  s.integer(regs.p.p);
  s.integer(regs.p.b);
  s.integer(regs.p.h);
  s.integer(regs.p.i);
  s.integer(regs.p.z);
  s.integer(regs.p.c);

  s.integer(dp);
  s.integer(sp);
  s.integer(rd);
  s.integer(wr);
  s.integer(bit);
  s.integer(ya);
}

#endif
