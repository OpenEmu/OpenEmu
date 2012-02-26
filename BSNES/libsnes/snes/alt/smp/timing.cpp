template<unsigned cycle_frequency>
void SMP::Timer<cycle_frequency>::tick() {
  if(++stage1_ticks < cycle_frequency) return;

  stage1_ticks = 0;
  if(enable == false) return;

  if(++stage2_ticks != target) return;

  stage2_ticks = 0;
  stage3_ticks = (stage3_ticks + 1) & 15;
}

template<unsigned cycle_frequency>
void SMP::Timer<cycle_frequency>::tick(unsigned clocks) {
  stage1_ticks += clocks;
  if(stage1_ticks < cycle_frequency) return;

  stage1_ticks -= cycle_frequency;
  if(enable == false) return;

  if(++stage2_ticks != target) return;

  stage2_ticks = 0;
  stage3_ticks = (stage3_ticks + 1) & 15;
}
