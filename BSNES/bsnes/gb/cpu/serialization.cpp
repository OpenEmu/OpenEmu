#ifdef CPU_CPP

void CPU::serialize(serializer &s) {
  LR35902::serialize(s);
  Thread::serialize(s);

  s.array(wram);
  s.array(hram);

  s.integer(status.clock);

  s.integer(status.p15);
  s.integer(status.p14);
  s.integer(status.joyp);
  s.integer(status.mlt_req);

  s.integer(status.serial_data);
  s.integer(status.serial_bits);

  s.integer(status.serial_transfer);
  s.integer(status.serial_clock);

  s.integer(status.div);
  s.integer(status.tima);
  s.integer(status.tma);
  s.integer(status.timer_enable);
  s.integer(status.timer_clock);

  s.integer(status.interrupt_request_joypad);
  s.integer(status.interrupt_request_serial);
  s.integer(status.interrupt_request_timer);
  s.integer(status.interrupt_request_stat);
  s.integer(status.interrupt_request_vblank);

  s.integer(status.speed_double);
  s.integer(status.speed_switch);

  s.integer(status.dma_source);
  s.integer(status.dma_target);
  s.integer(status.dma_mode);
  s.integer(status.dma_length);

  s.integer(status.ff6c);

  s.integer(status.wram_bank);

  s.integer(status.ff72);
  s.integer(status.ff73);
  s.integer(status.ff74);
  s.integer(status.ff75);

  s.integer(status.interrupt_enable_joypad);
  s.integer(status.interrupt_enable_serial);
  s.integer(status.interrupt_enable_timer);
  s.integer(status.interrupt_enable_stat);
  s.integer(status.interrupt_enable_vblank);
}

#endif
