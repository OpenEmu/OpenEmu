#ifdef SCPU_CPP

void sCPU::serialize(serializer &s) {
  CPU::serialize(s);
  CPUcore::core_serialize(s);

  event.serialize(s);

  s.integer(status.interrupt_pending);
  s.integer(status.interrupt_vector);

  s.integer(status.clock_count);
  s.integer(status.line_clocks);

  s.integer(status.irq_lock);
  s.integer(status.alu_lock);
  s.integer(status.dram_refresh_position);

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

  s.integer(status.wram_addr);

  s.integer(status.joypad_strobe_latch);
  s.integer(status.joypad1_bits);
  s.integer(status.joypad2_bits);

  s.integer(status.nmi_enabled);
  s.integer(status.hirq_enabled);
  s.integer(status.virq_enabled);
  s.integer(status.auto_joypad_poll);

  s.integer(status.pio);

  s.integer(status.mul_a);
  s.integer(status.mul_b);

  s.integer(status.div_a);
  s.integer(status.div_b);

  s.integer(status.hirq_pos);
  s.integer(status.virq_pos);

  s.integer(status.rom_speed);

  s.integer(status.r4214);
  s.integer(status.r4216);

  s.integer(status.joy1l);
  s.integer(status.joy1h);
  s.integer(status.joy2l);
  s.integer(status.joy2h);
  s.integer(status.joy3l);
  s.integer(status.joy3h);
  s.integer(status.joy4l);
  s.integer(status.joy4h);

  for(unsigned i = 0; i < 8; i++) {
    s.integer(channel[i].dma_enabled);
    s.integer(channel[i].hdma_enabled);
    s.integer(channel[i].dmap);
    s.integer(channel[i].direction);
    s.integer(channel[i].hdma_indirect);
    s.integer(channel[i].reversexfer);
    s.integer(channel[i].fixedxfer);
    s.integer(channel[i].xfermode);
    s.integer(channel[i].destaddr);
    s.integer(channel[i].srcaddr);
    s.integer(channel[i].srcbank);
    s.integer(channel[i].xfersize);
    s.integer(channel[i].hdma_ibank);
    s.integer(channel[i].hdma_addr);
    s.integer(channel[i].hdma_line_counter);
    s.integer(channel[i].unknown);
    s.integer(channel[i].hdma_completed);
    s.integer(channel[i].hdma_do_transfer);
  }

  s.integer(apu_port[0]);
  s.integer(apu_port[1]);
  s.integer(apu_port[2]);
  s.integer(apu_port[3]);

  s.integer(cycle_edge_state);
}

#endif
