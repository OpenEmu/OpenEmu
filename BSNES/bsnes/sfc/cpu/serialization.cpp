#ifdef CPU_CPP

void CPU::serialize(serializer &s) {
  R65816::serialize(s);
  Thread::serialize(s);
  PPUcounter::serialize(s);

  s.array(wram);

  s.integer(cpu_version);

  s.integer(status.interrupt_pending);

  s.integer(status.clock_count);
  s.integer(status.line_clocks);

  s.integer(status.irq_lock);

  s.integer(status.dram_refresh_position);
  s.integer(status.dram_refreshed);

  s.integer(status.hdma_init_position);
  s.integer(status.hdma_init_triggered);

  s.integer(status.hdma_position);
  s.integer(status.hdma_triggered);

  s.integer(status.nmi_valid);
  s.integer(status.nmi_line);
  s.integer(status.nmi_transition);
  s.integer(status.nmi_pending);
  s.integer(status.nmi_hold);

  s.integer(status.irq_valid);
  s.integer(status.irq_line);
  s.integer(status.irq_transition);
  s.integer(status.irq_pending);
  s.integer(status.irq_hold);

  s.integer(status.reset_pending);

  s.integer(status.dma_active);
  s.integer(status.dma_counter);
  s.integer(status.dma_clocks);
  s.integer(status.dma_pending);
  s.integer(status.hdma_pending);
  s.integer(status.hdma_mode);

  s.integer(status.auto_joypad_active);
  s.integer(status.auto_joypad_latch);
  s.integer(status.auto_joypad_counter);
  s.integer(status.auto_joypad_clock);

  s.array(status.port);

  s.integer(status.wram_addr);

  s.integer(status.joypad_strobe_latch);
  s.integer(status.joypad1_bits);
  s.integer(status.joypad2_bits);

  s.integer(status.nmi_enabled);
  s.integer(status.hirq_enabled);
  s.integer(status.virq_enabled);
  s.integer(status.auto_joypad_poll);

  s.integer(status.pio);

  s.integer(status.wrmpya);
  s.integer(status.wrmpyb);

  s.integer(status.wrdiva);
  s.integer(status.wrdivb);

  s.integer(status.hirq_pos);
  s.integer(status.virq_pos);

  s.integer(status.rom_speed);

  s.integer(status.rddiv);
  s.integer(status.rdmpy);

  s.integer(status.joy1);
  s.integer(status.joy2);
  s.integer(status.joy3);
  s.integer(status.joy4);

  s.integer(alu.mpyctr);
  s.integer(alu.divctr);
  s.integer(alu.shift);

  for(unsigned i = 0; i < 8; i++) {
    s.integer(channel[i].dma_enabled);
    s.integer(channel[i].hdma_enabled);
    s.integer(channel[i].direction);
    s.integer(channel[i].indirect);
    s.integer(channel[i].unused);
    s.integer(channel[i].reverse_transfer);
    s.integer(channel[i].fixed_transfer);
    s.integer(channel[i].transfer_mode);
    s.integer(channel[i].dest_addr);
    s.integer(channel[i].source_addr);
    s.integer(channel[i].source_bank);
    s.integer(channel[i].transfer_size);
    s.integer(channel[i].indirect_bank);
    s.integer(channel[i].hdma_addr);
    s.integer(channel[i].line_counter);
    s.integer(channel[i].unknown);
    s.integer(channel[i].hdma_completed);
    s.integer(channel[i].hdma_do_transfer);
  }

  s.integer(pipe.valid);
  s.integer(pipe.addr);
  s.integer(pipe.data);
}

#endif
