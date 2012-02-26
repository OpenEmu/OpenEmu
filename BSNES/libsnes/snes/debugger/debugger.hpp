struct Debugger {
  struct BreakEvent{ enum e{
    None,
    BreakpointHit,
    CPUStep,
    SMPStep,
  } i; } break_event;

  enum { Breakpoints = 8 };
  struct Breakpoint {
    bool enabled;
    unsigned addr;
    signed data;  //-1 = unused
    struct Mode{ enum e{ Exec, Read, Write } i; } mode;
    struct Source{ enum e{ CPUBus, APURAM, VRAM, OAM, CGRAM } i; } source;
    unsigned counter;  //number of times breakpoint has been hit since being set
  } breakpoint[Breakpoints];
  unsigned breakpoint_hit;
  void breakpoint_test(Breakpoint::Source::e source, Breakpoint::Mode::e mode, unsigned addr, uint8 data);

  struct MemorySource{ enum e{ CPUBus, APUBus, APURAM, VRAM, OAM, CGRAM } i; };
  uint8 read(MemorySource::e, unsigned addr);
  void write(MemorySource::e, unsigned addr, uint8 data);

  Debugger();
};

extern Debugger debugger;
