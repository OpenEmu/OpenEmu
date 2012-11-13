struct Scheduler : property<Scheduler> {
  enum class SynchronizeMode : unsigned { None, CPU, All } sync;
  enum class ExitReason : unsigned { UnknownEvent, FrameEvent, SynchronizeEvent };
  readonly<ExitReason> exit_reason;

  cothread_t host;
  cothread_t active;

  void enter();
  void exit(ExitReason);

  void power();
  Scheduler();
};

extern Scheduler scheduler;
