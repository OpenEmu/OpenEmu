#ifdef SMP_CPP

void SMP::add_clocks(unsigned clocks) {
  step(clocks);
  synchronize_dsp();

  #if defined(DEBUGGER)
  synchronize_cpu();
  #else
  //forcefully sync S-SMP to S-CPU in case chips are not communicating
  //sync if S-SMP is more than 24 samples ahead of S-CPU
  if(clock > +(768 * 24 * (int64)24000000)) synchronize_cpu();
  #endif
}

void SMP::cycle_edge() {
  timer0.tick();
  timer1.tick();
  timer2.tick();

  //TEST register S-SMP speed control
  //24 clocks have already been added for this cycle at this point
  switch(status.clock_speed) {
    case 0: break;                       //100% speed
    case 1: add_clocks(24); break;       // 50% speed
    case 2: while(true) add_clocks(24);  //  0% speed -- locks S-SMP
    case 3: add_clocks(24 * 9); break;   // 10% speed
  }
}

template<unsigned timer_frequency>
void SMP::Timer<timer_frequency>::tick() {
  //stage 0 increment
  stage0_ticks += smp.status.timer_step;
  if(stage0_ticks < timer_frequency) return;
  stage0_ticks -= timer_frequency;

  //stage 1 increment
  stage1_ticks ^= 1;
  synchronize_stage1();
}

template<unsigned timer_frequency>
void SMP::Timer<timer_frequency>::synchronize_stage1() {
  bool new_line = stage1_ticks;
  if(smp.status.timers_enable == false) new_line = false;
  if(smp.status.timers_disable == true) new_line = false;

  bool old_line = current_line;
  current_line = new_line;
  if(old_line != 1 || new_line != 0) return;  //only pulse on 1->0 transition

  //stage 2 increment
  if(enable == false) return;
  if(++stage2_ticks != target) return;

  //stage 3 increment
  stage2_ticks = 0;
  stage3_ticks++;
}

#endif
