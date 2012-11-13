//70224 clocks/frame
//  456 clocks/scanline
//  154 scanlines/frame

#ifdef CPU_CPP

void CPU::add_clocks(unsigned clocks) {
  system.clocks_executed += clocks;
  if(system.sgb()) scheduler.exit(Scheduler::ExitReason::StepEvent);

  status.clock += clocks;
  if(status.clock >= 4 * 1024 * 1024) {
    status.clock -= 4 * 1024 * 1024;
    cartridge.mbc3.second();
  }

  //4MHz / N(hz) - 1 = mask
  if((status.clock &   15) == 0) timer_262144hz();
  if((status.clock &   63) == 0)  timer_65536hz();
  if((status.clock &  255) == 0)  timer_16384hz();
  if((status.clock &  511) == 0)   timer_8192hz();
  if((status.clock & 1023) == 0)   timer_4096hz();

  ppu.clock -= clocks * ppu.frequency;
  if(ppu.clock < 0) co_switch(scheduler.active_thread = ppu.thread);

  apu.clock -= clocks * apu.frequency;
  if(apu.clock < 0) co_switch(scheduler.active_thread = apu.thread);
}

void CPU::timer_262144hz() {
  if(status.timer_enable && status.timer_clock == 1) {
    if(++status.tima == 0) {
      status.tima = status.tma;
      interrupt_raise(Interrupt::Timer);
    }
  }
}

void CPU::timer_65536hz() {
  if(status.timer_enable && status.timer_clock == 2) {
    if(++status.tima == 0) {
      status.tima = status.tma;
      interrupt_raise(Interrupt::Timer);
    }
  }
}

void CPU::timer_16384hz() {
  if(status.timer_enable && status.timer_clock == 3) {
    if(++status.tima == 0) {
      status.tima = status.tma;
      interrupt_raise(Interrupt::Timer);
    }
  }

  status.div++;
}

void CPU::timer_8192hz() {
  if(status.serial_transfer && status.serial_clock) {
    if(--status.serial_bits == 0) {
      status.serial_transfer = 0;
      interrupt_raise(Interrupt::Serial);
    }
  }
}

void CPU::timer_4096hz() {
  if(status.timer_enable && status.timer_clock == 0) {
    if(++status.tima == 0) {
      status.tima = status.tma;
      interrupt_raise(Interrupt::Timer);
    }
  }
}

void CPU::hblank() {
  if(status.dma_mode == 1 && status.dma_length) {
    for(unsigned n = 0; n < 16; n++) {
      bus.write(status.dma_target++, bus.read(status.dma_source++));
      add_clocks(4);
    }
    status.dma_length -= 16;
  }
}

#endif
