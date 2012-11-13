struct SMP : Processor::SPC700, Thread {
  uint8 iplrom[64];
  uint8 apuram[64 * 1024];

  enum : bool { Threaded = true };
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_cpu();
  alwaysinline void synchronize_dsp();

  uint8 port_read(uint2 port) const;
  void port_write(uint2 port, uint8 data);

  void enter();
  void power();
  void reset();

  void serialize(serializer&);
  SMP();
  ~SMP();

privileged:
  struct {
    //timing
    unsigned clock_counter;
    unsigned dsp_counter;
    unsigned timer_step;

    //$00f0
    uint8 clock_speed;
    uint8 timer_speed;
    bool timers_enable;
    bool ram_disable;
    bool ram_writable;
    bool timers_disable;

    //$00f1
    bool iplrom_enable;

    //$00f2
    uint8 dsp_addr;

    //$00f8,$00f9
    uint8 ram00f8;
    uint8 ram00f9;
  } status;

  static void Enter();

  friend class SMPcore;

  struct Debugger {
    hook<void (uint16)> op_exec;
    hook<void (uint16)> op_read;
    hook<void (uint16, uint8)> op_write;
  } debugger;

  //memory.cpp
  uint8 ram_read(uint16 addr);
  void ram_write(uint16 addr, uint8 data);

  uint8 op_busread(uint16 addr);
  void op_buswrite(uint16 addr, uint8 data);

  void op_io();
  uint8 op_read(uint16 addr);
  void op_write(uint16 addr, uint8 data);

  uint8 disassembler_read(uint16 addr);

  //timing.cpp
  template<unsigned frequency>
  struct Timer {
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
};

extern SMP smp;
