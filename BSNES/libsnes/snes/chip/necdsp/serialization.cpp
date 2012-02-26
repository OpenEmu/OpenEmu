#ifdef NECDSP_CPP

void NECDSP::serialize(serializer &s) {
  Processor::serialize(s);

  s.array(dataRAM);

  s.array(regs.stack);
  s.integer(regs.pc);
  s.integer(regs.rp);
  s.integer(regs.dp);
  s.integer(regs.sp);

  s.integer(regs.k);
  s.integer(regs.l);
  s.integer(regs.m);
  s.integer(regs.n);
  s.integer(regs.a);
  s.integer(regs.b);

  s.integer(regs.flaga.s1);
  s.integer(regs.flaga.s0);
  s.integer(regs.flaga.c);
  s.integer(regs.flaga.z);
  s.integer(regs.flaga.ov1);
  s.integer(regs.flaga.ov0);

  s.integer(regs.flagb.s1);
  s.integer(regs.flagb.s0);
  s.integer(regs.flagb.c);
  s.integer(regs.flagb.z);
  s.integer(regs.flagb.ov1);
  s.integer(regs.flagb.ov0);

  s.integer(regs.tr);
  s.integer(regs.trb);

  s.integer(regs.sr.rqm);
  s.integer(regs.sr.usf1);
  s.integer(regs.sr.usf0);
  s.integer(regs.sr.drs);
  s.integer(regs.sr.dma);
  s.integer(regs.sr.drc);
  s.integer(regs.sr.soc);
  s.integer(regs.sr.sic);
  s.integer(regs.sr.ei);
  s.integer(regs.sr.p1);
  s.integer(regs.sr.p0);

  s.integer(regs.dr);
  s.integer(regs.si);
  s.integer(regs.so);
}

#endif
