#ifdef SSMP_CPP

void sSMP::add_clocks(unsigned clocks) {
  scheduler.addclocks_smp(clocks);
  #if !defined(DSP_STATE_MACHINE)
  scheduler.sync_smpdsp();
  #else
  while(scheduler.clock.smpdsp < 0) dsp.enter();
  #endif
}

void sSMP::tick_timers() {
  t0.tick();
  t1.tick();
  t2.tick();

  //forcefully sync S-SMP to S-CPU in case chips are not communicating
  //sync if S-SMP is more than 24 samples ahead of S-CPU
  if(scheduler.clock.cpusmp > +(768 * 24 * (int64)24000000)) scheduler.sync_smpcpu();
}

#endif
