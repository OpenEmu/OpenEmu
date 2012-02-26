#ifdef SMP_CPP

alwaysinline uint8 SMP::ram_read(uint16 addr) {
  if(addr >= 0xffc0 && status.iplrom_enable) return iplrom[addr & 0x3f];
  if(status.ram_disable) return 0x5a;  //0xff on mini-SNES
  return apuram[addr];
}

alwaysinline void SMP::ram_write(uint16 addr, uint8 data) {
  //writes to $ffc0-$ffff always go to apuram, even if the iplrom is enabled
  if(status.ram_writable && !status.ram_disable) apuram[addr] = data;
}

uint8 SMP::port_read(uint2 port) const {
  return apuram[0xf4 + port];
}

void SMP::port_write(uint2 port, uint8 data) {
  apuram[0xf4 + port] = data;
}

alwaysinline uint8 SMP::op_busread(uint16 addr) {
  unsigned result;

  switch(addr) {
  case 0xf0:  //TEST -- write-only register
    return 0x00;

  case 0xf1:  //CONTROL -- write-only register
    return 0x00;

  case 0xf2:  //DSPADDR
    return status.dsp_addr;

  case 0xf3:  //DSPDATA
    //0x80-0xff are read-only mirrors of 0x00-0x7f
    return dsp.read(status.dsp_addr & 0x7f);

  case 0xf4:  //CPUIO0
  case 0xf5:  //CPUIO1
  case 0xf6:  //CPUIO2
  case 0xf7:  //CPUIO3
    synchronize_cpu();
    return cpu.port_read(addr);

  case 0xf8:  //RAM0
    return status.ram00f8;

  case 0xf9:  //RAM1
    return status.ram00f9;

  case 0xfa:  //T0TARGET
  case 0xfb:  //T1TARGET
  case 0xfc:  //T2TARGET -- write-only registers
    return 0x00;

  case 0xfd:  //T0OUT -- 4-bit counter value
    result = timer0.stage3_ticks;
    timer0.stage3_ticks = 0;
    return result;

  case 0xfe:  //T1OUT -- 4-bit counter value
    result = timer1.stage3_ticks;
    timer1.stage3_ticks = 0;
    return result;

  case 0xff:  //T2OUT -- 4-bit counter value
    result = timer2.stage3_ticks;
    timer2.stage3_ticks = 0;
    return result;
  }

  return ram_read(addr);
}

alwaysinline void SMP::op_buswrite(uint16 addr, uint8 data) {
  switch(addr) {
  case 0xf0:  //TEST
    if(regs.p.p) break;  //writes only valid when P flag is clear

    status.clock_speed    = (data >> 6) & 3;
    status.timer_speed    = (data >> 4) & 3;
    status.timers_enable  = data & 0x08;
    status.ram_disable    = data & 0x04;
    status.ram_writable   = data & 0x02;
    status.timers_disable = data & 0x01;

    status.timer_step = (1 << status.clock_speed) + (2 << status.timer_speed);

    timer0.synchronize_stage1();
    timer1.synchronize_stage1();
    timer2.synchronize_stage1();
    break;

  case 0xf1:  //CONTROL
    status.iplrom_enable = data & 0x80;

    if(data & 0x30) {
      //one-time clearing of APU port read registers,
      //emulated by simulating CPU writes of 0x00
      synchronize_cpu();
      if(data & 0x20) {
        cpu.port_write(2, 0x00);
        cpu.port_write(3, 0x00);
      }
      if(data & 0x10) {
        cpu.port_write(0, 0x00);
        cpu.port_write(1, 0x00);
      }
    }

    //0->1 transistion resets timers
    if(timer2.enable == false && (data & 0x04)) {
      timer2.stage2_ticks = 0;
      timer2.stage3_ticks = 0;
    }
    timer2.enable = data & 0x04;

    if(timer1.enable == false && (data & 0x02)) {
      timer1.stage2_ticks = 0;
      timer1.stage3_ticks = 0;
    }
    timer1.enable = data & 0x02;

    if(timer0.enable == false && (data & 0x01)) {
      timer0.stage2_ticks = 0;
      timer0.stage3_ticks = 0;
    }
    timer0.enable = data & 0x01;
    break;

  case 0xf2:  //DSPADDR
    status.dsp_addr = data;
    break;

  case 0xf3:  //DSPDATA
    if(status.dsp_addr & 0x80) break;  //0x80-0xff are read-only mirrors of 0x00-0x7f
    dsp.write(status.dsp_addr & 0x7f, data);
    break;

  case 0xf4:  //CPUIO0
  case 0xf5:  //CPUIO1
  case 0xf6:  //CPUIO2
  case 0xf7:  //CPUIO3
    synchronize_cpu();
    port_write(addr, data);
    break;

  case 0xf8:  //RAM0
    status.ram00f8 = data;
    break;

  case 0xf9:  //RAM1
    status.ram00f9 = data;
    break;

  case 0xfa:  //T0TARGET
    timer0.target = data;
    break;

  case 0xfb:  //T1TARGET
    timer1.target = data;
    break;

  case 0xfc:  //T2TARGET
    timer2.target = data;
    break;

  case 0xfd:  //T0OUT
  case 0xfe:  //T1OUT
  case 0xff:  //T2OUT -- read-only registers
    break;
  }

  ram_write(addr, data);  //all writes, even to MMIO registers, appear on bus
}

void SMP::op_io() {
  add_clocks(24);
  cycle_edge();
}

uint8 SMP::op_read(uint16 addr) {
  add_clocks(12);
  uint8 r = op_busread(addr);
  add_clocks(12);
  cycle_edge();
  return r;
}

void SMP::op_write(uint16 addr, uint8 data) {
  add_clocks(24);
  op_buswrite(addr, data);
  cycle_edge();
}

#endif
