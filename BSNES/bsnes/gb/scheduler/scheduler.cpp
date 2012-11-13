#include <gb/gb.hpp>

#define SCHEDULER_CPP
namespace GameBoy {

Scheduler scheduler;

void Scheduler::enter() {
  host_thread = co_active();
  co_switch(active_thread);
}

void Scheduler::exit(ExitReason reason) {
  exit_reason = reason;
  active_thread = co_active();
  co_switch(host_thread);
}

void Scheduler::init() {
  host_thread = co_active();
  active_thread = cpu.thread;
}

Scheduler::Scheduler() {
  exit_reason = ExitReason::UnknownEvent;
  host_thread = 0;
  active_thread = 0;
}

}
