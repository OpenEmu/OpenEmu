#ifdef SSMP_CPP

void sSMP::serialize(serializer &s) {
  SMP::serialize(s);
  SMPcore::core_serialize(s);

  s.integer(status.opcode);
  s.integer(status.in_opcode);
  s.integer(status.clock_counter);
  s.integer(status.dsp_counter);
  s.integer(status.clock_speed);
  s.integer(status.mmio_disabled);
  s.integer(status.ram_writable);
  s.integer(status.iplrom_enabled);
  s.integer(status.dsp_addr);
  s.integer(status.smp_f8);
  s.integer(status.smp_f9);

  s.integer(t0.target);
  s.integer(t0.stage1_ticks);
  s.integer(t0.stage2_ticks);
  s.integer(t0.stage3_ticks);
  s.integer(t0.enabled);

  s.integer(t1.target);
  s.integer(t1.stage1_ticks);
  s.integer(t1.stage2_ticks);
  s.integer(t1.stage3_ticks);
  s.integer(t1.enabled);

  s.integer(t2.target);
  s.integer(t2.stage1_ticks);
  s.integer(t2.stage2_ticks);
  s.integer(t2.stage3_ticks);
  s.integer(t2.enabled);
}

#endif
