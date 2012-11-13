#define CYCLE_ACCURATE

#include <sfc/sfc.hpp>

#define SMP_CPP
namespace SuperFamicom {

SMP smp;

#include "algorithms.cpp"
#include "core.cpp"
#include "memory.cpp"
#include "timing.cpp"

void SMP::synchronize_cpu() {
  if(CPU::Threaded == true) {
  //if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
  } else {
    while(clock >= 0) cpu.enter();
  }
}

void SMP::synchronize_dsp() {
  if(DSP::Threaded == true) {
  //if(dsp.clock < 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(dsp.thread);
  } else {
    while(dsp.clock < 0) dsp.enter();
  }
}

void SMP::enter() {
  while(clock < 0) op_step();
}

void SMP::power() {
  Thread::frequency = system.apu_frequency();
  Thread::clock = 0;

  timer0.target = 0;
  timer1.target = 0;
  timer2.target = 0;

  for(unsigned n = 0; n < 256; n++) {
    cycle_table_dsp[n] = (cycle_count_table[n] * 24);
    cycle_table_cpu[n] = (cycle_count_table[n] * 24) * cpu.frequency;
  }

  cycle_step_cpu = 24 * cpu.frequency;

  reset();
}

void SMP::reset() {
  for(unsigned n = 0x0000; n <= 0xffff; n++) apuram[n] = 0x00;

  opcode_number = 0;
  opcode_cycle = 0;

  regs.pc = 0xffc0;
  regs.sp = 0xef;
  regs.a = 0x00;
  regs.x = 0x00;
  regs.y = 0x00;
  regs.p = 0x02;

  //$00f1
  status.iplrom_enable = true;

  //$00f2
  status.dsp_addr = 0x00;

  //$00f8,$00f9
  status.ram00f8 = 0x00;
  status.ram00f9 = 0x00;

  //timers
  timer0.enable = timer1.enable = timer2.enable = false;
  timer0.stage1_ticks = timer1.stage1_ticks = timer2.stage1_ticks = 0;
  timer0.stage2_ticks = timer1.stage2_ticks = timer2.stage2_ticks = 0;
  timer0.stage3_ticks = timer1.stage3_ticks = timer2.stage3_ticks = 0;
}

void SMP::serialize(serializer &s) {
  Thread::serialize(s);

  s.array(apuram, 64 * 1024);

  s.integer(opcode_number);
  s.integer(opcode_cycle);

  s.integer(regs.pc);
  s.integer(regs.sp);
  s.integer(regs.a);
  s.integer(regs.x);
  s.integer(regs.y);

  s.integer(regs.p.n);
  s.integer(regs.p.v);
  s.integer(regs.p.p);
  s.integer(regs.p.b);
  s.integer(regs.p.h);
  s.integer(regs.p.i);
  s.integer(regs.p.z);
  s.integer(regs.p.c);

  s.integer(rd);
  s.integer(wr);
  s.integer(dp);
  s.integer(sp);
  s.integer(ya);
  s.integer(bit);

  s.integer(status.iplrom_enable);

  s.integer(status.dsp_addr);

  s.integer(status.ram00f8);
  s.integer(status.ram00f9);

  s.integer(timer0.enable);
  s.integer(timer0.target);
  s.integer(timer0.stage1_ticks);
  s.integer(timer0.stage2_ticks);
  s.integer(timer0.stage3_ticks);

  s.integer(timer1.enable);
  s.integer(timer1.target);
  s.integer(timer1.stage1_ticks);
  s.integer(timer1.stage2_ticks);
  s.integer(timer1.stage3_ticks);

  s.integer(timer2.enable);
  s.integer(timer2.target);

  s.integer(timer2.stage1_ticks);
  s.integer(timer2.stage2_ticks);
  s.integer(timer2.stage3_ticks);
}

SMP::SMP() {
  apuram = new uint8[64 * 1024];
  for(auto &byte : iplrom) byte = 0;
}

SMP::~SMP() {
}

}
