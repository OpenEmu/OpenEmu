//scheduler thread relationships:
//S-PPU <-> S-CPU <-> cartridge co-processor
//           <|>
//          S-SMP <-> S-DSP

class Scheduler {
public:
  cothread_t thread_snes;
  cothread_t thread_cpu;     //S-CPU (5a22)
  cothread_t thread_cop;     //cartridge co-processor (SuperFX, SA-1, ...)
  cothread_t thread_smp;     //S-SMP (SPC700)
  cothread_t thread_ppu;     //S-PPU
  cothread_t thread_dsp;     //S-DSP
  cothread_t thread_active;  //reference to active thread

  struct {
    uint32 cpu_freq;
    uint32 cop_freq;
    uint32 smp_freq;

    int64 cpucop;
    int64 cpuppu;
    int64 cpusmp;
    int64 smpdsp;
  } clock;

  enum sync_t { SyncNone, SyncCpu, SyncAll } sync;

  //==========
  //CPU <> COP
  //==========

  alwaysinline void sync_cpucop() {
    if(clock.cpucop < 0) {
      thread_active = thread_cop;
      co_switch(thread_cop);
    }
  }

  alwaysinline void sync_copcpu() {
    if(clock.cpucop >= 0 && sync != SyncAll) {
      thread_active = thread_cpu;
      co_switch(thread_cpu);
    }
  }

  //==========
  //CPU <> PPU
  //==========

  alwaysinline void sync_cpuppu() {
    if(clock.cpuppu < 0) {
      thread_active = thread_ppu;
      co_switch(thread_ppu);
    }
  }

  alwaysinline void sync_ppucpu() {
    if(clock.cpuppu >= 0 && sync != SyncAll) {
      thread_active = thread_cpu;
      co_switch(thread_cpu);
    }
  }

  //==========
  //CPU <> SMP
  //==========

  alwaysinline void sync_cpusmp() {
    if(clock.cpusmp < 0) {
      thread_active = thread_smp;
      co_switch(thread_smp);
    }
  }

  alwaysinline void sync_smpcpu() {
    if(clock.cpusmp >= 0 && sync != SyncAll) {
      thread_active = thread_cpu;
      co_switch(thread_cpu);
    }
  }

  //==========
  //SMP <> DSP
  //==========

  alwaysinline void sync_smpdsp() {
    if(clock.smpdsp < 0 && sync != SyncAll) {
      thread_active = thread_dsp;
      co_switch(thread_dsp);
    }
  }

  alwaysinline void sync_dspsmp() {
    if(clock.smpdsp >= 0 && sync != SyncAll) {
      thread_active = thread_smp;
      co_switch(thread_smp);
    }
  }

  //==========
  //add clocks
  //==========

  alwaysinline void addclocks_cpu(unsigned clocks) {
    clock.cpucop -= clocks * (uint64)clock.cop_freq;
    clock.cpuppu -= clocks;
    clock.cpusmp -= clocks * (uint64)clock.smp_freq;
  }

  alwaysinline void addclocks_cop(unsigned clocks) {
    clock.cpucop += clocks * (uint64)clock.cpu_freq;
  }

  alwaysinline void addclocks_ppu(unsigned clocks) {
    clock.cpuppu += clocks;
  }

  alwaysinline void addclocks_smp(unsigned clocks) {
    clock.cpusmp += clocks * (uint64)clock.cpu_freq;
    clock.smpdsp -= clocks;
  }

  alwaysinline void addclocks_dsp(unsigned clocks) {
    clock.smpdsp += clocks;
  }

  enum ExitReason { UnknownEvent, FrameEvent, SynchronizeEvent, DebuggerEvent };

  void enter();
  void exit(ExitReason);
  ExitReason exit_reason() const;

  void init();

  Scheduler();

private:
  ExitReason exit_reason_;
};

extern Scheduler scheduler;
