  enum {
    EventNone,
    EventIrqLockRelease,
    EventAluLockRelease,
    EventDramRefresh,
    EventHdmaInit,
    EventHdmaRun,

    //cycle edge
    EventFlagHdmaInit = 1 << 0,
    EventFlagHdmaRun  = 1 << 1,
  };
  unsigned cycle_edge_state;

  //timing.cpp
  unsigned dma_counter();

  void add_clocks(unsigned clocks);
  void scanline();

  alwaysinline void cycle_edge();
  alwaysinline void last_cycle();

  void timing_power();
  void timing_reset();

  //irq.cpp
  alwaysinline void poll_interrupts();
  void nmitimen_update(uint8 data);
  bool rdnmi();
  bool timeup();

  alwaysinline bool nmi_test();
  alwaysinline bool irq_test();

  //joypad.cpp
  void run_auto_joypad_poll();

  //event.cpp
  void queue_event(unsigned);  //priorityqueue callback function
