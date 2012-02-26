#ifdef HITACHIDSP_CPP

void HitachiDSP::serialize(serializer &s) {
  Processor::serialize(s);

  s.array(dataRAM);
  foreach(n, stack) s.integer(n);
  s.integer(opcode);

  unsigned state_ = (unsigned)state.i;
  s.integer(state_);
  state.i = (State::e)state_;

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
  foreach(n, regs.gpr) s.integer(n);

  s.integer(regs.dma_source);
  s.integer(regs.dma_length);
  s.integer(regs.dma_target);
  s.integer(regs.r1f48);
  s.integer(regs.program_offset);
  s.integer(regs.r1f4c);
  s.integer(regs.page_number);
  s.integer(regs.program_counter);
  s.integer(regs.r1f50);
  s.integer(regs.r1f51);
  s.integer(regs.r1f52);
  s.array(regs.vector);
}

#endif
