struct Scheduler : property<Scheduler> {
  struct SynchronizeMode {
    enum e {
      None, CPU, All
    } i;
  } sync;

  struct ExitReason {
    enum e {
      UnknownEvent, StepEvent, FrameEvent, SynchronizeEvent
    } i;
  };

  ExitReason exit_reason;

  cothread_t host_thread;
  cothread_t active_thread;

  void enter();
  void exit(ExitReason::e);
  void swapto(Processor&);

  void init();
  Scheduler();
};

extern Scheduler scheduler;
