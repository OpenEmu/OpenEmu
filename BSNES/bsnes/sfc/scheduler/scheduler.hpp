struct Scheduler : property<Scheduler> {
  enum class SynchronizeMode : unsigned { None, CPU, All } sync;
  enum class ExitReason : unsigned { UnknownEvent, FrameEvent, SynchronizeEvent, DebuggerEvent };
  readonly<ExitReason> exit_reason;

  cothread_t host_thread;  //program thread (used to exit emulation)
  cothread_t thread;       //active emulation thread (used to enter emulation)

  void enter();
  void exit(ExitReason);
  void debug();

  void init();
  Scheduler();
};

extern Scheduler scheduler;
