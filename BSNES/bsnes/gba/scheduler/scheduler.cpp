#include <gba/gba.hpp>

namespace GameBoyAdvance {

Scheduler scheduler;

void Scheduler::enter() {
  host = co_active();
  co_switch(active);
}

void Scheduler::exit(ExitReason reason) {
  exit_reason = reason;
  active = co_active();
  co_switch(host);
}

void Scheduler::power() {
  host = co_active();
  active = cpu.thread;
}

Scheduler::Scheduler() {
  sync = SynchronizeMode::None;
  exit_reason = ExitReason::UnknownEvent;
  host = nullptr;
  active = nullptr;
}

}
