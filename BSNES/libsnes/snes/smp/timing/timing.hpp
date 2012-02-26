template<unsigned timer_frequency>
class Timer {
public:
  uint8 stage0_ticks;
  uint8 stage1_ticks;
  uint8 stage2_ticks;
  uint4 stage3_ticks;
  bool current_line;
  bool enable;
  uint8 target;

  void tick();
  void synchronize_stage1();
};

Timer<192> timer0;
Timer<192> timer1;
Timer< 24> timer2;

alwaysinline void add_clocks(unsigned clocks);
alwaysinline void cycle_edge();
