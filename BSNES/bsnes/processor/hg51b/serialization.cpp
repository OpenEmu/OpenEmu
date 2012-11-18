#ifdef PROCESSOR_HG51B_HPP

void HG51B::serialize(serializer &s) {
  s.array(dataRAM);
  for(auto &n : stack) s.integer(n);
  s.integer(opcode);

  s.integer(regs.halt);

  s.integer(regs.pc);
  s.integer(regs.p);
  s.integer(regs.n);
  s.integer(regs.z);
  s.integer(regs.c);

  s.integer(regs.a);
  s.integer(regs.acch);
  s.integer(regs.accl);
  s.integer(regs.busdata);
  s.integer(regs.romdata);
  s.integer(regs.ramdata);
  s.integer(regs.busaddr);
  s.integer(regs.ramaddr);
  for(auto &n : regs.gpr) s.integer(n);
}

#endif
