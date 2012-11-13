void APU::DMC::start() {
  if(length_counter == 0) {
    read_addr = 0x4000 + (addr_latch << 6);
    length_counter = (length_latch << 4) + 1;
  }
}

void APU::DMC::stop() {
  length_counter = 0;
  dma_delay_counter = 0;
  cpu.set_rdy_line(1);
  cpu.set_rdy_addr({ false, 0u });
}

uint8 APU::DMC::clock() {
  uint8 result = dac_latch;

  if(dma_delay_counter > 0) {
    dma_delay_counter--;

    if(dma_delay_counter == 1) {
      cpu.set_rdy_addr({ true, uint16(0x8000 | read_addr) });
    } else if(dma_delay_counter == 0) {
      cpu.set_rdy_line(1);
      cpu.set_rdy_addr({ false, 0u });

      dma_buffer = cpu.mdr();
      have_dma_buffer = true;
      length_counter--;
      read_addr++;

      if(length_counter == 0) {
        if(loop_mode) {
          start();
        } else if(irq_enable) {
          irq_pending = true;
          apu.set_irq_line();
        }
      }
    }
  }

  if(--period_counter == 0) {
    if(have_sample) {
      signed delta = (((sample >> bit_counter) & 1) << 2) - 2;
      unsigned data = dac_latch + delta;
      if((data & 0x80) == 0) dac_latch = data;
    }

    if(++bit_counter == 0) {
      if(have_dma_buffer) {
        have_sample = true;
        sample = dma_buffer;
        have_dma_buffer = false;
      } else {
        have_sample = false;
      }
    }

    period_counter = ntsc_dmc_period_table[period];
  }

  if(length_counter > 0 && have_dma_buffer == false && dma_delay_counter == 0) {
    cpu.set_rdy_line(0);
    dma_delay_counter = 4;
  }

  return result;
}

void APU::DMC::power() {
}

void APU::DMC::reset() {
  length_counter = 0;
  irq_pending = 0;

  period = 0;
  period_counter = ntsc_dmc_period_table[0];
  irq_enable = 0;
  loop_mode = 0;
  dac_latch = 0;
  addr_latch = 0;
  length_latch = 0;
  read_addr = 0;
  dma_delay_counter = 0;
  bit_counter = 0;
  have_dma_buffer = 0;
  dma_buffer = 0;
  have_sample = 0;
  sample = 0;
}

void APU::DMC::serialize(serializer &s) {
  s.integer(length_counter);
  s.integer(irq_pending);

  s.integer(period);
  s.integer(period_counter);

  s.integer(irq_enable);
  s.integer(loop_mode);

  s.integer(dac_latch);
  s.integer(addr_latch);
  s.integer(length_latch);

  s.integer(read_addr);
  s.integer(dma_delay_counter);

  s.integer(bit_counter);
  s.integer(have_dma_buffer);
  s.integer(dma_buffer);

  s.integer(have_sample);
  s.integer(sample);
}
