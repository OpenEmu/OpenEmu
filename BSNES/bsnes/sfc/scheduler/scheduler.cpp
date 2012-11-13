#ifdef SYSTEM_CPP

Scheduler scheduler;

void Scheduler::enter() {
  host_thread = co_active();
  co_switch(thread);
}

void Scheduler::exit(ExitReason reason) {
  exit_reason = reason;
  thread = co_active();
  co_switch(host_thread);
}

void Scheduler::debug() {
  exit(ExitReason::DebuggerEvent);
}

void Scheduler::init() {
  host_thread = co_active();
  thread = cpu.thread;
  sync = SynchronizeMode::None;
}

Scheduler::Scheduler() {
  host_thread = 0;
  thread = 0;
  exit_reason = ExitReason::UnknownEvent;
}

#endif
