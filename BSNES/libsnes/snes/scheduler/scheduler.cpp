#ifdef SYSTEM_CPP

Scheduler scheduler;

void Scheduler::enter() {
  host_thread = co_active();
  co_switch(thread);
}

void Scheduler::exit(ExitReason::e reason) {
  exit_reason.i = reason;
  thread = co_active();
  co_switch(host_thread);
}

void Scheduler::init() {
  host_thread = co_active();
  thread = cpu.thread;
  sync.i = SynchronizeMode::None;
}

Scheduler::Scheduler() {
  host_thread = 0;
  thread = 0;
  exit_reason.i = ExitReason::UnknownEvent;
}

#endif
