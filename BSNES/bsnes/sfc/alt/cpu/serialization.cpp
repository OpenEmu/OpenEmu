#ifdef CPU_CPP

void CPU::serialize(serializer &s) {
  R65816::serialize(s);
  Thread::serialize(s);
  PPUcounter::serialize(s);

  s.array(wram);

  queue.serialize(s);
  s.array(port_data);

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

  s.integer(status.nmi_valid);
  s.integer(status.nmi_line);
  s.integer(status.nmi_transition);
  s.integer(status.nmi_pending);

  s.integer(status.irq_valid);
  s.integer(status.irq_line);
  s.integer(status.irq_transition);
  s.integer(status.irq_pending);

  s.integer(status.irq_lock);
  s.integer(status.hdma_pending);

  s.integer(status.wram_addr);

  s.integer(status.joypad_strobe_latch);

  s.integer(status.nmi_enabled);
  s.integer(status.virq_enabled);
  s.integer(status.hirq_enabled);
  s.integer(status.auto_joypad_poll_enabled);

  s.integer(status.pio);

  s.integer(status.wrmpya);
  s.integer(status.wrmpyb);
  s.integer(status.wrdiva);
  s.integer(status.wrdivb);

  s.integer(status.htime);
  s.integer(status.vtime);

  s.integer(status.rom_speed);

  s.integer(status.rddiv);
  s.integer(status.rdmpy);

  s.integer(status.joy1l);
  s.integer(status.joy1h);
  s.integer(status.joy2l);
  s.integer(status.joy2h);
  s.integer(status.joy3l);
  s.integer(status.joy3h);
  s.integer(status.joy4l);
  s.integer(status.joy4h);
}

#endif
