struct Scheduler : property<Scheduler> {
  struct SynchronizeMode {
    enum e {
      None,
      CPU,
      All
    } i;
  } sync;

  struct ExitReason {
    enum e {
       UnknownEvent,
       FrameEvent,
       SynchronizeEvent,
       DebuggerEvent
    } i;
  };

  ExitReason exit_reason;

  cothread_t host_thread;  //program thread (used to exit emulation)
  cothread_t thread;       //active emulation thread (used to enter emulation)

  void enter();
  void exit(ExitReason::e);

  void init();
  Scheduler();
};

extern Scheduler scheduler;
