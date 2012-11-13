#ifdef SMP_CPP

void SMP::serialize(serializer &s) {
  SPC700::serialize(s);
  Thread::serialize(s);

  s.array(apuram);

  s.integer(status.clock_counter);
  s.integer(status.dsp_counter);
  s.integer(status.timer_step);

  s.integer(status.clock_speed);
  s.integer(status.timer_speed);
  s.integer(status.timers_enable);
  s.integer(status.ram_disable);
  s.integer(status.ram_writable);
  s.integer(status.timers_disable);

  s.integer(status.iplrom_enable);

  s.integer(status.dsp_addr);

  s.integer(status.ram00f8);
  s.integer(status.ram00f9);

  s.integer(timer0.stage0_ticks);
  s.integer(timer0.stage1_ticks);
  s.integer(timer0.stage2_ticks);
  s.integer(timer0.stage3_ticks);
  s.integer(timer0.current_line);
  s.integer(timer0.enable);
  s.integer(timer0.target);

  s.integer(timer1.stage0_ticks);
  s.integer(timer1.stage1_ticks);
  s.integer(timer1.stage2_ticks);
  s.integer(timer1.stage3_ticks);
  s.integer(timer1.current_line);
  s.integer(timer1.enable);
  s.integer(timer1.target);

  s.integer(timer2.stage0_ticks);
  s.integer(timer2.stage1_ticks);
  s.integer(timer2.stage2_ticks);
  s.integer(timer2.stage3_ticks);
  s.integer(timer2.current_line);
  s.integer(timer2.enable);
  s.integer(timer2.target);
}

#endif
