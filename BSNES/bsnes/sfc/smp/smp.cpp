#include <sfc/sfc.hpp>

#define SMP_CPP
namespace SuperFamicom {

SMP smp;

#include "memory.cpp"
#include "timing.cpp"
#include "serialization.cpp"

void SMP::step(unsigned clocks) {
  clock += clocks * (uint64)cpu.frequency;
  dsp.clock -= clocks;
}

void SMP::synchronize_cpu() {
  if(CPU::Threaded == true) {
    if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
  } else {
    while(clock >= 0) cpu.enter();
  }
}

void SMP::synchronize_dsp() {
  if(DSP::Threaded == true) {
    if(dsp.clock < 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(dsp.thread);
  } else {
    while(dsp.clock < 0) dsp.enter();
  }
}

void SMP::Enter() { smp.enter(); }

void SMP::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    debugger.op_exec(regs.pc);
    op_step();
  }
}

void SMP::power() {
  //targets not initialized/changed upon reset
  timer0.target = 0;
  timer1.target = 0;
  timer2.target = 0;
}

void SMP::reset() {
  create(Enter, system.apu_frequency());

  regs.pc = 0xffc0;
  regs.a = 0x00;
  regs.x = 0x00;
  regs.y = 0x00;
  regs.s = 0xef;
  regs.p = 0x02;

  for(auto &n : apuram) n = random(0x00);
  apuram[0x00f4] = 0x00;
  apuram[0x00f5] = 0x00;
  apuram[0x00f6] = 0x00;
  apuram[0x00f7] = 0x00;

  status.clock_counter = 0;
  status.dsp_counter = 0;
  status.timer_step = 3;

  //$00f0
  status.clock_speed = 0;
  status.timer_speed = 0;
  status.timers_enable = true;
  status.ram_disable = false;
  status.ram_writable = true;
  status.timers_disable = false;

  //$00f1
  status.iplrom_enable = true;

  //$00f2
  status.dsp_addr = 0x00;

  //$00f8,$00f9
  status.ram00f8 = 0x00;
  status.ram00f9 = 0x00;

  timer0.stage0_ticks = 0;
  timer1.stage0_ticks = 0;
  timer2.stage0_ticks = 0;

  timer0.stage1_ticks = 0;
  timer1.stage1_ticks = 0;
  timer2.stage1_ticks = 0;

  timer0.stage2_ticks = 0;
  timer1.stage2_ticks = 0;
  timer2.stage2_ticks = 0;

  timer0.stage3_ticks = 0;
  timer1.stage3_ticks = 0;
  timer2.stage3_ticks = 0;

  timer0.current_line = 0;
  timer1.current_line = 0;
  timer2.current_line = 0;

  timer0.enable = false;
  timer1.enable = false;
  timer2.enable = false;
}

SMP::SMP() {
  for(auto &byte : iplrom) byte = 0;
}

SMP::~SMP() {
}

}
