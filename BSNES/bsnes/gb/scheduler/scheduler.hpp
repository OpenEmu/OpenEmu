struct Scheduler : property<Scheduler> {
  enum class SynchronizeMode : unsigned { None, CPU, All } sync;
  enum class ExitReason : unsigned { UnknownEvent, StepEvent, FrameEvent, SynchronizeEvent };
  readonly<ExitReason> exit_reason;

  cothread_t host_thread;
  cothread_t active_thread;

  void enter();
  void exit(ExitReason);

  void init();
  Scheduler();
};

extern Scheduler scheduler;
